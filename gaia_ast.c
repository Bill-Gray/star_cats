#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include "mpc_func.h"
#include "gaia32.h"

/* Code to read a file of ADES and/or 80-column astrometry,  or a mix
thereof,  and look for matching stars in a compressed Gaia-DR2 catalogue
provided by Dave Tholen.  (Dave has plans for a Gaia-DR3 version,  but
the format will almost certainly change.  But I don't think I'll have
much work to update this.)

   The basic procedure is as follows :

   The file is opened,  and we read from it and look for astrometry.
When we find it,  we store it in the 'ilines' array in main( ).

   That array can only handle ILINE_MAX entries.  When we fill the
array,  we sort it by the order in which it would appear in Dave's
compressed catalogue,  which has one-degree bands in declination
with stars within each band sorted by RA.  Thus,  we'll be looking
through that catalogue "in order".

   This helps quite a bit with speed.  The catalogue is a bit over
47 GBytes.

   Once sorted,  we look up each line of astrometry using the
functions in gaia32.c to see if any Gaia-DR2 star lands near that
star.

   After all stars are checked,  we sort the data back out into its
original order and dump it to the output file.

   When the input file has been completely read,  we apply the same
sort/check/resort/output process to the remaining records.  Note
that most input files have far fewer than ILINE_MAX entries;  we
mostly encounter that when running the entire ITF and similarly
large compilations.        */

static const char *get_arg( const char **argv)
{
   const char *rval;

   if( argv[0][0] == '-' && argv[0][1])
      {
      if( !argv[0][2] && argv[1])
         rval = argv[1];
      else
         rval = argv[0] + 2;
      }
   else
      rval = NULL;
   if( !rval)
      {
      fprintf( stderr, "Can't get an argument : '%s'\n", argv[0]);
      exit( 0);
      }
   return( rval);
}

typedef struct
   {
   double ra, dec, delta_ra, delta_dec;
   char buff[81];
   int zone, mag, idx;
   uint32_t offset;
   } iline_t;

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923

static int set_a_gaia32_star( void *context, const int zone,
               const uint32_t offset, GAIA32_STAR *star)
{
   iline_t *c = (iline_t *)context;
   const double radians_to_arcsec = 3600. * 180. / PI;

   c->zone = zone;
   c->offset = offset;
   c->delta_ra  = (double)star->ra  / 1000. - c->ra  * radians_to_arcsec;
   c->delta_dec = (double)star->dec / 1000. - c->dec * radians_to_arcsec;
   c->delta_ra *= cos( c->dec);
   c->mag = star->mag;
   return( 0);
}

#define SORT_BY_DEC        0
#define SORT_BY_IDX        1

static int sort_order;

static int compare( const void *aptr, const void *bptr)
{
   const iline_t *a = (const iline_t *)aptr;
   const iline_t *b = (const iline_t *)bptr;
   int rval;

   if( sort_order == SORT_BY_DEC)
      {
      const int zone_a = (int)( a->dec * 180. / PI + 90.);
      const int zone_b = (int)( b->dec * 180. / PI + 90.);

      rval = zone_b - zone_a;
      if( !rval)     /* sort by RA within zone */
         rval = (a->ra > b->ra ? 1 : -1);
      }
   else           /* SORT_BY_IDX */
      rval = (a->idx > b->idx ? 1 : -1);
   return( rval);
}

static const char *path_to_data = "";
static double search_radius = 1.;
static int verbose = 0;

static void dump_ilines( iline_t *ilines, const int n_ilines, FILE *output_file)
{
   int i;

   sort_order = SORT_BY_DEC;
   qsort( ilines, n_ilines, sizeof( iline_t), compare);
            /* should sort by dec order here,  then... */
   setbuf( stdout, NULL);
   for( i = 0; i < n_ilines; i++)
      {
      const int n_found =
             extract_gaia32_stars_callback( ilines + i, set_a_gaia32_star,
                  ilines[i].ra * 180. / PI, ilines[i].dec * 180. / PI,
                  search_radius * 2. / cos( ilines[i].dec),
                  search_radius * 2., path_to_data);

      if( n_found < 0)
         {
         fprintf( stderr, "Error %d reading Gaia-DR2\n", n_found);
         exit( -1);
         }
      if( verbose && !(i % (n_ilines / 80 + 1)))
         printf( ".");
      }
   if( verbose)
      printf( "\n");
            /* ...sort back out by idx (order in original file) and output : */
   sort_order = SORT_BY_IDX;
   qsort( ilines, n_ilines, sizeof( iline_t), compare);
   for( i = 0; i < n_ilines; i++)
      if( ilines[i].zone > -1)
         {
         fprintf( output_file, "%s\nDelta RA = %.3f\" Delta Dec = %.3f\"\n",
                     ilines[i].buff, ilines[i].delta_ra, ilines[i].delta_dec);
         fprintf( output_file, "Gaia %d %ld;  mag %2d.%03d\n",
                  ilines[i].zone, (long)ilines[i].offset + 1,
                  (int)ilines[i].mag / 1000, (int)ilines[i].mag % 1000);
         }
}

static void error_exit( void)
{
   fprintf( stderr,
      "'gaia_ast' will read an input file of 80-column or ADES astrometry\n"
      "and look for matching stars from Dave Tholen's compressed version\n"
      "of the Gaia2 catalogue.  Command line arguments are the name of the\n"
      "input file and any of the following switches :\n\n"
      "-d            Show input astrometry\n"
      "-o (filename) Output goes to file name\n"
      "-p (path)     Specify path to Gaia2 files (default is current dir)\n"
      "-r (radius)   Specify search distance in arcsec (default is 1)\n"
      "-v            Verbose (currently just provides a progress bar)\n");
   exit( -1);
}

#define ILINE_MAX  (1 << 23)
#define is_power_of_two( X)   (!((X) & ((X) - 1)))

int main( const int argc, const char **argv)
{
   FILE *ifile, *output_file = stdout;
   char buff[400];
   void *ades_context = init_ades2mpc( );
   int i, show_data = 0, n_found = 0, line_no = 0;
   iline_t *ilines = NULL;
   int n_ilines = 0;

   if( argc < 2)
      error_exit( );
   ifile = fopen( argv[1], "rb");
   if( !ifile)
      {
      fprintf( stderr, "Couldn't open '%s' : %s\n\n", argv[1], strerror( errno));
      error_exit( );
      }
   assert( ades_context);
   for( i = 2; i < argc; i++)
      if( argv[i][0] == '-')
         {
         const char *arg = get_arg( argv + i);

         switch( argv[i][1])
            {
            case 'd':
               show_data = 1;
               break;
            case 'o':
               output_file = fopen( arg, "wb");
               if( !output_file)
                  {
                  fprintf( stderr, "Couldn't open '%s' : %s\n\n",
                              arg, strerror( errno));
                  error_exit( );
                  }
               break;
            case 'p':
               path_to_data = arg;
               break;
            case 'r':
               search_radius = atof( arg);
               break;
            case 'v':
               verbose = 1;
               if( argv[i][2])
                  verbose = atoi( argv[i] + 2);
               break;
            default:
               fprintf( stderr, "'%s' not recognized\n", argv[i]);
               break;
            }
         }

   search_radius /= 3600.;    /* cvt arcsec to degrees */

   while( n_found >= 0 && fgets_with_ades_xlation( buff, sizeof( buff), ades_context, ifile))
      {
      double ra, dec;

      line_no++;
      if( show_data)
         printf( "%s\n", buff);
      if( !get_ra_dec_from_mpc_report( buff, NULL, &ra, NULL,
                                              NULL, &dec, NULL))
         {
         if( n_ilines == ILINE_MAX)
            {
            dump_ilines( ilines, n_ilines, output_file);
            n_ilines = 0;
            }
         if( strlen( buff) != 80)
            printf( "Line %d : '%s'\n", line_no, buff);
         assert( strlen( buff) == 80);
         if( is_power_of_two( n_ilines + 1))
            {
            ilines = (iline_t *)realloc( ilines, 2 * (n_ilines + 1) * sizeof( iline_t));
            assert( ilines);
            }
         strcpy( ilines[n_ilines].buff, buff);
         ilines[n_ilines].idx = n_ilines;
         ilines[n_ilines].ra  = ra;
         ilines[n_ilines].dec = dec;
         ilines[n_ilines].zone = -1;
         n_ilines++;
         }
      memset( buff, 0, 80);
      }
   if( ilines)
      {
      dump_ilines( ilines, n_ilines, output_file);
      free( ilines);
      }
   free_ades2mpc_context( ades_context);
   fclose( ifile);
   return( 0);
}
