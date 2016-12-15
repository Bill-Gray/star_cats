#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "urat1.h"

/* Basic access functions for URAT1.  Please contact
pluto (at) projectpluto.com with comments/bug fixes.

   History:
     2014 Dec 17:  (BJG) Created using 'ucac4.c' as a template.
     2015 Mar 18:  Noticed that 'u1index.unf' had become 'v1index.unf',
         and changed all instances of that filename.  Made some small
         fixes to eliminate clang and gcc compiler warnings,  and added
         comments on the FORTRAN output format.
     2016 Dec 14:  Moved to GitHub.

   URAT1 consists of 900 zones,  each .2 degrees high in declination.    */

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

void flip_urat1_star( URAT1_STAR *star)
{
   size_t i;

   swap_32( &star->ra);
   swap_32( &star->spd);
   swap_16( &star->sigs);
   swap_16( &star->sigm);
   swap_16( &star->epoc);
   swap_16( &star->mmag);
   swap_16( &star->sigp);
   swap_16( &star->nit);
   swap_16( &star->niu);
   swap_16( &star->pmr);
   swap_16( &star->pmd);
   swap_16( &star->pme);
   swap_32( &star->id2);
   for( i = 0; i < 3; i++)
      {
      swap_16( &star->twomass_mag[i]);
      swap_16( &star->twomass_mag_sigma[i]);
      }
   for( i = 0; i < 5; i++)
      {
      swap_16( &star->apass_mag[i]);
      swap_16( &star->apass_mag_sigma[i]);
      }
}
#endif                   /* #if __BYTE_ORDER == __BIG_ENDIAN */
#endif                   /* #ifdef __BYTE_ORDER              */

/* comments indicate bytes/field,  then starting columns (one-based) */

static int write_urat1_star_fortran_style( const int zone, const long offset,
               char *obuff, const URAT1_STAR *star, const int output_format)
{
   sprintf( obuff, "%10d%10d%4d%4d%3d%4d%6d%6d%4d%3d%2d%4d%4d%4d%4d%6d%6d%4d",
            star->ra, star->spd, star->sigs, star->sigm,    /* 10 10 4 4;  1 11 21 25 */
            star->nst, star->nsu, star->epoc, star->mmag,   /*  3  4 6 6; 29 32 36 42 */
            star->sigp, star->nsm, star->ref,               /*  4  3 2  ; 48 52 55 */
            star->nit, star->niu, star->ngt, star->ngu,     /*  4  4 4 4; 57 61 65 69 */
            star->pmr, star->pmd, star->pme);               /*  6  6 4  ; 73 79 85 */


   sprintf( obuff + strlen( obuff),
            "%3d%3d%11d%6d%6d%6d%5d%5d%5d%2d%2d%2d%2d%2d%2d",
            star->mfm, star->mfa, star->id2,                /*  3  3 11; 89 92 95 */
            star->twomass_mag[0], star->twomass_mag[1],     /* 6 6 6; 106 112 118 */
            star->twomass_mag[2],
            star->twomass_mag_sigma[0],                     /* 5 5 5; 124 129 134 */
            star->twomass_mag_sigma[1],
            star->twomass_mag_sigma[2],
            star->icc_flag[0], star->icc_flag[1], star->icc_flag[2], /* 2 2 2: 139 141 143 */
            star->photo_flag[0], star->photo_flag[1], star->photo_flag[2]);
                                                                     /* 2 2 2: 145 147 149 */
   sprintf( obuff + strlen( obuff),
               "%6d%6d%6d%6d%6d%5d%5d%5d%5d%5d%4d%4d",
            star->apass_mag[0],        /* 6 6 6 6 6: 151 157 163 169 175 */
            star->apass_mag[1],
            star->apass_mag[2],
            star->apass_mag[3],
            star->apass_mag[4],
            star->apass_mag_sigma[0],  /* 5 5 5 5 5: 181 186 191 196 201 */
            star->apass_mag_sigma[1],
            star->apass_mag_sigma[2],
            star->apass_mag_sigma[3],
            star->apass_mag_sigma[4],
            star->ann, star->ano);     /* 4 4: 206 210 (ending at 214) */
   if( output_format & 0x8)
      sprintf( obuff + strlen( obuff), " %03d-%06ld", zone, offset);

   strcat( obuff, "\n");
   return( 0);
}

/* This function writes out the data for a URAT1 star into an ASCII
   buffer,  in a (hopefully) human-readable format.  */

int write_urat1_star( const int zone, const long offset, char *obuff,
                     const URAT1_STAR *star, const int output_format)
{
   size_t i;

   if( output_format & URAT1_FORTRAN_STYLE)
      return( write_urat1_star_fortran_style( zone, offset, obuff, star,
                     output_format));

   sprintf( obuff, "%03d-%06ld %12.8f %12.8f",
               zone, offset,
               (double)star->ra / 3600000., (double)star->spd / 3600000. - 90.);

   sprintf( obuff + strlen( obuff), " %3d %3d %2d %2d %4d.%03d %2d.%03d %3d",
               star->sigs, star->sigm,    /* two different posn sigmas */
               star->nst, star->nsu,      /* total sets/n of sets used */
               2000 + star->epoc / 1000, star->epoc % 1000,
               star->mmag / 1000, abs( star->mmag % 1000),
               star->sigp);

   sprintf( obuff + strlen( obuff), " %2d %2d %3d %3d %2d %2d",
               star->nsm,   /*  number of sets used for URAT magnitude  */
               star->ref,   /*  largest reference star flag             */
               star->nit,   /*  total number of images (observations)   */
               star->niu,   /*  number of images used for mean position */
               star->ngt,   /*  total number of 1st order grating obs.  */
               star->ngu);  /*  number of 1st order grating pairs used  */

   sprintf( obuff + strlen( obuff), " %5d %5d %4d %2d %2d %10d",
        star->pmr,   /* proper motion RA*cosDec (from 2MASS)    */
        star->pmd,   /* proper motion Dec                       */
        star->pme,   /* proper motion error per coordinate      */
        star->mfm,   /* match flag URAT with 2MASS              */
        star->mfa,   /* match flag URAT with APASS              */
        star->id2);  /* unique 2MASS star identification number */

   for( i = 0; i < 3; i++)
      sprintf( obuff + strlen( obuff), " %2u.%03u %4d %02x %02x",
               star->twomass_mag[i] / 1000,
               star->twomass_mag[i] % 1000,
               star->twomass_mag_sigma[i],
               star->icc_flag[i],
               star->photo_flag[i]);

   for( i = 0; i < 5; i++)
      sprintf( obuff + strlen( obuff), " %2u.%03u %4d",
               star->apass_mag[i] / 1000,
               star->apass_mag[i] % 1000,
               star->apass_mag_sigma[i]);

   sprintf( obuff + strlen( obuff), "%2d %2d\n",
            star->ann, star->ano);

   if( output_format & URAT1_WRITE_SPACES)
      {
      char *tptr;

      while( (tptr = strstr( obuff + 124, " 30.000 9000 00 00 ")) != NULL)
         memset( tptr, ' ', 19);
      while( (tptr = strstr( obuff + 124, " 30.000 9000 ")) != NULL)
         memset( tptr, ' ', 12);
      }
   return( 0);
}

#if defined( __linux__) || defined( __unix__) || defined( __APPLE__)
   static const char *path_separator = "/", *read_only_permits = "r";
#elif defined( _WIN32) || defined( _WIN64) || defined( __WATCOMC__)
   static const char *path_separator = "\\", *read_only_permits = "rb";
#else
#error "Unknown platform; please report so it can be fixed!"
#endif

/* I've not seen an actual copy of URAT1 yet,  so the following may
still apply as it did for UCAC4,  or change may have occurred.  The
basic need to search in multiple directories will probably remain :

   The layout of UCAC-4 is such that data files are in the 'u4b'
folders of the two DVDs. People may copy these retaining the path
structure,  or maybe they'll put all 900 files in one folder.  So if
you ask this function for,  say, zone_number = 314 and files in the
folder /data/ucac4,  the function will look for the data under the
following four names:

z314         (i.e.,  all data copied to the current folder)
u4b/z314     (i.e.,  you've copied everything to the u4b subfolder)
/data/ucac4/z314
/data/ucac4/u4b/z314

   ...stopping when it finds a file.  This will,  I hope,  cover all
likely situations.  If you make things any more complicated,  you've
only yourself to blame.  Note that the index file 'v1index.unf' is
similarly looked for in various possible folders;  and the code will
still work even if the index isn't found -- the result will just be
a tiny bit slower. */

static FILE *get_urat1_zone_file( const int zone_number, const char *path)
{
   FILE *ifile;
   char filename[80];

   sprintf( filename, "ur1%sz%03d", path_separator, zone_number);
            /* First,  look for file in current path: */
   ifile = fopen( filename + 4, read_only_permits);
   if( !ifile)
      ifile = fopen( filename, read_only_permits);
         /* If file isn't there,  use the 'path' passed in as an argument: */
   if( !ifile && *path)
      {
      char filename2[80], *endptr;
      int i;

      strcpy( filename2, path);
      endptr = filename2 + strlen( filename2);
      if( endptr[-1] != *path_separator)
         *endptr++ = *path_separator;
      for( i = 0; !ifile && i < 2; i++)
         {
         strcpy( endptr, filename + 4 * (1 - i));
         ifile = fopen( filename2, read_only_permits);
         }
      }
   return( ifile);
}

#define URAT1_FILE_NOT_FOUND        -1
#define URAT1_BAD_ZONE_NUMBER       -2
#define URAT1_BAD_OFFSET            -3
#define URAT1_FREAD_FAILED          -4

int extract_urat1_info( const int zone, const long offset, URAT1_STAR *star,
                     const char *path)
{
   int rval;

   if( zone < 1 || zone > 900)     /* not a valid sequential number */
      rval = URAT1_BAD_ZONE_NUMBER;
   else
      {
      FILE *ifile = get_urat1_zone_file( zone, path);

      if( ifile)
         {
         if( fseek( ifile, (offset - 1) * sizeof( URAT1_STAR), SEEK_SET))
            rval = URAT1_BAD_OFFSET;
         else if( !fread( star, sizeof( URAT1_STAR), 1, ifile))
            rval = URAT1_FREAD_FAILED;
         else           /* success! */
            {
            rval = 0;
#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
            flip_urat1_star( star);
#endif
#endif
            }
         fclose( ifile);
         }
      else
         rval = URAT1_FILE_NOT_FOUND;
      }
   return( rval);
}

/* This looks for the index file v1index.unf first in the current
directory;  then in 'path';  then in the ur1 subdirectory under 'path'.
One of the three will probably work... */

static FILE *get_urat1_index_file( const char *path)
{
   FILE *index_file;
   const char *idx_filename = "v1index.unf";

                     /* Look for the index file in the local directory... */
   index_file = fopen( idx_filename, read_only_permits);
                     /* ...and if it's not there,  look for it in the same */
                     /* directory as the data: */
   if( !index_file)
      {
      char filename[100], *tptr;
      int i;

      strcpy( filename, path);
      if( filename[strlen( filename) - 1] != path_separator[0])
         strcat( filename, path_separator);
      tptr = filename + strlen( filename);
      for( i = 0; i < 2 && !index_file; i++)
         {
         if( i)      /* try 'ur1' folder */
            {
            strcpy( tptr, "ur1");
            strcat( tptr, path_separator);
            }
         strcat( filename, idx_filename);
         index_file = fopen( filename, read_only_permits);
         }
      }
   return( index_file);
}

static long get_index_file_offset( const int zone, const int ra_start)
{
   const int rval = (zone - 1) + ra_start * 900;

/* printf( "Zone %d, ra_start %d\n", zone, ra_start);   */
   return( rval * sizeof( int32_t));
}

/* RA, dec, width, height are in degrees */

/* A note on indexing:  within each zone,  we want to locate the stars
within a particular range in RA.  If an index is unavailable,  then
we have things narrowed down to somewhere between the first and
last records.  If an index is available,  our search can take
place within a narrower range.  But in either case,  the range is
refined by doing a secant search which narrows down the starting
point to within 'acceptable_limit' records,  currently set to
40;  i.e.,  it's possible that we will read in forty records that
are before the low end of the desired RA range.  The secant search
is slightly modified to ensure that each iteration knocks off at
least 1/8 of the current range.  Otherwise,  you can get cases where
secant search doesn't converge very quickly;  with the 1/8 limit,
you can nudge it away from such situations,  and it'll (usually)
start converging rapidly after that.

   Records are then read in 'buffsize' stars at a time and,  if
they're in the desired RA/dec rectangle,  written out to 'ofile'.

   A further note about v1index.unf:  the first half of the file
consists of 1440 sets of 900 32-bit integers.  Each integer gives
the total number of stars within that dec zone,  up to that (360/1440)
= 0.25 degree point within the dec zone,  _including_ the quarter
degree in question.  When we use v1index.unf,  if we're looking
for an RA between zero and 0.25 degrees,  we just say "offset=0":
we start from the beginning of the zone.  After that,  we look
for the index entry corresponding to the _previous_ quarter-degree
block,  because its end matches the beginning of the block we
really want.

   After that,  we fseek() ahead 900 integers in the index and get
the end of that block.  This leaves us,  on average,  with only
1/1440 of the zone file to search.  (In practice,  I don't think
it makes a lot of difference,  but doing the binary search in a
smaller section of the zone file may speed matters up slightly.)
*/

int extract_urat1_stars( FILE *ofile, const double ra, const double dec,
                  const double width, const double height, const char *path,
                  const int output_format)
{
   const double dec1 = dec - height / 2., dec2 = dec + height / 2.;
   const double ra1 = ra - width / 2., ra2 = ra + width / 2.;
   const double zone_height = .2;    /* zones are .2 degrees each */
   int zone = (int)( (dec1  + 90.) / zone_height) + 1;
   const int end_zone = (int)( (dec2 + 90.) / zone_height) + 1;
   const int index_ra_resolution = 1440;  /* = .25 degrees */
   int ra_start = (int)( ra1 * (double)index_ra_resolution / 360.);
   int rval = 0;
   const int buffsize = 400;     /* read this many stars at a try */
   FILE *index_file = get_urat1_index_file( path);
   URAT1_STAR *stars = (URAT1_STAR *)calloc( buffsize, sizeof( URAT1_STAR));

   if( !stars)
      rval = -1;
   if( zone < 1)
      zone = 1;
   if( ra_start < 0)
      ra_start = 0;
   while( rval >= 0 && zone <= end_zone)
      {
      FILE *ifile = get_urat1_zone_file( zone, path);

      if( ifile)
         {
         int keep_going = 1;
         int i, n_read;
         const int32_t max_ra  = (int32_t)( ra2 * 3600. * 1000.);
         const int32_t min_ra  = (int32_t)( ra1 * 3600. * 1000.);
         const int32_t min_spd = (int32_t)( (dec1 + 90.) * 3600. * 1000.);
         const int32_t max_spd = (int32_t)( (dec2 + 90.) * 3600. * 1000.);
         long offset, end_offset;
         const long acceptable_limit = 40;
         long index_file_offset = get_index_file_offset( zone, ra_start);
         static long cached_index_data[3] = {-1L, 0L, 0L};
         const uint32_t ra_range = (uint32_t)( 360 * 3600 * 1000);
         uint32_t ra_lo = (uint32_t)( ra_start * (ra_range / index_ra_resolution));
         uint32_t ra_hi = ra_lo + ra_range / index_ra_resolution;

         if( index_file_offset == cached_index_data[0])
            {
            offset = cached_index_data[1];
            end_offset = cached_index_data[2];
            }
         else
            {
            if( index_file)
               {
               uint32_t loc;
               size_t count;

               cached_index_data[0] = index_file_offset;
               if( !ra_start)
                  offset = 0;
               else
                  {
                  fseek( index_file,
                              index_file_offset - 900 * sizeof( int32_t),
                              SEEK_SET);
                  count = fread( &loc, sizeof( int32_t), 1, index_file);
                  assert( count == 1);
                  offset = (long)loc;
                  }
               cached_index_data[1] = offset;
               fseek( index_file, index_file_offset, SEEK_SET);
               count = fread( &loc, sizeof( int32_t), 1, index_file);
               assert( count == 1);
               cached_index_data[2] = end_offset = (long)loc;
               }
            else     /* no index:  binary-search within entire zone: */
               {
               offset = 0;
               fseek( ifile, 0L, SEEK_END);
               end_offset = ftell( ifile) / sizeof( URAT1_STAR);
               ra_lo = 0;
               ra_hi = ra_range;
               }
            }
                     /* Secant-search within the known limits: */
         while( end_offset - offset > acceptable_limit)
            {
            size_t count;
            URAT1_STAR star;
            long delta = end_offset - offset, toffset;
            long minimum_bite = delta / 8 + 1;
            long tval = (long)( (int64_t)delta * (int64_t)( min_ra - ra_lo)
                          / (int64_t)( ra_hi - ra_lo));
                              /* the above could overflow 32 bits */

            if( tval < minimum_bite)
               tval = minimum_bite;
            else if( tval > delta - minimum_bite)
               tval = delta - minimum_bite;
            toffset = offset + (uint32_t)tval;
            fseek( ifile, toffset * sizeof( URAT1_STAR), SEEK_SET);
            count = fread( &star, sizeof( URAT1_STAR), 1, ifile);
            assert( count == 1);
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
         fseek( ifile, offset * sizeof( URAT1_STAR), SEEK_SET);

         while( (n_read = (int)fread( stars, sizeof( URAT1_STAR), buffsize, ifile)) > 0
                                                   && keep_going)
            for( i = 0; i < n_read && keep_going; i++)
               {
               URAT1_STAR star = stars[i];

#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
               flip_urat1_star( &star);
#endif
#endif
               if( star.ra > max_ra)
                  keep_going = 0;
               else if( star.ra > min_ra && star.spd > min_spd
                                           && star.spd < max_spd)
                  {
                  rval++;
                  if( ofile)
                     {
                     if( output_format & URAT1_RAW_BINARY)
                        fwrite( &star, 1, sizeof( URAT1_STAR), ofile);
                     else
                        {
                        char buff[URAT1_ASCII_SIZE];

                        write_urat1_star( zone, offset + 1, buff, &star,
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
   if( index_file)
      fclose( index_file);
   free( stars);

            /* We need some special handling for cases where the area
               to be extracted crosses RA=0 or RA=24: */
   if( rval >= 0 && ra > 0. && ra < 360.)
      {
      if( ra1 < 0.)      /* left side crosses over RA=0h */
         rval += extract_urat1_stars( ofile, ra+360., dec, width, height,
                                          path, output_format);
      if( ra2 > 360.)    /* right side crosses over RA=24h */
         rval += extract_urat1_stars( ofile, ra-360., dec, width, height,
                                          path, output_format);
      }
   return( rval);
}
