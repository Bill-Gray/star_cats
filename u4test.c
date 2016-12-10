#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "ucac4.h"

      /* The following code shows how 'ucac4.cpp' can be used.  Run it as

         u4test (RA) (dec) (width) (height)

         (all arguments in decimal degrees) and it will extract all
         UCAC4 stars within the specified rectangle.  Optionally,  you
         can add a "path" argument.  The data is written to the file
         'ucac4.txt',  and the total number of stars extracted is shown
         on the console. */

static void show_error_message( void)
{
   printf( "'u4test.exe' takes as command-line arguments the RA/dec of the\n");
   printf( "center of the region to be extracted;  its width and height;\n");
   printf( "and,  optionally,  the path to the UCAC-4 data.  All angles\n");
   printf( "are in decimal degrees.  For example:\n\n");
   printf( "u4test 50 -16.3 2 1.5 d:\\u4\n\n");
   printf( "would extract a 2-degree wide,  1.5-degree high area centered\n");
   printf( "on RA=50 degrees=3h20m, dec=-16.3,  with the data drawn from\n");
   printf( "the path d:\\u4.  Data will be written to the file 'ucac4.txt'.\n");
   printf( "\nOptionally, one may add command line options -h to include a header\n");
   printf( "line,  and/or -f4 to get the same output as from the FORTRAN code.\n");
}

static const char *fortran_header = "       ran      spdn  mag1  mag2 smot\
 ds sra sdc nt nu nc  RAepochDec   PMra  PMdec PMsigma    2MassID  magJ\
  magH  magK  ICQflgs  e2mpho   Vmag  Bmag  gmag  rmag  imag Vsi\
 Bsi gsi rsi isiYG -----catflags---- Le 2X   ID numb 2Uz  2Unum";

static const char *usual_header = "  UCAC4        RA             dec     \
 mag1  mag2 smag ot dsf  RA epoch dec  dRA dde nt nu nc   pmRA  pmDec pmSigmas\
  2MASS ID  mag_j  mag_h  mag_k  e2mphos    icq_flag  Bmag   Vmag   gmag   rmag\
   imag  sig_B sig_V sig_g sig_r sig_i catflags  Ya Led 2MX  ID numb  UCAC2 ID";

int main( int argc, const char **argv)
{
   int rval = -9, i, j, show_debug_data = 0;
   unsigned format = UCAC4_WRITE_SPACES;
   int show_header = 0;

   for( i = 1; i < argc; i++)
      if( argv[i][0] == '-' && argv[i][1] > '9')
         {
         switch( argv[i][1])
            {
            case 'h': case 'H':
               show_header = 1;
               if( format & UCAC4_FORTRAN_STYLE)
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
      UCAC4_STAR star;
      int zone;
      long offset;

      if( sscanf( argv[1], "%d-%ld", &zone, &offset) != 2)
         printf( "Couldn't parse '%s' as a UCAC4 ID\n", argv[1]);
      else
         {
         int rval = extract_ucac4_info( zone, offset, &star,
                                (argc == 2 ? "" : argv[2]));

         if( !rval)
            {
            char buff[UCAC4_ASCII_SIZE];

            write_ucac4_star( zone, offset, buff, &star, format);
            if( show_header)
               printf( "%s\n", (format & UCAC4_FORTRAN_STYLE) ?
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
      FILE *ofile = fopen( "ucac4.txt", "wb");

      if( !ofile)
         {
         printf( "Couldn't open ucac4.txt\n");
         show_error_message( );
         return( -1);
         }
      if( show_header)
         fprintf( ofile, "%s\n", (format & UCAC4_FORTRAN_STYLE) ?
                           fortran_header : usual_header);
      rval = extract_ucac4_stars( ofile, atof( argv[1]), atof( argv[2]),
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
