#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

/* This reads in the sky brightness map created from Gaia-DR2 data
using 'bright.c' (q.v.),  and writes out a .pbm (portable bitmap)
image from it. Run,  e.g.,

./make_map 20 200

to get a visually interesting result.  This is used to make the
'bright2.pgm' image used by Find_Orb to estimate galactic confusion. The
same result could also be used (and,  I expect,  will eventually get
used) in computing the background sky brightness in 'vislimit.cpp'.  */

#define XSIZE 3600
#define YSIZE 1800
#define PI 3.1415926535897932384626433832795028841971693993751058209749445923

/* Thought it might help to 'blur' the image with a low-pass filter.
I'm no longer so sure of this.  */

static void low_pass_horizontal( int32_t *iline, const int len)
{
   const int32_t new_start = (iline[0] * 2 + iline[1] + iline[len - 1]) >> 2;
   const int32_t new_end = (iline[len - 1] * 2 + iline[0] + iline[len - 2]) >> 2;
   int i;
   int32_t saved_value = *iline;

   for( i = 1; i < len; i++)
      {
      const int32_t new_value =
               (iline[i] * 2 + iline[i - 1] + saved_value) >> 2;

      saved_value = iline[i];
      iline[i] = new_value;
      }
   iline[0] = new_start;
   iline[len - 1] = new_end;
}

static void low_pass_vertical( int32_t *iline, const int xsize, const int ysize)
{
   int32_t *last = iline + xsize * (ysize - 1);
   const int32_t new_start = (*iline * 2 + iline[xsize] + *last) >> 2;
   const int32_t new_end = (*last * 2 + last[-xsize] * *iline) >> 2;
   int i;
   int32_t saved_value = *iline;

   for( i = 1; i < ysize; i++)
      {
      const int32_t new_value =
               (iline[i * xsize] * 2 + iline[(i - 1) * xsize] + saved_value) >> 2;

      saved_value = iline[i * xsize];
      iline[i * xsize] = new_value;
      }
   iline[0] = new_start;
   *last = new_end;
}

int main( const int argc, const char **argv)
{
   FILE *ifile = fopen( "bright.zq", "rb");
   int32_t *ivals = (int32_t *)calloc( XSIZE * YSIZE, sizeof( int32_t));
   int i, y;
   const int scale = atoi( argv[1]);
   const int offset = atoi( argv[2]);
   size_t n_read;
   FILE *ofile = fopen( "bright.pgm", "wb");
   int n_high = 0, n_low = 0;

   assert( ifile);
   assert( ofile);
   assert( ivals);
   n_read = fread( ivals, sizeof( int32_t), XSIZE * YSIZE, ifile);
   assert( n_read == XSIZE * YSIZE);
   fclose( ifile);

   if( argc > 3)
      for( i = 0; i < atoi( argv[3]); i++)
         {
         for( y = 0; y < YSIZE; y++)
            low_pass_horizontal( ivals + y * XSIZE, XSIZE);
         for( i = 0; i < XSIZE; i++)
            low_pass_vertical( ivals + i, XSIZE, YSIZE);
         }

   fprintf( ofile, "P5\n%d %d\n255\n", XSIZE, YSIZE);
   for( y = YSIZE - 1; y >= 0; y--)
      {
      int32_t *tptr = ivals + y * XSIZE;
      const double dec = (double)( (y - YSIZE / 2) + .5) * PI / (double)YSIZE;
      const double cos_dec = cos( dec);
      uint8_t obuff[XSIZE], *optr = obuff;

      for( i = XSIZE - 1; i >= 0; i--)
         {
//       int oval = (int)( 2.5 * log10( (double)( tptr[i] + 1) / cos_dec) * scale) - offset;
         int oval = (int)( 255. * (double)(tptr[i] - offset) / (scale * cos_dec));

         if( oval > 255)
            {
            n_high++;
            oval = 255;
            }
         else if( oval < 0)
            {
            n_low++;
            oval = 0;
            }
         *optr++ = (uint8_t)oval;
         }
      fwrite( obuff, sizeof( uint8_t), XSIZE, ofile);
      }
   free( ivals);
   fclose( ofile);
   printf( "%d%% high, %d%% low\n", n_high * 100 / (XSIZE * YSIZE),
                                    n_low  * 100 / (XSIZE * YSIZE));
   return( 0);
}
