#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "gaia32.h"

/* Basic access functions for Dave Tholen's Gaia32 data.  Please
contact pluto (at) projectpluto.com with comments/bug fixes.
See git log for history.         */

#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
static void swap_32( int32_t *ival)
{
   int8_t temp, *zval = (int8_t *)ival;

   temp = zval[0];
   zval[0] = zval[3];
   zval[3] = temp;
   temp = zval[1];
   zval[1] = zval[2];
   zval[2] = temp;
}

static void swap_16( int16_t *ival)
{
   int8_t temp, *zval = (int8_t *)ival;

   temp = zval[0];
   zval[0] = zval[1];
   zval[1] = temp;
}

void flip_gaia32_star( GAIA32_STAR *star)
{
   int i;

   swap_32( &star->ra);
   swap_32( &star->dec);
   swap_32( &star->pm_ra);
   swap_32( &star->pm_dec);
   swap_16( &star->pm_ra_sigma);
   swap_16( &star->pm_dec_sigma);
   swap_16( &star->epoch);
   swap_16( &star->mag);
   swap_16( &star->mag_sigma);
}
#endif                   // #if __BYTE_ORDER == __BIG_ENDIAN
#endif                   // #ifdef __BYTE_ORDER

/* This function writes out the data for a GAIA32 star
into an ASCII buffer,  in a passably human-readable format.  */

int write_gaia32_star( const int zone, const long offset, char *obuff,
                     const GAIA32_STAR *star, const int output_format)
{
   const long epoch  = 2000000 + star->epoch;

   sprintf( obuff, "%03d-%08ld ", zone, offset);
   if( output_format & GAIA32_BASE_60)
      {
      const int64_t ra = (int64_t)( star->ra * 100. / 15. + .5);
      const long dec = (long)abs( star->dec);

      sprintf( obuff + 13, "%02d %02d %02d.%05d %c%02ld %02ld %02ld.%03ld",
               (int)( ra / (int64_t)360000000),
               (int)( ra / (int64_t)  6000000) % 60,
               (int)( ra / (int64_t)   100000) % 60,
               (int)( ra % (int64_t)   100000),
               (star->dec > 0. ? '+' : '-'),
               dec / 3600000L, (dec / 60000L) % 60L,
               (dec / 1000L) % 60L, dec % 1000L);
      }
  else         /* output RA/decs in decimal degrees */
      sprintf( obuff + 13, "%12.8lf %+012.8lf",
               (double)star->ra / 3600000., (double)star->dec / 3600000.);

   sprintf( obuff + strlen( obuff), " %2d.%03d %3d %4d.%03d %3d %3d ",
               star->mag / 1000, abs( star->mag % 1000),
               star->mag_sigma,
               (int)epoch / 1000, (int)epoch % 1000,
               star->ra_sigma + 128, star->dec_sigma + 128);

   if( star->pm_ra || star->pm_dec || !(output_format & GAIA32_WRITE_SPACES))
      sprintf( obuff + strlen( obuff), "%8d %8d %5u %5u",
            (int)star->pm_ra, (int)star->pm_dec,
            (unsigned)star->pm_ra_sigma, (unsigned)star->pm_dec_sigma);
   else        /* no proper motion given,  keep these fields blank */
      strcat( obuff, "                             ");

   strcat( obuff, "\n");
   return( 0);
}

#if defined( __linux__) || defined( __unix__) || defined( __APPLE__)
   #define path_separator          "/"
   #define read_only_permits       "r"
#elif defined( _WIN32) || defined( _WIN64) || defined( __WATCOMC__)
   #define path_separator          "\\"
   #define read_only_permits       "rb"
#else
#error "Unknown platform; please report so it can be fixed!"
#endif

static FILE *get_gaia32_zone_file( const int zone_number, const char *path)
{
   FILE *ifile;
   char filename[10], fullname[80];

   assert( zone_number >= -1 && zone_number < 180);
   if( zone_number == -1)
      strcpy( filename, "gaia.idx");
   else
      sprintf( filename, "%03d.cat", zone_number);
   sprintf( fullname, "%s" path_separator "%s", path, filename);
   ifile = fopen( fullname, read_only_permits);
   if( !ifile)
      ifile = fopen( filename, read_only_permits);
   return( ifile);
}

/* A note on indexing:  within each zone,  we want to locate the stars
within a particular range in RA.  One can binary-search the zone file
to locate the desired starting RA,  avoiding the need for any explicit
index.  But this can be (slightly) slow.

'gaia.idx' contains an index to provide approximate ranges in RA for
each declination zone.  Searching is still done,  but within a
(usually quite small) part of the zone file.  See 'gaia_idx.c' for
details on the indexing.

The index provides a range within which to search;  this is refined by
doing a secant search which narrows down the starting point to within
'acceptable_limit' records, currently set to 40;  i.e.,  it's possible
that we will read in forty records that are before the low end of the
desired RA range. The secant search is slightly modified to ensure
that each iteration knocks off at least 1/8 of the current range.

   Records are then read in 'buffsize' stars at a time and,  if
they're in the desired RA/dec rectangle,  handed to the specified
callback function.  */

#include <time.h>

clock_t time_searching = 0;

#define GAIA32_BAD_FILE_SIZE           -1
#define GAIA32_SEEK_FAILED             -2
#define GAIA32_SEEK2_FAILED            -3
#define GAIA32_READ_FAILED             -4
#define GAIA32_ALLOC_FAILED            -5
#define GAIA32_NO_INDEX_FILE           -6
#define GAIA32_BAD_MAGIC_NUMBER        -7
#define GAIA32_CANT_READ_INDEX         -8
#define GAIA32_CANT_READ_INDEX_2       -9

int extract_gaia32_stars_callback( void *context,
     int (*callback_fn)( void *, const int, const uint32_t, GAIA32_STAR *),
                  const double ra, const double dec,
                  const double width, const double height, const char *path)
{
   const double dec1 = dec - height / 2., dec2 = dec + height / 2.;
   const double ra1 = ra - width / 2., ra2 = ra + width / 2.;
   const double zone_height = 1.;    /* zones are one degree strips in dec */
   int zone = (int)( (dec1  + 90.) / zone_height);
   const int end_zone = (int)( (dec2 + 90.) / zone_height);
   int rval = 0;
   const int buffsize = 400;     /* read this many stars at a try */
   GAIA32_STAR *stars = (GAIA32_STAR *)calloc( buffsize, sizeof( GAIA32_STAR));
   int32_t header[3], sizes[180];
   FILE *idx_file;

   if( !stars)
      rval = GAIA32_ALLOC_FAILED;
   if( zone < 0)
      zone = 0;
   idx_file = get_gaia32_zone_file( -1, path);
   if( !idx_file)
      rval = GAIA32_NO_INDEX_FILE;
   else
      {
      if( fread( header, sizeof( int32_t), 3, idx_file) != 3
            || fread( sizes, sizeof( int32_t), 180, idx_file) != 180)
          rval = GAIA32_CANT_READ_INDEX;
      else if( header[0] != (int32_t)0xfa1a3202)
          rval = GAIA32_BAD_MAGIC_NUMBER;
      }
   while( rval >= 0 && zone <= end_zone)
      {
      FILE *ifile = get_gaia32_zone_file( zone, path);

      if( ifile)
         {
         int keep_going = 1;
         int i, n_read, idx_size = (int)( sizes[zone] / header[2]);
         const int32_t max_ra  = (int32_t)( ra2  * 3600. * 1000.);
         const int32_t min_ra  = (int32_t)( ra1  * 3600. * 1000.);
         const int32_t min_dec = (int32_t)( dec1 * 3600. * 1000.);
         const int32_t max_dec = (int32_t)( dec2 * 3600. * 1000.);
         uint32_t offset, end_offset;
         const uint32_t acceptable_limit = 40;
         clock_t t0 = clock( );
         int32_t ra_lo = 0, ra_hi = 0;
         long index_offset = 183L;
         int32_t *idx = (int32_t *)malloc( (idx_size + 2) * sizeof( int32_t));
         const int32_t maximum_possible_ra = 360 * 3600 * 1000;

         for( i = 0; i < zone; i++)
             index_offset += (long)( sizes[i] / header[2]);
         fseek( idx_file, index_offset * 4L, SEEK_SET);
         *idx = 0;
         if( fread( idx + 1, sizeof( int32_t), idx_size, idx_file) != (size_t)idx_size)
            rval = GAIA32_CANT_READ_INDEX_2;
         idx[idx_size + 1] = maximum_possible_ra;
         assert( min_ra < maximum_possible_ra);
         i = 1;
         while( idx[i] < min_ra)
            i++;
         ra_lo = idx[i - 1];
         ra_hi = idx[i];
         free( idx);
         offset = (i - 1) * header[2];
         if( i == idx_size + 1)     /* we're in the last, partial block */
            end_offset = sizes[zone];
         else
            end_offset = offset + header[2];
#ifdef DEBUGGING_CODE
         printf( "Zone %d : searching offset %ld to %ld (RAs %f to %f)\n",
                  zone, (long)offset, (long)end_offset,
                  (double)ra_lo / 3600000., (double)ra_hi / 3600000.);
#endif
                     /* Secant-search within the known limits: */
         while( rval >= 0 && end_offset - offset > acceptable_limit)
            {
            GAIA32_STAR star;
            uint32_t delta = end_offset - offset, toffset;
            uint32_t minimum_bite = delta / 8 + 1;
            uint64_t tval = (uint64_t)delta *
                        (uint64_t)( min_ra - ra_lo) / (uint64_t)( ra_hi - ra_lo);

            if( tval < minimum_bite)
               tval = minimum_bite;
            else if( tval > delta - minimum_bite)
               tval = delta - minimum_bite;
            toffset = offset + (uint32_t)tval;
            if( fseek( ifile, toffset * sizeof( GAIA32_STAR), SEEK_SET))
               rval = GAIA32_SEEK2_FAILED;
            if( fread( &star, sizeof( GAIA32_STAR), 1, ifile) != 1)
               rval = GAIA32_READ_FAILED;
            if( star.ra < min_ra)
               {
               offset = toffset;
               ra_lo = star.ra;
               }
            else
               {
               end_offset = toffset;
               ra_hi = star.ra;
               }
            }
         time_searching += clock( ) - t0;
         fseek( ifile, offset * sizeof( GAIA32_STAR), SEEK_SET);

         while( rval >= 0 && keep_going &&
                  (n_read = fread( stars, sizeof( GAIA32_STAR), buffsize, ifile)) > 0)
            for( i = 0; i < n_read && keep_going; i++)
               {
               GAIA32_STAR star = stars[i];

#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
               flip_gaia32_star( &star);
#endif
#endif
               if( star.ra > max_ra)
                  keep_going = 0;
               else if( star.ra > min_ra && star.dec > min_dec
                                          && star.dec < max_dec)
                  {
                  if( callback_fn)
                     (callback_fn)( context, zone, offset, &star);
                  rval++;
                  }
               offset++;
               }
         fclose( ifile);
         }
      zone++;
      }
   if( stars)
      free( stars);
   if( idx_file)
      fclose( idx_file);

            /* We need some special handling for cases where the area
               to be extracted crosses RA=0 or RA=24: */
   if( rval >= 0 && ra >= 0. && ra < 360.)
      {
      if( ra1 < 0.)      /* left side crosses over RA=0h */
         rval += extract_gaia32_stars_callback( context, callback_fn,
                                    ra+360., dec, width, height, path);
      if( ra2 > 360.)    /* right side crosses over RA=24h */
         rval += extract_gaia32_stars_callback( context, callback_fn,
                                    ra-360., dec, width, height, path);
      }
   return( rval);
}

typedef struct
   {
   FILE *ofile;
   int output_format;
   } file_output_t;

static int output_a_gaia32_star( void *context, const int zone,
               const uint32_t offset, GAIA32_STAR *star)
{
   file_output_t *f = (file_output_t *)context;

   if( f->ofile)
      {
      if( f->output_format & GAIA32_RAW_BINARY)
         fwrite( star, 1, sizeof( GAIA32_STAR), f->ofile);
      else
         {
         char buff[GAIA32_ASCII_SIZE];

         write_gaia32_star( zone, offset + 1, buff, star,
                                       f->output_format);
         fwrite( buff, 1, strlen( buff), f->ofile);
         }
      }
   return( 0);
}

int extract_gaia32_stars( FILE *ofile, const double ra, const double dec,
                  const double width, const double height, const char *path,
                  const int output_format)
{
   file_output_t f;

   f.ofile = ofile;
   f.output_format = output_format;
   return( extract_gaia32_stars_callback( &f, output_a_gaia32_star,
                             ra, dec, width, height, path));
}
