#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "cmc1x.h"

/* Certain lines in CMC-15 come in with 60.0000 in the RA seconds field,
or 60.000 in the dec seconds field.  In both cases,  the identifier is
fine.  Examples :

073300.0+230158 07 32 60.0000  23 01 58.361  ....
124350.5+223600 12 43 50.5727  22 35 60.000  ....
*/

static unsigned fix_60_problem( char *buff)
{
   unsigned rval = 0;

   if( !memcmp( buff + 22, "60.0000", 7))    /* fix the RA */
      {
      buff[16] = buff[0];
      buff[17] = buff[1];
      buff[19] = buff[2];
      buff[20] = buff[3];
      buff[22] = ' ';     /* use a leading space,  not a leading zero */
      buff[23] = buff[5];
      rval++;
      }
   if( !memcmp( buff + 37, "60.000", 6))    /* fix the dec */
      {
      buff[31] = buff[9];
      buff[32] = buff[10];
      buff[34] = buff[11];
      buff[35] = buff[12];
      buff[37] = ' ';     /* use a leading space,  not a leading zero */
      buff[38] = buff[14];
      rval++;
      }
   return( rval);
}

/* Main program to read in an ASCII CMC-1x file and write out its binary
counterpart,  resulting in about a 4:1 compression (to be exact,  each
ASCII 102-byte record becomes a binary 25-byte one.)  */

int main( int argc, char **argv)
{
   FILE *ifile, *ofile;
   char buff[200];
   int line = 0, n_recs, output_freq = 128;
   int output_counter = 0;
   unsigned n_60_problems_found = 0;
   double prev_t = 0.;

   if( argc != 2)
      {
      printf( "'cmc_xvt' takes the name of an ASCII CMC-14 or CMC-15 file as a command\n");
      printf( "line argument,  and creates the corresponding compressed binary\n");
      printf( "CMC-14/15 file.  For example,  the command\n\ncmc14xvt cmc14s5\n\n");
      printf( "would cause the program to read in the file 'cmc14s5.dat',  and\n");
      printf( "build the binary file 'cmc14s5.cmc'.  As the records are compressed,\n");
      printf( "they are decompressed and compared with the original to ensure no\n");
      printf( "compression errors occur.\n");
      exit( -1);
      }
                  /* Open ASCII file first,  exiting if it's not found: */
   sprintf( buff, "%s.dat", argv[1]);
   ifile = fopen( buff, "rb");
   if( !ifile)
      ifile = fopen( argv[1], "rb");
   if( !ifile)
      {
      printf( "%s not opened\n", buff);
      exit( -1);
      }
   fseek( ifile, 0L, SEEK_END);
                  /* Find out how many records are in the input file.  */
                  /* This is used for the progress indicator.          */
   n_recs = ftell( ifile) / CMC1x_ASCII_RECORD_SIZE;
   printf( "%ld stars in file\n", (long)n_recs);
   fseek( ifile, 0L, SEEK_SET);
                  /* Open the output binary file:  */
   sprintf( buff, "%s.cmc", argv[1]);
   ofile = fopen( buff, "wb");
   if( !ofile)
      {
      printf( "%s not opened\n", buff);
      exit( -1);
      }
                  /* Read in all records from the input ASCII file: */
   while( fread( buff, CMC1x_ASCII_RECORD_SIZE, 1, ifile))
      {
      char obuff[200], compressed[CMC1x_BINARY_RECORD_SIZE];
      CMC1x_REC irec;

      line++;
                   /* Compress the input record and write it out: */
      cmc1x_ascii_to_struct( &irec, buff);
      cmc1x_struct_to_binary_rec( compressed, &irec);
      fwrite( compressed, CMC1x_BINARY_RECORD_SIZE, 1, ofile);
                  /* The result of compressing,  then decompressing a */
                  /* record should match the original record:         */
      memset( obuff, 0, sizeof( obuff));
      memset( &irec, 0, sizeof( CMC1x_REC));
      cmc1x_binary_rec_to_struct( &irec, compressed);
      cmc1x_struct_to_ascii( obuff, &irec);
               /* In CMC15,  the seconds and arcseconds places */
               /* lack leading zeroes.  If there's a zero in either */
               /* place,  we need to convert it to a space. */
      if( obuff[22] == '0')
         obuff[22] = ' ';
      if( obuff[37] == '0')
         obuff[37] = ' ';
      if( memcmp( obuff, buff, CMC1x_ASCII_RECORD_SIZE))
         {
         n_60_problems_found += fix_60_problem( buff);
         if( memcmp( obuff, buff, CMC1x_ASCII_RECORD_SIZE))
            {
            printf( "Problem at line %d\n", line);
            printf( "%s\n%s\n", buff, obuff);
            }
         }
//    fwrite( compressed, CMC1x_BINARY_RECORD_SIZE, 1, ofile);
      output_counter++;
      if( output_counter == output_freq)
         {
         const double new_t = (double)clock( ) / (double)CLOCKS_PER_SEC;
         const double fraction = (double)line / (double)n_recs;

         if( new_t != prev_t)
            output_freq = (int)( (double)output_freq / (new_t - prev_t)) + 1;
         printf( "%.1lf%% done; %.0lf seconds elapsed, %.0lf remain   \r",
                  fraction * 100., new_t,
                  new_t * (1. - fraction) / fraction);
         fflush( stdout);
         prev_t = new_t;
         output_counter = 0;
         }
      }
   if( n_60_problems_found)
      printf( "\n%u 60-problems found (and handled)\n", n_60_problems_found);
   return( 0);
}

