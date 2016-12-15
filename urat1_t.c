#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "urat1.h"

      /* The following code shows how 'urat1_t.cpp' can be used.  Run it as

         urat1_t (RA) (dec) (width) (height)

         (all arguments in decimal degrees) and it will extract all
         URAT1 stars within the specified rectangle.  Optionally,  you
         can add a "path" argument.  The data is written to the file
         'urat1.txt',  and the total number of stars extracted is shown
         on the console.         */

/* 2015 Mar 18: fixes to eliminate compiler warning messages */

static void show_error_message( void)
{
   printf( "'urat1_t.exe' takes as command-line arguments the RA/dec of the\n");
   printf( "center of the region to be extracted;  its width and height;\n");
   printf( "and,  optionally,  the path to the URAT1 data.  All angles\n");
   printf( "are in decimal degrees.  For example:\n\n");
   printf( "urat1_t 50 -16.3 2 1.5 d:\\ur1\n\n");
   printf( "would extract a 2-degree wide,  1.5-degree high area centered\n");
   printf( "on RA=50 degrees=3h20m, dec=-16.3,  with the data drawn from\n");
   printf( "the path d:\\ur1.  Data will be written to the file 'urat1.txt'.\n");
   printf( "\nOptionally, one may add command line options -h to include a header\n");
   printf( "line,  and/or -f4 to get the same output as from the FORTRAN code.\n");
}

static const char *fortran_header =
"   RA         sdp   sigssigm nstnsu epoc mmag sigp nsmref nit niu ngt ngu pmr\
  pmd pme mfm mfa 2MASS_id  Jmag  Hmag  Kmag  sigJ sigH sigK _icc_ photo  Bmag\
  Vmag  gmag  rmag  imag sigB sigV sigg sigr sigi ann ano";

static const char *usual_header = "  URAT1        RA             dec   sigs\
 sigmnst nsu epoch   mmag sigp nsmref nit niungt ngu pmRA pmDec pme mfmmfa\
  2MASS_ID  magH   sig IC ph  magJ   sig IC ph  magK   sig IC fl mag_B  sig\
  mag_V  sig  mag_g  sig  mag_r  sig  mag_i  sig ann ano";

int main( int argc, const char **argv)
{
   int rval = -9, i, j, show_debug_data = 0;
   unsigned format = URAT1_WRITE_SPACES;
   bool show_header = false;

   for( i = 1; i < argc; i++)
      if( argv[i][0] == '-' && argv[i][1] > '9')
         {
         switch( argv[i][1])
            {
            case 'h': case 'H':
               show_header = true;
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

   if( argc == 2 || argc == 3)
      {
      URAT1_STAR star;
      int zone;
      long offset;

      if( sscanf( argv[1], "%d-%ld", &zone, &offset) != 2)
         printf( "Couldn't parse '%s' as a URAT1 ID\n", argv[1]);
      else
         {
         rval = extract_urat1_info( zone, offset, &star,
                                (argc == 2 ? "" : argv[2]));

         if( !rval)
            {
            char buff[URAT1_ASCII_SIZE];

            write_urat1_star( zone, offset, buff, &star, format);
            if( show_header)
               printf( "%s\n", (format & URAT1_FORTRAN_STYLE) ?
                           fortran_header : usual_header);
            printf( "%s", buff);
            }
         else
            printf( "Couldn't get data: error %d\n", rval);
         }
      }
   else if( argc < 5)
      show_error_message( );
   else
      {
      FILE *ofile = fopen( "urat1.txt", "wb");

      if( !ofile)
         {
         printf( "Couldn't open urat1.txt\n");
         show_error_message( );
         return( -1);
         }
      if( show_header)
         fprintf( ofile, "%s\n", (format & URAT1_FORTRAN_STYLE) ?
                           fortran_header : usual_header);
      rval = extract_urat1_stars( ofile, atof( argv[1]), atof( argv[2]),
                               atof( argv[3]), atof( argv[4]),
                               (argc == 5 ? "" : argv[5]), format);

      if( show_debug_data)
         {
         printf( "%.2f seconds elapsed\n",
               (double)clock( ) / (double)CLOCKS_PER_SEC);
         printf( "%d stars extracted\n", rval);
         }
      fclose( ofile);
      }
   return( rval);
}
