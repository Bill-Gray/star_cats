#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "cmc1x.h"

/* A convenience function to extract the RA/dec from an ASCII CMC-14
or CMC-15 record,  in the original units of .0001 second of RA and .001
arcsec of declination. */

static void get_cmc1x_ra_dec( long *ra, long *dec, const char *buff)
{
   if( ra)
      {
      *ra = atoi( buff + 16) * 3600 + atoi( buff + 19) * 60 + atoi( buff + 22);
      *ra = *ra * 10000 + atoi( buff + 25);
      }
   if( dec)
      {
      *dec = atoi( buff + 31) * 3600 + atoi( buff + 34) * 60 + atoi( buff + 37);
      *dec = *dec * 1000 + atoi( buff + 40);
      if( buff[30] == '-')
         *dec = -(*dec);
      }
}

/* extract_cmc1x_stars( ),  shown below,  handles both ASCII and binary
cmc-1x records.  If the input record is binary,  xvt_record( ) converts it
back to ASCII.   */

static void xvt_record( char *tbuff, const int record_size)
{
   if( record_size == CMC1x_BINARY_RECORD_SIZE)
      {
      CMC1x_REC star;

      cmc1x_binary_rec_to_struct( &star, tbuff);
      cmc1x_struct_to_ascii( tbuff, &star);
      }
}

/* extract_cmc1x_stars() finds all CMC-1x stars within the specified RA/dec
rectangle,  and writes out the ASCII records for them to the specified
output file.  It will do this from either the ASCII or binary files;
in either case,  the ASCII record is written out.  It defaults to looking
for the cmc1x*.dat or cmc1x*.cmc files in the current directory,  then
looks for them in the directory specified by 'path'.  If the 'rejected'
flag is TRUE,  then it will search the 'rejected star' files instead of
the main catalog files.

   The return value is the number of extracted stars.

   The logic is as follows:

   (1) Figure out which zones cover the rectangle's range in declination,
and open each of them in order.

   (2) Within that zone,  do a binary search to find the first record
with right ascension less than or equal to the "low" end of the rectangle's
range in right ascension.

   (3) Start reading records and writing out those falling within the
desired rectangle,  until you either run out of records or find one with
an RA greater than the "high" end of the rectangle's RA range.

   (4) When you're done with all this,  check to see if the rectangle
projects past RA=0 or RA=24 hours=360 degrees.  If so,  recurse with a
suitably shifted RA.
*/

int extract_cmc1x_stars( FILE *ofile, const double ra_in_degrees,
                  const double dec_in_degrees,
                  const double width_in_degrees,
                  const double height_in_degrees,
                  const char *path, const int rejected)
{
            /* Rectangle runs from ra1 to ra2,  and from dec1 to dec2: */
   const double dec1 = dec_in_degrees - height_in_degrees / 2.;
   const double dec2 = dec_in_degrees + height_in_degrees / 2.;
   const double ra1 = ra_in_degrees - width_in_degrees / 2.;
   const double ra2 = ra_in_degrees + width_in_degrees / 2.;
   const double zone_height = 5.;   /* CMC-1x zones are five degrees high */
   int zone = (int)( (dec1  + 90.) / zone_height) + 1;
   const int end_zone = (int)( (dec2 + 90.) / zone_height) + 1;
            /* RA is stored in .0001 seconds, dec in .001 arcseconds: */
   long ra_start = (long)( ra1 * 10000. * 3600. / 15.);
   const long ra_end = (long)( ra2 * 10000. * 3600. / 15.);
   const long dec_start = (long)( dec1 * 1000. * 3600);
   const long dec_end = (long)( dec2 * 1000. * 3600);
   char tbuff[103];
   int record_size = 0;
   int rval = 0, pass;
   const char *path_separator = "/", *read_only_permits = "r";

   if( zone < 0)
      zone = 0;
   if( ra_start < 0)
      ra_start = 0;
   while( rval >= 0 && zone <= end_zone)
      {
      FILE *ifile = NULL;
      char base_name[40];

      if( zone < 19)             /* southern hemisphere zone */
         sprintf( base_name, "cmc15s%x", 18 - zone);
      else                       /* northern hemisphere zone */
         sprintf( base_name, "cmc15n%x", zone - 19);
      if( rejected)
         strcat( base_name, "r");
                  /* Look for eight different possible incarnations of the */
                  /* CMC-14 and CMC-15 data:                               */
                  /* (1) CMC-15,  binary,  current path;                   */
                  /* (2) CMC-15,  ASCII,  current path;                    */
                  /* (3) CMC-15,  binary,  path specified in 'path' param; */
                  /* (4) CMC-15,  ASCII,  path specified in 'path' param;  */
                  /* (5) CMC-14,  binary,  current path;                   */
                  /* (6) CMC-14,  ASCII,  current path;                    */
                  /* (7) CMC-14,  binary,  path specified in 'path' param; */
                  /* (8) CMC-14,  ASCII,  path specified in 'path' param.  */
      for( pass = 0; !ifile && pass < 8; pass++)
         {
         char filename[180];

         base_name[4] = ((pass & 4) ? '4' : '5');  /* select CMC-15 vs. 14 */
         if( (pass & 3) < 2 || !path)     /* search local path */
            *filename = '\0';
         else                             /* search specified path */
            {
            strcpy( filename, path);
            if( filename[strlen( filename) - 1] != path_separator[0])
               strcat( filename, path_separator);
            }
         strcat( filename, base_name);
                  /* ASCII files are assumed to have extension .dat.    */
                  /* Binary files are assumed to have extension .cmc.   */
         strcat( filename, (pass & 1) ? ".cmc" : ".dat");
         ifile = fopen( filename, read_only_permits);
         if( ifile)
            record_size = ((pass & 1) ? CMC1x_BINARY_RECORD_SIZE :
                                        CMC1x_ASCII_RECORD_SIZE);
         }
                  /* Failure to find a file is _not_ an error condition. */
                  /* Could be,  for example,  we're outside the CMC-1x   */
                  /* coverage area (decs -30 to +50).                    */
      if( ifile)
         {
         int n_recs, loc = 0, loc1, step;
         long ra, dec;

         fseek( ifile, 0L, SEEK_END);
         n_recs = ftell( ifile) / record_size;
                        /* Do a binary search on the zone file,  to find  */
                        /* the first record with ra >= ra_start.          */
         for( step = 0x8000000; step; step >>= 1)
            if( (loc1 = loc + step) < n_recs)
               {
               fseek( ifile, loc1 * record_size, SEEK_SET);
               if( fread( tbuff, record_size, 1, ifile) != 1)
                  {
                  fclose( ifile);
                  return( 0);
                  }
               xvt_record( tbuff, record_size);
               get_cmc1x_ra_dec( &ra, NULL, tbuff);
               if( ra < ra_start)
                  loc = loc1;
               }
                     /* Now seek to that record,  and start reading records */
                     /* until we've gone past the RA region of interest.    */
                     /* If the records fall within the RA/dec rectangle,    */
                     /* write out the result to 'ofile'.                    */
         fseek( ifile, loc * record_size, SEEK_SET);
         ra = 0;
         while( ra < ra_end &&
                         fread( tbuff, record_size, 1, ifile))
            {
            xvt_record( tbuff, record_size);
            get_cmc1x_ra_dec( &ra, &dec, tbuff);
            if( ra >= ra_start && ra < ra_end && dec > dec_start && dec < dec_end)
               {        /* This record falls in the RA/dec rectangle: */
               fwrite( tbuff, CMC1x_ASCII_RECORD_SIZE, 1, ofile);
               rval++;
               }
            }
         fclose( ifile);
         }
      zone++;
      }

            /* If the area to be extracted crosses RA=0 or RA=24,  we */
            /* recurse to pick up the data on the "other side" of the */
            /* prime meridian:                                        */
   if( rval >= 0 && ra_in_degrees > 0. && ra_in_degrees < 360.)
      {
      if( ra1 < 0.)      /* left side crosses over RA=0h */
         rval += extract_cmc1x_stars( ofile, ra_in_degrees+360., dec_in_degrees,
                                    width_in_degrees, height_in_degrees,
                                    path, rejected);
      if( ra2 > 360.)    /* right side crosses over RA=24h */
         rval += extract_cmc1x_stars( ofile, ra_in_degrees-360., dec_in_degrees,
                                    width_in_degrees, height_in_degrees,
                                    path, rejected);
      }
   return( rval);
}

