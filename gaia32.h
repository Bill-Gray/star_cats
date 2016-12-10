#ifndef GAIA32_H_INCLUDED
#define GAIA32_H_INCLUDED

/* Basic access functions for Dave Tholen's GAIA32 catalog(ue).
Public domain.  Please contact pluto (at) projectpluto.com with
comments/bug fixes.  */

#include <stdint.h>

      /* Raw structures are read herein,  so the following structure  */
      /* must be packed on byte boundaries:                           */
#pragma pack( 1)

#define GAIA32_STAR struct gaia32_star

GAIA32_STAR
   {
   int32_t ra, dec;         /* RA/dec at J2000.0,  ICRS,  in milliarcsec */
   int8_t ra_sigma;         /* half-mas,  offset by 128 */
   int8_t dec_sigma;        /* half-mas,  offset by 128 */
   int32_t pm_ra, pm_dec;   /* in microarcsec/yr = 0.1 mas/century */
   uint16_t pm_ra_sigma;    /* same units */
   uint16_t pm_dec_sigma;
   int16_t epoch;           /* in milliyears since 2000 */
   uint16_t mag, mag_sigma; /* in millimagnitudes */
   };
#pragma pack( )

/* Note: sizeof( GAIA32_STAR) = 78 bytes */

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

         /* Function to do byte-reversal for non-Intel-order platforms. */
         /* NOT ACTUALLY TESTED ON SUCH MACHINES YET.  At least,  not   */
         /* to my knowledge... please let me know if you _do_ test it,  */
         /* fixes needed,  etc.!                                        */

#ifdef __BIG_ENDIAN
void flip_gaia32_star( GAIA32_STAR *star);
#endif

         /* Extracts data for a give RA/dec rectangle,  writes out result */
         /* as ASCII text to 'ofile'.  RA, dec, width, height in degrees. */
int extract_gaia32_stars( FILE *ofile, const double ra, const double dec,
                  const double width, const double height, const char *path,
                  const int output_format);

int extract_gaia32_info( const int zone, const long offset, GAIA32_STAR *star,
                     const char *path);
int write_gaia32_star_fortran_style( char *obuff, const GAIA32_STAR *star);
int write_gaia32_star( const int zone, const long offset, char *obuff,
                     const GAIA32_STAR *star, const int output_format);

#define GAIA32_ASCII_SIZE 100

         /* By default,  zero magnitudes and proper motions are written    */
         /* out as zeroes.  Setting this 'output_format' flag causes them  */
         /* to be written out as spaces.                                   */
#define GAIA32_WRITE_SPACES                0x2

         /* By default,  data is written out in a somewhat human-readable */
         /* form.  Use of this flag causes the data to be written exactly */
         /* as the FORTRAN code would do it, as a series of integers.     */
#define GAIA32_FORTRAN_STYLE               0x4

         /* By default,  RA/decs are written in decimal degrees.    */
#define GAIA32_BASE_60                     0x8

         /* "Raw binary" means the data is written out in the same 28 byte */
         /* per star format in which it was read: no reformatting is done. */
#define GAIA32_RAW_BINARY                  0x10

#ifdef __cplusplus
}
#endif  /* #ifdef __cplusplus */

#endif  /* #ifndef GAIA32_H_INCLUDED */
