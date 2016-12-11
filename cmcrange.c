#include <stdio.h>
#include <stdint.h>
#include "cmc1x.h"

/* This little program is probably of no actual use to anyone.  It looks
through all CMC-15 files (used to look through all the CMC-14 ones) to
find the maximum and minimum values for most parameters in the data.  I
did this to see how many bits would be required for each,  thereby
allowing me to figure out how to pack all the bits into a binary
structure for file compression.   */

static void show_results( CMC1x_REC *min, CMC1x_REC *max)
{
   printf( "n_photo: %d to %d\n", min->n_photo, max->n_photo);
   printf( "n_astro: %d to %d\n", min->n_astro, max->n_astro);
   printf( "n_total: %d to %d\n", min->n_total, max->n_total);
   printf( "sigma_ra: %d to %d\n", min->sigma_ra, max->sigma_ra);
   printf( "sigma_dec: %d to %d\n", min->sigma_dec, max->sigma_dec);
   printf( "sigma_mag: %d to %d\n", min->sigma_mag, max->sigma_mag);
   printf( "epoch: %d to %d\n", min->epoch, max->epoch);
   printf( "mag_h: %d to %d\n", min->mag_h, max->mag_h);
   printf( "mag_j: %d to %d\n", min->mag_j, max->mag_j);
   printf( "mag_ks: %d to %d\n", min->mag_ks, max->mag_ks);
   printf( "mag_r: %d to %d\n", min->mag_r, max->mag_r);
   printf( "\n");
}

static const char *filenames[] = {
        "cmc15n9.cmc", "cmc15n9r.cmc",
        "cmc15n0.cmc", "cmc15n0r.cmc",
        "cmc15n1.cmc", "cmc15n1r.cmc",
        "cmc15n2.cmc", "cmc15n2r.cmc",
        "cmc15n3.cmc", "cmc15n3r.cmc",
        "cmc15n4.cmc", "cmc15n4r.cmc",
        "cmc15n5.cmc", "cmc15n5r.cmc",
        "cmc15n6.cmc", "cmc15n6r.cmc",
        "cmc15n7.cmc", "cmc15n7r.cmc",
        "cmc15n8.cmc", "cmc15n8r.cmc",
        "cmc15na.cmc", "cmc15nar.cmc",
        "cmc15s0.cmc", "cmc15s0r.cmc",
        "cmc15s1.cmc", "cmc15s1r.cmc",
        "cmc15s2.cmc", "cmc15s2r.cmc",
        "cmc15s3.cmc", "cmc15s3r.cmc",
        "cmc15s4.cmc", "cmc15s4r.cmc",
        "cmc15s5.cmc", "cmc15s5r.cmc",
        "cmc15s6.cmc", "cmc15s6r.cmc",
        "cmc15s7.cmc", "cmc15s7r.cmc", NULL };

int main( const int unused_argc, const char **unused_argv)
{
   CMC1x_REC max, min, irec;
   char tbuff[CMC1x_BINARY_RECORD_SIZE];
   int i;

   for( i = 0; filenames[i]; i++)
      {
      FILE *ifile = fopen( filenames[i], "rb");
      int rec_no = 0;

      printf( "Looking through %s (%d of 38)\n", filenames[i], i);
      if( !ifile)
         printf( "WARNING: %s not opened\n", filenames[i]);
      else
         {
         while( fread( tbuff, CMC1x_BINARY_RECORD_SIZE, 1, ifile))
            {
            cmc1x_binary_rec_to_struct( &irec, tbuff);
            if( irec.n_photo > irec.n_astro)
               irec.n_total -= irec.n_photo;
            else
               irec.n_total -= irec.n_astro;
            if( irec.n_total < 0 || irec.n_total > 30)
               {
               char obuff[CMC1x_ASCII_RECORD_SIZE + 1];

               cmc1x_struct_to_ascii( obuff, &irec);
               obuff[CMC1x_ASCII_RECORD_SIZE] = '\0';
               printf( "File %s\n", filenames[i]);
               printf( "Record %d:\n %s\n", rec_no + 1, obuff);
               printf( "%d %d %d\n", irec.n_total, irec.n_astro, irec.n_photo);
               return( -1);
               }
            if( !rec_no && !i)
               max = min = irec;
            else
               {
               if( min.n_photo > irec.n_photo)
                  min.n_photo = irec.n_photo;
               if( max.n_photo < irec.n_photo)
                  max.n_photo = irec.n_photo;
               if( min.n_astro > irec.n_astro)
                  min.n_astro = irec.n_astro;
               if( max.n_astro < irec.n_astro)
                  max.n_astro = irec.n_astro;
               if( min.n_total > irec.n_total)
                  min.n_total = irec.n_total;
               if( max.n_total < irec.n_total)
                  max.n_total = irec.n_total;
               if( min.sigma_ra > irec.sigma_ra)
                  min.sigma_ra = irec.sigma_ra;
               if( max.sigma_ra < irec.sigma_ra)
                  max.sigma_ra = irec.sigma_ra;
               if( min.sigma_dec > irec.sigma_dec)
                  min.sigma_dec = irec.sigma_dec;
               if( max.sigma_dec < irec.sigma_dec)
                  max.sigma_dec = irec.sigma_dec;
               if( min.sigma_mag > irec.sigma_mag)
                  min.sigma_mag = irec.sigma_mag;
               if( max.sigma_mag < irec.sigma_mag)
                  max.sigma_mag = irec.sigma_mag;
               if( min.epoch > irec.epoch)
                  min.epoch = irec.epoch;
               if( max.epoch < irec.epoch)
                  max.epoch = irec.epoch;
               if( min.mag_r > irec.mag_r)
                  min.mag_r = irec.mag_r;
               if( max.mag_r < irec.mag_r)
                  max.mag_r = irec.mag_r;
               if( (min.mag_h > irec.mag_h && irec.mag_h) || !min.mag_h)
                  min.mag_h = irec.mag_h;
               if( max.mag_h < irec.mag_h)
                  max.mag_h = irec.mag_h;
               if( (min.mag_ks > irec.mag_ks && irec.mag_ks) || !min.mag_ks)
                  min.mag_ks = irec.mag_ks;
               if( max.mag_ks < irec.mag_ks)
                  max.mag_ks = irec.mag_ks;
               if( (min.mag_j > irec.mag_j && irec.mag_j) || !min.mag_j)
                  min.mag_j = irec.mag_j;
               if( max.mag_j < irec.mag_j)
                  max.mag_j = irec.mag_j;
               }
            rec_no++;
            if( rec_no % 1000000 == 10)
               show_results( &min, &max);
            }
         fclose( ifile);
         }
      show_results( &min, &max);
      }
   return( 0);
}
