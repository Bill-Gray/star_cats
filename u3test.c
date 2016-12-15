#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ucac3.h"

      /* The following code shows how 'ucac3.cpp' can be used.  Run it as

         u3test (RA) (dec) (width) (height)

         (all arguments in decimal degrees) and it will extract all
         UCAC3 stars within the specified rectangle.  Optionally,  you
         can add a "path" argument.  The data is written to the file
         'ucac3.txt',  and the total number of stars extracted is shown
         on the console. */

static void show_error_message( void)
{
   printf( "'u3test.exe' takes as command-line arguments the RA/dec of the\n");
   printf( "center of the region to be extracted;  its width and height;\n");
   printf( "and,  optionally,  the path to the UCAC-3 data.  All angles\n");
   printf( "are in decimal degrees.  For example:\n\n");
   printf( "u3test 50 -16.3 2 1.5 d:\\u3\n\n");
   printf( "would extract a 2-degree wide,  1.5-degree high area centered\n");
   printf( "on RA=50 degrees=3h20m, dec=-16.3,  with the data drawn from\n");
   printf( "the path d:\\u3.  Data will be written to the file 'ucac3.txt'.\n");
}

int main( const int argc, const char **argv)
{
   int rval = -9;

   if( argc == 2 || argc == 3)
      {
      UCAC3_STAR star;
      int zone;
      long offset;
      const char *data_path = (argc == 2 ? "" : argv[2]);

      if( sscanf( argv[1], "%d-%ld", &zone, &offset) != 2)
         printf( "Couldn't parse '%s' as a UCAC3 ID\n", argv[1]);
      else
         {
         rval = extract_ucac3_info( zone, offset, &star, data_path);

         if( !rval)
            {
            char buff[UCAC3_ASCII_SIZE];

            write_ucac3_star( zone, offset, buff, &star, 0);
            printf( "%s", buff);
            }
         else
            printf( "Couldn't get data: err %d\n", rval);
         }
      }
   else if( argc < 5)
      show_error_message( );
   else
      {
      FILE *ofile = fopen( "ucac3.txt", "w");
      int i;
      int pass = 0;         /* for the nonce,  there isn't a BSS-3 yet: */
      unsigned format = UCAC3_WRITE_SPACES;

      for( i = 5; i < argc; i++)
         if( argv[i][0] == '-')
            switch( argv[i][1])
               {
               case 'h': case 'H':
                  if( format & UCAC3_FORTRAN_STYLE)
                     fprintf( ofile, "       ran      spdn   im1  im2 smag \
ot ds sra sdc na1nu1us1 cn1 cepRA cepde pmRA pmDec spmRA spmDe 2MassID  \
magJ  magH  magK  ICQflg  e2mpho    SCBmag Rmag  Imag clbl qB qR qI \
catflags g1c1 leda x2m  rn   \n");
                  else
                     {
                     fprintf( ofile, "  UCAC3 id   RA(deg)       dec(deg)  ");
                     fprintf( ofile, "mag1   mag2  e_mag otdb  epRA  epDec   ");
                     fprintf( ofile, "eRA eDe nU uU uT #cats pmRA   pmDe eRp ");
                     fprintf( ofile, "eDp  2MASS ID  magJ   magH   magKs   ");
                     fprintf( ofile, "e_2mass_ph 2massflg mag_B  mag_R  mag_I  ");
                     fprintf( ofile, "cl Bq Rq Iq cats_used g1j1 LED ext ");
                     fprintf( ofile, "running# \n");
                     }
                  break;
               case 'f': case 'F':
                  sscanf( argv[i] + 2, "%x", &format);
                  break;
               default:
                  printf( "%s is an unrecognized option\n", argv[i]);
                  break;
               }
//    for( pass = 0; pass < 2; pass++)
         {
         rval = extract_ucac3_stars( ofile, atof( argv[1]), atof( argv[2]),
                                            atof( argv[3]), atof( argv[4]),
                                         (argc > 5 ? argv[5] : ""), pass,
                                         format);

         printf( "%d stars extracted\n", rval);
         }
      fclose( ofile);
      }
   return( rval);
}
