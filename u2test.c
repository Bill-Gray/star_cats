#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "ucac2.h"

/* The following code shows how the UCAC2 code can be used,  and
is a utility for extracting RA/dec rectangles of UCAC2 data to an
ASCII file. */

#if defined( __linux__) || defined( __unix__) || defined( __APPLE__)
   #define example_path "/mnt/cdrom/u2"
#else
   #define example_path "d:\\u2"
#endif

static void show_error_message( void)
{
   printf( "'u2test' takes as command-line arguments the RA/dec of the\n"
           "center of the region to be extracted;  its width and height;\n"
           "and,  optionally,  the path to the UCAC-2 data.  All angles\n"
           "are in decimal degrees.  For example:\n\n"
           "ucac2 50 -16.3 2 1.5 " example_path "\n\n"
           "would extract a 2-degree wide,  1.5-degree high area centered\n"
           "on RA=50 degrees=3h20m, dec=-16.3,  with the data drawn from\n"
           "the path " example_path ".  Data will be written to the file 'ucac2.txt'.\n");
}

int main( const int argc, const char **argv)
{
   int pass, rval = -9;

   if( argc == 2 || argc == 3)
      {
      UCAC2_STAR star;
      const long ucac2_number = atol( argv[1]);

      rval = extract_ucac2_info( ucac2_number, &star,
                                (argc == 2 ? "" : argv[2]));
      if( !rval)
         {
         char buff[200];

         write_ucac2_star( ucac2_number, buff, &star);
         printf( "%s", buff);
         }
      else
         printf( "Couldn't get data : err code %d\n", rval);
      }
   else if( argc < 5)
      show_error_message( );
   else
      {
      FILE *ofile = fopen( "ucac2.txt", "w");

      for( pass = 0; pass < 2; pass++)
         {
         rval = extract_ucac2_stars( ofile, atof( argv[1]), atof( argv[2]),
                                            atof( argv[3]), atof( argv[4]),
                                            (argc > 5 ? argv[5] : ""), pass);

         printf( "%d stars extracted\n", rval);
         }
      fclose( ofile);
      }
   return( rval);
}
