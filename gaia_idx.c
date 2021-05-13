#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include "gaia32.h"

/* The Gaia indexing scheme may seem a little strange at first.  Bear
with me;  there is method behind the madness.

Most star catalog indexing schemes that I've seen work roughly as
follows.  The stars are broken into zones,  and then each zone is
broken into RA bands.  For example,  each zone might be considered
as 1440 bands,  each 1/4 degree = 3.75 RA minutes wide.  If your
RA is,  say,  314.159 degrees,  you're in band 314.159*4 = 1256
(truncating).  The corresponding index entry tells you where to
seek within the file.  If you really want to get clever,  you can
look at index entry 1257 and say "the data I want starts between
this file offset and this file offset",  and you could binary
search or secant search within that.

This would work pretty well if stars were evenly spread across
the sky.  They are not,  and especially in Gaia,  you may look
at index entries 1256 and 1257 and realize that you still have
a lot of file to search.  (Meanwhile,  entries 256 and 257 may
be right next to each other.)  So I decided that I would create
a Gaia index that worked in terms of the nth star per zone
(the variable 'spacing' in the following code,  set by the first
command line argument.)  Set 'spacing' to 100000,  and the index
will tell you the RA of every 100000th star in the zone.

As a result,  the index size for a particular zone is a function
of how many stars are in that zone.  For example,  if you run
this code as ./gaia_idx 200000 -r8,8 -v2 ("only process zone 8,
spacing is 200000,  full verbosity") it will output

Opening '008.cat' : 915973 stars
   loc 200000 : RA 444786983 (123.551940)
   loc 400000 : RA 651195068 (180.887519)
   loc 600000 : RA 823649466 (228.791518)
   loc 800000 : RA 1040241037 (288.955844)

The first 200K stars cover RAs 0 to 123.55 degrees.  The next
200K keep going to RA=180.88 degrees,  and so on.  Implicitly,
there are first and last lines

   loc 0 : RA 0 (0.000000)
   loc 915973 : RA 1296000000 (360.000000)

Another departure from "standard practice" : there is a single
index for all zones (instead of a .acc file for each zone) and
the indices are binary,  for speed and compactness.

Physically,  the index is a series of four-byte integers.
The first is the magic number 0xfa1a3202 (because 0xgaia3202
would have two non-hexadecimal digits).  The second is unused
at present,  and the third gives the 'spacing' parameter.  I
expect to play around a bit to see what spacing works best.
(The file size of the index will be about 6.6GBytes / spacing,
plus overhead.)

After the three integers for the header are 180 integers for
the number of stars per zone (i.e.,  the ninth such integer
will be the 915973 stars shown above).  Following that are the
per-zone integers shown above;  for zone 8,  four integers
(the RAs in the above table) will be written.

'gaia32.c' has logic to read in the 183-byte header,  then make
use of it to figure out where in the index to get the offsets
within each zone file.  A Gaia32-stored star takes 28 bytes;
this code run with spacing = 100K will therefore give you an
index that always gets you to within 2.8 MBytes of the actual
starting RA.

One could binary-search within that 2.8 MBytes.  'gaia32.h'
does a secant search,  modified to always reduce the search
area by 1/8 on each step (to avoid perverse behavior where
you'd make lots of tiny steps).

The above may seem like excessive optimization.  In some cases,
such as extracting an area of stars for astrometric reduction
of an image,  it won't matter.  But 'gaia_ast' makes lots of
small extractions from the data,  potentially scattered
widely.  In such cases,  performance becomes a real issue. */

static void error_exit( void)
{
   fprintf( stderr,
            "'gaia_idx' takes,  as a command line argument,  a 'spacing'\n"
            "parameter.  I used 10000 for the index supplied with this\n"
            "software,  which appears to be reasonably fast while keeping\n"
            "the index small.  See 'gaia_idx.c' for details on how the\n"
            "indexing works and what the 'spacing' means.  The program must\n"
            "be run in the directory containing the compact Gaia data.\n");
   exit( -1);
}

int main( const int argc, const char **argv)
{
   const int end_zone = 179;
   const int spacing = (argc > 1 ? atoi( argv[1]) : 0);
   int i, verbose = 0, zone = 0;
   FILE *ifile, *ofile = fopen( "gaia.idx", "wb");
   int32_t header[3], sizes[180];
   size_t n;

   if( spacing < 100)
      {
      fprintf( stderr, "Invalid or no spacing specified\n");
      error_exit( );
      }
   for( i = 2; i < argc; i++)
      if( argv[i][0] == '-')
         switch( argv[i][1])
            {
            case 'v':
               verbose = 1 + atoi( argv[i] + 2);
               break;
            }
   header[0] = 0xfa1a3202;    /* magic number;  2 at end means DR2 */
   header[1] = 0;             /* future use */
   header[2] = (int32_t)spacing;
   n = fwrite( header, sizeof( int32_t), 3, ofile);
   assert( n == 3);
   for( i = 0; i < 180; i++)
      sizes[i] = 0;
   n = fwrite( sizes, sizeof( int32_t), 180, ofile);
   assert( n == 180);
   while( zone <= end_zone)
      {
      char filename[10];

      snprintf( filename, sizeof( filename), "%03d.cat", zone);
      ifile = fopen( filename, "rb");
      if( !ifile)
         {
         fprintf( stderr, "Couldn't open Gaia data file '%s' : %s\n",
                     filename, strerror( errno));
         error_exit( );
         }
      fseek( ifile, 0L, SEEK_END);
      sizes[zone] = (int32_t)( ftell( ifile) / sizeof( GAIA32_STAR));
      if( verbose)
         printf( "Opening '%s' : %ld stars\n", filename, (long)sizes[zone]);
      for( i = spacing; i < sizes[zone]; i += spacing)
         {
         GAIA32_STAR istar;
         const int32_t max_ra = 360 * 3600 * 1000;

         fseek( ifile, i * sizeof( GAIA32_STAR), SEEK_SET);
         n = fread( &istar, sizeof( GAIA32_STAR), 1, ifile);
         assert( n == 1);
         n = fwrite( &istar.ra, sizeof( int32_t), 1, ofile);
         assert( n == 1);
         if( verbose > 1)
            printf( "   loc %d : RA %ld (%f)\n", i,
                           (long)istar.ra,
                           (double)istar.ra * 360. / (double)max_ra);
         }
      fclose( ifile);
      zone++;
      }
               /* We now have accurate file sizes;  rewrite the header : */
   fseek( ofile, 12L, SEEK_SET);
   n = fwrite( sizes, sizeof( int32_t), 180, ofile);
   assert( n == 180);
   fclose( ofile);
   printf( "'gaia.idx' index created\n");
   return( 0);
}
