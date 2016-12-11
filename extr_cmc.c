#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "cmc1x.h"

      /* The following code shows how the CMC-1x code can be used.  Run it as

         extr_cmc (RA) (dec) (width) (height)

         (all arguments in decimal degrees) and it will extract all
         CMC1x stars within the specified rectangle.  Optionally,  you
         can add a "path" argument.  The data is written to the file
         'cmc.txt',  and the total number of stars extracted is shown
         on the console. */

static void show_error_message( void)
{
   printf( "'extr_cmc.exe' takes as command-line arguments the RA/dec of the\n");
   printf( "center of the region to be extracted;  its width and height;\n");
   printf( "and,  optionally,  the path to the CMC-15 or CMC-14 data.  All angles\n");
   printf( "are in decimal degrees.  For example:\n\n");
   printf( "extr_cmc 50 -16.3 2 1.5 d:\\u2\n\n");
   printf( "would extract a 2-degree wide,  1.5-degree high area centered\n");
   printf( "on RA=50 degrees=3h20m, dec=-16.3,  with the data drawn from\n");
   printf( "the path d:\\u2.  Data will be written to the file 'cmc.txt'.\n");
}

int main( int argc, char **argv)
{
   int rval = -9;

   if( argc < 5)
      show_error_message( );
   else
      {
      FILE *ofile = fopen( "cmc.txt", "w");

      rval = extract_cmc1x_stars( ofile, atof( argv[1]), atof( argv[2]),
                                            atof( argv[3]), atof( argv[4]),
                                            (argc > 5 ? argv[5] : ""), 0);

      printf( "%d stars extracted\n", rval);
      fclose( ofile);
      }
   return( rval);
}
