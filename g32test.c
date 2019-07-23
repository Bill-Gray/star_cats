#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "gaia32.h"

      /* The following code shows how 'gaia32.cpp' can be used.  Run it as

         g32test (RA) (dec) (width) (height)

         (all arguments in decimal degrees) and it will extract all
         Gaia32 stars within the specified rectangle.  Optionally,  you
         can add a "path" argument.  The data is written to the file
         'gaia32.txt',  and the total number of stars extracted is shown
         on the console. */

static void show_error_message( void)
{
   printf( "'g32test.exe' takes as command-line arguments the RA/dec of the\n");
   printf( "center of the region to be extracted;  its width and height;\n");
   printf( "and,  optionally,  the path to the Gaia32 data.  All angles\n");
   printf( "are in decimal degrees,  though six digits and a decimal will\n");
   printf( "be considered HHMMSS or ddmmss values.  For examples,  either of:\n\n");
   printf( "g32test 50 -16.3 2 1.5 d:\\g32\n");
   printf( "g32test 032000.0 -161800.00 2 1.5 d:\\g32\n\n");
   printf( "would extract a 2-degree wide,  1.5-degree high area centered\n");
   printf( "on RA=50 degrees=3h20m, dec=-16.3,  with the data drawn from\n");
   printf( "the path d:\\g32.  Data will be written to the file 'gaia32.txt'.\n");
   printf( "\nOptionally, one may add command line options -h to include a header\n");
   printf( "line,  and/or -f4 to get the same output as from the FORTRAN code.\n");
}

static double extract_ra_dec( const char *str, const bool is_ra)
{
   double rval;
   bool is_negative = false;

   if( *str == '-')
      {
      is_negative = true;
      str++;
      }
   else if( *str == '+')
      str++;
   if( strlen( str) > 5 && str[6] == '.')
      {
      const long seconds = atol( str);

      rval = (double)( seconds / 10000L)
                  + (double)( (seconds / 100L) % 100L) / 60.
                  + atof( str + 4) / 3600.;
      if( is_ra)           /* cvt hours to degrees */
         rval *= 15.;
      }
   else
      rval = atof( str);
   if( is_negative)
      rval = -rval;
   return( rval);
}

static const char *usual_header = "  32G          RA     %s        dec"
            "     Gmag sGmag  epoch  dRA dde     pmRA    pmDec    pmSigmas\n";

int main( int argc, const char **argv)
{
   int rval = -9, i, j, show_debug_data = 0;
   unsigned format = GAIA32_WRITE_SPACES;
   int show_header = 0;

   for( i = 1; i < argc; i++)
      if( argv[i][0] == '-' && argv[i][1] > '9')
         {
         switch( argv[i][1])
            {
            case 'h': case 'H':
               show_header = 1;
               break;
            case 'f': case 'F':
               sscanf( argv[i] + 2, "%x", &format);
               break;
            case 't': case 'T':
               show_debug_data = 1;
               break;
            default:
               printf( "%s is an unrecognized option\n", argv[i]);
               break;
            }
         for( j = i; j < argc - 1; j++)      /* remove this argument */
            argv[j] = argv[j + 1];
         i--;
         argc--;
         }

   if( argc < 5)
      show_error_message( );
   else
      {
      FILE *ofile = fopen( "gaia32.txt", "wb");

      if( !ofile)
         {
         printf( "Couldn't open gaia32.txt\n");
         show_error_message( );
         return( -1);
         }
      if( show_header)
         fprintf( ofile, usual_header,
                   (format & GAIA32_BASE_60) ? "   " : "");
      rval = extract_gaia32_stars( ofile,
                               extract_ra_dec( argv[1], true),
                               extract_ra_dec( argv[2], false),
                               atof( argv[3]), atof( argv[4]),
                               (argc == 5 ? "" : argv[5]), format);

      if( show_debug_data)
         {
         extern clock_t time_searching;

         printf( "%.2lf seconds elapsed\n",
               (double)clock( ) / (double)CLOCKS_PER_SEC);
         printf( "%.2lf seconds indexing\n",
               (double)time_searching / (double)CLOCKS_PER_SEC);
         printf( "%d stars extracted\n", rval);
         }
      fclose( ofile);
      }
   return( rval);
}
