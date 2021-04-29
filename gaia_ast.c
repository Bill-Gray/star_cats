#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "mpc_func.h"
#include "gaia32.h"

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
   const char *input_line;
   int zone;
   uint32_t offset;
   } file_output_t;

#define PI 3.1415926535897932384626433832795028841971693993751058209749445923

static int output_a_gaia32_star( void *context, const int zone,
               const uint32_t offset, GAIA32_STAR *star)
{
   file_output_t *c = (file_output_t *)context;
   const double radians_to_arcsec = 3600. * 180. / PI;

   c->zone = zone;
   c->offset = offset;
   c->delta_ra  = (double)star->ra  / 1000. - c->ra  * radians_to_arcsec;
   c->delta_dec = (double)star->dec / 1000. - c->dec * radians_to_arcsec;
   printf( "%s\nDelta RA = %.3f\" Delta Dec = %.3f\"\n",
                              c->input_line, c->delta_ra, c->delta_dec);
   printf( "Gaia %d %ld;  mag %2d.%03d\n", zone, (long)offset + 1,
                  (int)star->mag / 1000, (int)star->mag % 1000);
   return( 0);
}

static void error_exit( void)
{
   fprintf( stderr,
      "'gaia_ast' will read an input file of 80-column or ADES astrometry\n"
      "and look for matching stars from Dave Tholen's compressed version\n"
      "of the Gaia2 catalogue.  Command line arguments are the name of the\n"
      "input file and any of the following switches :\n\n"
      "-d            Show input astrometry\n"
      "-p (path)     Specify path to Gaia2 files (default is current dir)\n"
      "-r (radius)   Specify search distance in arcsec (default is 1)\n");
   exit( -1);
}

int main( const int argc, const char **argv)
{
   FILE *ifile;
   char buff[400];
   void *ades_context = init_ades2mpc( );
   const char *path_to_data = "";
   int i, show_data = 0, n_found = 0;
   double search_radius = 1.;

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
            case 'p':
               path_to_data = arg;
               break;
            case 'r':
               search_radius = atof( arg);
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

      if( show_data)
         printf( "%s\n", buff);
      if( !get_ra_dec_from_mpc_report( buff, NULL, &ra, NULL,
                                              NULL, &dec, NULL))
         {
         file_output_t c;

         c.input_line = buff;
         c.ra = ra;
         c.dec = dec;
         n_found = extract_gaia32_stars_callback( &c, output_a_gaia32_star,
                  ra * 180. / PI, dec * 180. / PI,
                  search_radius * 2., search_radius * 2., path_to_data);
         memset( buff, 0, 80);
         }
      }
   if( n_found < 0)
      printf( "Error %d\n", n_found);
   free_ades2mpc_context( ades_context);
   fclose( ifile);
   return( 0);
}
