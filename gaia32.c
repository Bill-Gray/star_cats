#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gaia32.h"

/* Basic access functions for Dave Tholen's Gaia32 data.  Please
contact pluto (at) projectpluto.com with comments/bug fixes.  */

/* History:
     2016 Dec 10:  (BJG) First version.
*/

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

   sprintf( obuff, "%03d-%06ld ", zone, offset);
   if( output_format & GAIA32_BASE_60)
      {
      const int64_t ra = (int64_t)( star->ra * 100. / 15. + .5);
      const long dec = (long)abs( star->dec);

      sprintf( obuff + 11, "%02d %02d %02d.%05d %c%02ld %02ld %02ld.%03ld",
               (int)( ra / (int64_t)360000000),
               (int)( ra / (int64_t)  6000000) % 60,
               (int)( ra / (int64_t)   100000) % 60,
               (int)( ra % (int64_t)   100000),
               (star->dec > 0. ? '+' : '-'),
               dec / 3600000L, (dec / 60000L) % 60L,
               (dec / 1000L) % 60L, dec % 1000L);
      }
  else         /* output RA/decs in decimal degrees */
      sprintf( obuff + 11, "%12.8lf %12.8lf",
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
   char filename[80];

   sprintf( filename, "%s" path_separator "%03d.cat", path, zone_number);
   ifile = fopen( filename, read_only_permits);
   if( !ifile)
      {
      sprintf( filename, "%03d.cat", zone_number);
      ifile = fopen( filename, read_only_permits);
      }
   return( ifile);
}

/* A note on indexing:  within each zone,  we want to locate the stars
within a particular range in RA.  If an index is unavailable (this is
currently always the case with Gaia32), we have things narrowed down
to somewhere between the first and last records.  If an index is
available,  our search can take place within a narrower range.  But
in either case,  the range is refined by doing a secant search which
narrows down the starting point to within 'acceptable_limit' records,
currently set to 40;  i.e.,  it's possible that we will read in
forty records that are before the low end of the desired RA range.
The secant search is slightly modified to ensure that each iteration
knocks off at least 1/8 of the current range.

   Records are then read in 'buffsize' stars at a time and,  if
they're in the desired RA/dec rectangle,  written out to 'ofile'. */

#include <time.h>

clock_t time_searching = 0;

#define GAIA32_BAD_FILE_SIZE           -1
#define GAIA32_SEEK_FAILED             -2
#define GAIA32_SEEK2_FAILED            -3
#define GAIA32_READ_FAILED             -4
#define GAIA32_ALLOC_FAILED            -5

int extract_gaia32_stars( FILE *ofile, const double ra, const double dec,
                  const double width, const double height, const char *path,
                  const int output_format)
{
   const double dec1 = dec - height / 2., dec2 = dec + height / 2.;
   const double ra1 = ra - width / 2., ra2 = ra + width / 2.;
   const double zone_height = 1.;    /* zones are one degree strips in dec */
   int zone = (int)( (dec1  + 90.) / zone_height);
   const int end_zone = (int)( (dec2 + 90.) / zone_height);
   int rval = 0;
   const int buffsize = 400;     /* read this many stars at a try */
   GAIA32_STAR *stars = (GAIA32_STAR *)calloc( buffsize, sizeof( GAIA32_STAR));

   if( !stars)
      rval = GAIA32_ALLOC_FAILED;
   if( zone < 0)
      zone = 0;
   while( rval >= 0 && zone <= end_zone)
      {
      FILE *ifile = get_gaia32_zone_file( zone, path);

      if( ifile)
         {
         int keep_going = 1;
         int i, n_read;
         const int32_t max_ra  = (int32_t)( ra2  * 3600. * 1000.);
         const int32_t min_ra  = (int32_t)( ra1  * 3600. * 1000.);
         const int32_t min_dec = (int32_t)( dec1 * 3600. * 1000.);
         const int32_t max_dec = (int32_t)( dec2 * 3600. * 1000.);
         uint32_t offset, end_offset;
         const uint32_t acceptable_limit = 40;
         clock_t t0 = clock( );
         size_t filesize;
         int32_t ra_lo = 0;
         int32_t ra_hi = 360 * 3600 * 1000;

         offset = 0;
         if( fseek( ifile, 0L, SEEK_END))
            rval = GAIA32_SEEK_FAILED;
         filesize = ftell( ifile);
         if( filesize % sizeof( GAIA32_STAR))
            {
            fclose( ifile);
            free( stars);
            return( GAIA32_BAD_FILE_SIZE);
            }
         end_offset = filesize / sizeof( GAIA32_STAR);
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
                  rval++;
                  if( ofile)
                     {
                     if( output_format & GAIA32_RAW_BINARY)
                        fwrite( &star, 1, sizeof( GAIA32_STAR), ofile);
                     else
                        {
                        char buff[GAIA32_ASCII_SIZE];

                        write_gaia32_star( zone, offset + 1, buff, &star,
                                                      output_format);
                        fwrite( buff, 1, strlen( buff), ofile);
                        }
                     }
                  }
               offset++;
               }
         fclose( ifile);
         }
      zone++;
      }
   free( stars);

            /* We need some special handling for cases where the area
               to be extracted crosses RA=0 or RA=24: */
   if( rval >= 0 && ra >= 0. && ra < 360.)
      {
      if( ra1 < 0.)      /* left side crosses over RA=0h */
         rval += extract_gaia32_stars( ofile, ra+360., dec, width, height,
                                          path, output_format);
      if( ra2 > 360.)    /* right side crosses over RA=24h */
         rval += extract_gaia32_stars( ofile, ra-360., dec, width, height,
                                          path, output_format);
      }
   return( rval);
}
