/* Code to compute an all-sky brightness map.  It does so by reading
in data on Gaia-DR2 stars,  from Dave Tholen's compressed version,
and computing the total brightness in 0.1-degree squares,  for a
3600x1800 map.  (That size is adjustable by changing XSIZE and YSIZE,
but the defaults seem to be pretty good.)

Each (four-byte integer) pixel is the total brightness of the stars
within that 0.1x0.1-degree region,  in units where a single count
would represent a mag 20 star.  So a pixel count of 100 would mean
that the total brightness in that area is equivalent to a mag 15
star;  a pixel count of 1000 would equate to a mag 12.5 star,  and
so on.

The '-c' command line option allows you to switch to having stars
simply counted,  rather than computing a cumulative magnitude.  I am
still testing this out,  but it seems that for purposes of judging
the extent to which galactic confusion will make imaging difficult,
it's the count of stars that matters more than their total brightness.

You can also limit the magnitude depth by re-defining MAG_LIMIT.
I do see some artifacts in the map for MAG_LIMIT=22.000;  I think
that reflects the extent to which Gaia has gotten more data in
some parts of the sky than others.  Setting a lower MAG_LIMIT
might remove some of that observational bias.

See 'make_map.c' for code that makes a .pbm (portable bitmap) image
from this data.  The intended use for this file,  though,  is for
computing sky background brightnesses in 'vislimit.cpp'.  That code
currently computes brightness from the sun,  moon,  airglow,  and
various other sources,  but not from the sky background.  It's
also now in use in Find_Orb for computing a 'galactic confusion'
estimate.  */

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "gaia32.h"

#define XSIZE 3600
#define YSIZE 1800

static FILE *get_zone_file( const int zone)
{
   char filename[80];
   FILE *rval;

   assert( zone >= 0 && zone < 180);
   sprintf( filename, "/home/phred/gaia2/%03d.cat", zone);
   rval = fopen( filename, "rb");
   if( !rval)
      {
      printf( "Couldn't open '%s'\n", filename);
      perror( "error");
      exit( -1);
      }
   return( rval);
}


static void show_histo( const int32_t *map)
{
   int histo[32], i, j;

   for( i = 0; i < 32; i++)
      histo[i] = 0;
   for( i = XSIZE * YSIZE; i; i--, map++)
      {
      int32_t tval = *map;

      for( j = 0; tval; j++)
         tval >>= 1;
      assert( j < 32);
      histo[j]++;
      }
   for( i = 0; i < 32; i++)
      printf( "%d ", histo[i]);
   printf( "\n");
}

#define BUFF_SIZE 10000
#define MAG_LIMIT 22000

int main( const int argc, const char **argv)
{
   int32_t *remap = (int32_t *)calloc( MAG_LIMIT, sizeof( int32_t));
   int32_t *map = (int32_t *)calloc( XSIZE * YSIZE, sizeof( int32_t));
   int i, zone0 = 0, zone1 = 179, zone, counting = 0;
   const char *map_name = "bright.zq";
   FILE *fp;
   GAIA32_STAR *stars = (GAIA32_STAR *)calloc( BUFF_SIZE, sizeof( GAIA32_STAR));

   for( i = 0; i < argc; i++)
      if( argv[i][0] == '-')
         switch( argv[i][1])
            {
            case 'z':
               sscanf( argv[i] + 2, "%d,%d", &zone0, &zone1);
               break;
            case 'c':
               counting = 1;
               break;
            default:
               printf( "Unrecognized option '%s'\n", argv[i]);
               return( -1);
            }

   for( i = 0; i < MAG_LIMIT; i++)
      remap[i] = pow( 100., (double)( 20000 - i) / 5000.);
   fp = fopen( map_name, "rb");
   if( fp)
      {
      const size_t n_read = fread( map, sizeof( int32_t), XSIZE * YSIZE, fp);

      fclose( fp);
      assert( n_read == XSIZE * YSIZE);
      }
   for( zone = zone0; zone <= zone1; zone++)
      {
      size_t n_read, n_total = 0;

      fp = get_zone_file( zone);
      while( (n_read = fread( stars, sizeof( GAIA32_STAR), BUFF_SIZE, fp)) > 0)
         {
         for( i = 0; i < (int)n_read; i++)
            {
            const int x = (int)( stars[i].ra  / 360000);
            const int y = (int)( (stars[i].dec + 90 * 3600000)/ 360000);


//          if( stars[i].mag <= 0 || stars[i].mag >= MAG_LIMIT)
//             printf( "mag %d (i = %d)\n", stars[i].mag, i);
//          assert( stars[i].mag > 0 && stars[i].mag < MAG_LIMIT);
            assert( stars[i].mag > 0);
            if( stars[i].mag < MAG_LIMIT)
               {
               if( counting)
                  map[x + y * XSIZE]++;
               else
                  map[x + y * XSIZE] += remap[stars[i].mag];
               }
            }
         n_total += n_read;
         }
      fclose( fp);
      printf( "Zone %d: %ld read\n", zone, (long)n_total);
      show_histo( map);
      fp = fopen( map_name, "wb");
      assert( fp);
      fwrite( map, XSIZE * YSIZE, sizeof( int32_t), fp);
      fclose( fp);
      }
   return( 0);
}
