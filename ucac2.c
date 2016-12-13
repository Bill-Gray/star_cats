#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "ucac2.h"

/* History: */

/*    25 Sep 2009: (BJG) Made the 'write_ucac2_star' function global.
   I needed to access it from a program that matches UCAC-2 to UCAC-3,
   looking for stars in the former that didn't make it into the newer
   catalog.          */

/*     3 Jan 2007: (BJG) Added a function to get the UCAC-2 data for a
   specific UCAC-2 numbered star.  Also,  I found that all stars in
   'z000' (zone from the south celestial pole to dec -89.5) were
   given wrong numbers in the output from 'extract_ucac2_stars()'
   (offset by one),  and fixed this.   */

/*    22 Jan 2004: (BJG) Revised to handle the Bright Star Supplement.
   That required looking at a different index and different files with
   different heights,  and displaying epochs to .01 year precision
   instead of .001 year precision.  I've given BSS stars UCAC2 numbers
   starting at 50000000 (fifty million),  to distinguish them from
   "normal" UCAC2 stars.  extract_ucac2_stars() now takes a Boolean
   argument to distinguish between "extract from 'normal' UCAC-2" and
   "extract from the supplement".

      Also,  since the BSS includes some negative-magnitude stars,
   I had to add some 'abs()' statements.

      Also,  the test main() routine now has to call extract_ucac2_stars()
   twice:  once to get 'normal' UCAC-2 stars,  then a second time to get
   supplement stars.  Most programs using this source ought to do the same. */

/*    24 Oct 2003:  (BJG)  While running a little program to extract
   data for high proper-motion stars from UCAC2,  I learned that in
   some cases,  an extra digit is needed for the RA,  dec,  and
   "goodness of fit" parameters for proper motion.  (The latter can,
   on rare conditions,  be as high as 1250,  thus requiring four
   digits instead of the usual three.)  Note that this means each
   field expands by one character,  and the total line length expands
   by four bytes.  */

/*     4 Aug 2003:  (BJG)  Revised to have goodness of fit run from 0
   to 100,  instead of 0 to 20,  reflecting the fact that it's given
   in .05 units. */

/*    17 May 2003:  (BJG)  Renamed extract_ucac_stars() to
   extract_ucac2_stars() to evade confusion with UCAC-1.  Altered same
   function so that the 'path' to the data can end in a path separator;
   i.e.,  either d:\u2\ or d:\u2 would work on most DOS/Windows boxes.
   Put an #ifdef TEST_PROGRAM around the test main()... necessary because
   I'm gonna use this code in Charon and Guide and elsewhere. */

/* 13 Jun 2003:  (BJG) Learned that in the final format,  the "reference
   flag" byte is replaced by a "position error" one,  with the same 127
   offset.  So in the structure def for UCAC2_STAR,  changed "reference_flag"
   to "e_posn",  and modified "write_ucac2_star()" accordingly... this
   does mean the ASCII output has an extra byte in it,  and everything
   afterward is pushed over by one column. */

/* 13 Jun 2003: (BJG) 'u2index.unf' has been replaced by 'u2index.da',
   with a much more straightforward layout.  Changed the code accordingly. */

/* 16 Jun 2003: (BJG) Run the test program without enough command line
   data,  and it gives you an error message/usage directions.           */

/* On non-Intel-ordered (big-Endian) machines,  we need the 'swap_32',
   'swap_16', and 'flip_ucac2_star' functions. */

static long ucac2_offsets[289] = {
        0,      875,     3545,     8116,    14437,    22600,
    32331,    43410,    56200,    70717,    86739,   104139,
   122349,   142651,   164289,   187090,   211870,   238034,
   265375,   294799,   327491,   363063,   400192,   441762,
   484531,   527748,   572184,   621122,   673488,   730594,
   790713,   855264,   924503,   998122,  1076967,  1159688,
  1247343,  1341270,  1438985,  1547334,  1664224,  1788919,
  1922104,  2063014,  2208024,  2355622,  2504839,  2663318,
  2839845,  3024501,  3218641,  3416550,  3614242,  3823195,
  4029562,  4236521,  4458436,  4700900,  4959345,  5229516,
  5484377,  5734942,  5987107,  6234934,  6478565,  6713669,
  6947728,  7188195,  7428973,  7669036,  7917496,  8175245,
  8429983,  8685256,  8930815,  9167166,  9396194,  9627414,
  9860545, 10092663, 10320862, 10551285, 10774361, 10998590,
 11227599, 11453953, 11678371, 11908501, 12134024, 12354875,
 12585217, 12819083, 13046418, 13265014, 13482010, 13683843,
 13890306, 14107275, 14323849, 14533480, 14745194, 14964881,
 15191793, 15422295, 15653737, 15875063, 16088251, 16300592,
 16514023, 16735917, 16954676, 17177755, 17403706, 17628453,
 17856124, 18086130, 18321145, 18564424, 18810057, 19053837,
 19304846, 19557852, 19808606, 20059141, 20296845, 20532251,
 20763595, 20995878, 21222631, 21447796, 21675333, 21906169,
 22133624, 22368349, 22602503, 22824927, 23039674, 23277827,
 23528066, 23775599, 24016899, 24257144, 24501108, 24748699,
 24995171, 25231017, 25450607, 25677943, 25907511, 26135851,
 26364200, 26588604, 26803907, 27019266, 27219606, 27405577,
 27590917, 27770924, 27948516, 28125012, 28301884, 28475469,
 28654401, 28840950, 29027318, 29208324, 29386615, 29572897,
 29761158, 29943107, 30122203, 30294911, 30459034, 30623077,
 30784133, 30937915, 31094635, 31260348, 31434082, 31613516,
 31794346, 31967487, 32133973, 32302278, 32474013, 32640632,
 32807126, 32981497, 33159328, 33331374, 33503698, 33682890,
 33860265, 34035932, 34212619, 34389277, 34574252, 34764824,
 34950061, 35135902, 35321341, 35504346, 35683742, 35862760,
 36045226, 36224598, 36398377, 36574893, 36752129, 36930520,
 37111989, 37294415, 37472017, 37645337, 37824357, 38004525,
 38187498, 38368241, 38547410, 38727431, 38907261, 39078865,
 39248680, 39421447, 39601092, 39780761, 39954653, 40122325,
 40286803, 40453466, 40625043, 40793811, 40960542, 41132008,
 41304350, 41476052, 41648355, 41821084, 41996487, 42171604,
 42344935, 42518638, 42692706, 42865927, 43036597, 43207721,
 43380289, 43555473, 43727481, 43899332, 44071847, 44243601,
 44418909, 44596123, 44768425, 44936690, 45103771, 45273665,
 45437507, 45596873, 45753189, 45911835, 46068626, 46213812,
 46352739, 46490280, 46624345, 46751682, 46876305, 47000092,
 47124389, 47246439, 47360659, 47469968, 47575715, 47676545,
 47771271, 47856394, 47932606, 48005366, 48071224, 48126923,
 48173418, 48219131, 48260062, 48295089, 48319510, 48329822,
 48330571 };

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

void flip_ucac2_star( UCAC2_STAR *star)
{
   swap_32( &star->ra);
   swap_32( &star->dec);
   swap_16( &star->mag);
   swap_16( &star->epoch_ra);
   swap_16( &star->epoch_dec);
   swap_32( &star->pm_ra);
   swap_32( &star->pm_dec);
   swap_32( &star->twomass_id);
   swap_16( &star->j_mag);
   swap_16( &star->h_mag);
   swap_16( &star->ks_mag);
}
#endif                   // #if __BYTE_ORDER == __BIG_ENDIAN
#endif                   // #ifdef __BYTE_ORDER

/* The following function writes out a UCAC2 star in a standardized
   ASCII format that looks like this (column header plus sample lines):
UCAC2 ID       RA          dec      Rmag   RAepoch decepoch s_RA sd #o rf #c cf   pmRA  pmdec epr epd qpr qpd   2massid   Jmag   Hmag   Ksmag 2mph 2mc
        0  28.50131028  51.78109444 14.84 2002.104 2002.340  18  15  2 00  2 01     45    -29  37  37  20  20  931184455 13.796 13.403 13.327 000 000
        0  28.50290111  51.83046000 14.31 2000.120 2001.926  33  20  3 00  2 01   1062    154  38  37  20  20  931184614 13.486 13.167 12.996 220 000
*/

#define BSS_OFFSET 50000000

int write_ucac2_star( const long offset, char *obuff,
                                          const UCAC2_STAR *star)
{
   const int short_goodness_fmt = (star->q_pm_ra < 73 && star->q_pm_dec < 73);
   const int is_supplement = (offset > BSS_OFFSET);
   const long epoch_ra  = 1975000 + star->epoch_ra * (is_supplement ? 10 : 1);
   const long epoch_dec = 1975000 + star->epoch_dec * (is_supplement ? 10 : 1);

   sprintf( obuff, " %08ld %12.8lf %12.8lf %2d.%02d %4ld.%03ld %4ld.%03ld", offset,
                  (double)star->ra / 3600000., (double)star->dec / 3600000.,
                  star->mag / 100, abs( star->mag % 100),
                  epoch_ra / 1000L, epoch_ra % 1000L,
                  epoch_dec / 1000L, epoch_dec % 1000L);

   if( is_supplement)               /* epochs given to .01 year */
      obuff[49] = obuff[58] = ' ';

   sprintf( obuff + strlen( obuff),
            " %3d %3d %2d %3d %2d %02x %7ld %7ld %3d %3d ",
            star->e_ra + 127, star->e_dec + 127,
            star->nobs, star->e_posn + 127, star->ncat,
            (unsigned char) star->catalog_flag,
            (long)star->pm_ra, (long)star->pm_dec,
            star->e_pm_ra + 127, star->e_pm_dec + 127);

   sprintf( obuff + strlen( obuff),
                   short_goodness_fmt ? " %03d  %03d " : "%4d %4d ",
                   (star->q_pm_ra + 127) * 5, (star->q_pm_dec + 127) * 5);

   sprintf( obuff + strlen( obuff),
            "%10ld %2d.%03d %2d.%03d %2d.%03d %03d %03d\n",
            (long)star->twomass_id,
            star->j_mag / 1000, abs( star->j_mag % 1000),
            star->h_mag / 1000, abs( star->h_mag % 1000),
            star->ks_mag / 1000, abs( star->ks_mag % 1000),
            star->twomass_ph_qual + 127, star->twomass_cc_flag + 127);
   return( 0);
}

#if defined( __linux__) || defined( __unix__) || defined( __APPLE__)
   static const char *path_separator = "/", *read_only_permits = "r";
#elif defined( _WIN32) || defined( _WIN64) || defined( __WATCOMC__)
   static const char *path_separator = "\\", *read_only_permits = "rb";
#else
#error "Unknown platform; please report so it can be fixed!"
#endif


static FILE *get_ucac2_zone_file( const int zone_number,
              const int is_supplement, const char *path)
{
   FILE *ifile;
   const char *filename_format = (is_supplement ? "s%02d" : "z%03d");
   char filename[80];

   sprintf( filename, filename_format, zone_number);
   ifile = fopen( filename, read_only_permits);
   if( !ifile)
      {
      strcpy( filename, path);
      if( filename[strlen( filename) - 1] != path_separator[0])
         strcat( filename, path_separator);
      sprintf( filename + strlen( filename), filename_format, zone_number);
      ifile = fopen( filename, read_only_permits);
      }
   return( ifile);
}

#define UCAC2_BAD_STAR_NUMBER       -1
#define UCAC2_BAD_FSEEK             -2
#define UCAC2_BAD_FREAD             -3
#define UCAC2_BAD_FREAD2            -4
#define UCAC2_FILE_NOT_FOUND        -5

int extract_ucac2_info( const long ucac2_number, UCAC2_STAR *star,
                     const char *path)
{
   int zone, rval = 0;
   FILE *ifile;

   for( zone = 288; zone && ucac2_offsets[zone] + 1 > ucac2_number; zone--)
      ;
   if( zone == 288 || ucac2_number < 1)
      return( UCAC2_BAD_STAR_NUMBER);
   ifile = get_ucac2_zone_file( zone + 1, 0, path);
   if( !ifile)
      rval = UCAC2_FILE_NOT_FOUND;
   else
      {
      const long star_in_zone = ucac2_number - (ucac2_offsets[zone] + 1);

      if( fseek( ifile, star_in_zone * sizeof( UCAC2_STAR), SEEK_SET))
         rval = UCAC2_BAD_FSEEK;
      else if( fread( star, sizeof( UCAC2_STAR), 1, ifile) != 1)
         rval = UCAC2_BAD_FREAD;
#ifdef WRONG_ENDIAN
      flip_ucac2_star( &star);
#endif
      fclose( ifile);
      }
   return( rval);
}

int extract_ucac2_stars( FILE *ofile, const double ra, const double dec,
                  const double width, const double height, const char *path,
                  const int is_supplement)
{
   const double dec1 = dec - height / 2., dec2 = dec + height / 2.;
   const double ra1 = ra - width / 2., ra2 = ra + width / 2.;
   const double zone_height = (is_supplement ? 5. : .5);
   int zone = (int)( (dec1  + 90.) / zone_height) + 1;
   int end_zone = (int)( (dec2 + 90.) / zone_height) + 1;
   int ra_start = (int)( ra1 / 1.5);
   int rval = 0;
   FILE *index_file;
   const char *idx_filename = (is_supplement ? "bsindex.da" : "u2index.da");

                     /* Look for the index file in the local directory... */
   index_file = fopen( idx_filename, read_only_permits);
                     /* ...and if it's not there,  look for it in the same */
                     /* directory as the data: */
   if( !index_file)
      {
      char filename[100];

      strcpy( filename, path);
      if( filename[strlen( filename) - 1] != path_separator[0])
         strcat( filename, path_separator);
      strcat( filename, idx_filename);
      index_file = fopen( filename, read_only_permits);
      }

   if( zone < 0)
      zone = 0;
   if( ra_start < 0)
      ra_start = 0;
   while( rval >= 0 && zone <= end_zone)
      {
      FILE *ifile = get_ucac2_zone_file( zone, is_supplement, path);

      if( ifile)
         {
         int keep_going = 1;
         UCAC2_STAR star;
         const long ra2_in_mas = (long)( ra2 * 3600. * 1000.);
         int32_t offset0, offset;

         if( !index_file)
            offset0 = offset = 0;
         else
            {           /* 'u2index.da' gives the _ending_ offset for each */
                        /* zone.  So we have to do some odd things to find  */
                        /* the _beginning_ offset for each zone.            */
            if( zone == 1)
               offset0 = 0;
            else
               {
               fseek( index_file, ((zone - 1L) * 240L - 1L) * sizeof( int32_t),
                       SEEK_SET);
               if( fread( &offset0, sizeof( int32_t), 1, index_file) != 1)
                  rval = UCAC2_BAD_FREAD;
               }
            if( !ra_start)
               offset = offset0;
            else
               {
               fseek( index_file, ((zone - 1L) * 240L + ra_start - 1L) * sizeof( int32_t),
                       SEEK_SET);
               if( fread( &offset, sizeof( int32_t), 1, index_file) != 1)
                  rval = UCAC2_BAD_FREAD2;
               }

#ifdef WRONG_ENDIAN
            swap_32( &offset0);
            swap_32( &offset);
#endif
            }
         fseek( ifile, (offset - offset0) * sizeof( UCAC2_STAR), SEEK_SET);

         while( fread( &star, 1, sizeof( UCAC2_STAR), ifile) && keep_going)
            {
#ifdef WRONG_ENDIAN
            flip_ucac2_star( &star);
#endif
            if( star.ra > ra2_in_mas)
               keep_going = 0;
            else if( star.ra > (long)( ra1 * 3600. * 1000.) &&
                     star.dec > (long)( dec1 * 3600. * 1000.) &&
                     star.dec < (long)( dec2 * 3600. * 1000.))
               {
               char buff[200];

               write_ucac2_star( offset + 1L + (is_supplement ? BSS_OFFSET : 0),
                                       buff, &star);
               fwrite( buff, 1, strlen( buff), ofile);
               rval++;
               }
            offset++;
            }
         fclose( ifile);
         }
      zone++;
      }
   if( index_file)
      fclose( index_file);

            /* We need some special handling for cases where the area
               to be extracted crosses RA=0 or RA=24: */
   if( rval >= 0 && ra > 0. && ra < 360.)
      {
      if( ra1 < 0.)      /* left side crosses over RA=0h */
         rval += extract_ucac2_stars( ofile, ra+360., dec, width, height,
                                                      path, is_supplement);
      if( ra2 > 360.)    /* right side crosses over RA=24h */
         rval += extract_ucac2_stars( ofile, ra-360., dec, width, height,
                                                      path, is_supplement);
      }
   return( rval);
}

int extract_ucac2_stars_given_filename( const char *output_filename,
                  const double ra, const double dec,
                  const double width, const double height, const char *path,
                  const int is_supplement)
{
   FILE *ofile = fopen( output_filename, "wb");
   int rval;

   if( !ofile)
      rval = -1;
   else
      {
      rval = extract_ucac2_stars( ofile, ra, dec, width, height,
                                       path, is_supplement);
      fclose( ofile);
      }
   return( rval);
}

