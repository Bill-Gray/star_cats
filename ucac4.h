#ifndef UCAC4_H_INCLUDED
#define UCAC4_H_INCLUDED

/* Basic access functions for UCAC-4.  Public domain.  Please contact
pluto (at) projectpluto.com with comments/bug fixes.  */

#include <stdint.h>

      /* Raw structures are read herein,  so the following structure  */
      /* must be packed on byte boundaries:                           */
#pragma pack( 1)

/* Changes from UCAC-3:  ra_sigma, dec_sigma, mag_sigma, pm_ra_sigma &
pm_dec_sigma were int16_ts in UCAC3. pm_ra & pm_dec were int32_ts.
n_cats_total has been removed for UCAC4,  as has SuperCOSMOS data.
APASS mags were added. The two Yale flags were combined,  to save a
byte.  The ra/dec and proper motion sigmas are now offset by 128;
i.e.,  a value of -128 would indicate a zero sigma.
*/

#define UCAC4_STAR struct ucac4_star

UCAC4_STAR
   {
   int32_t ra, spd;         /* RA/dec at J2000.0,  ICRS,  in milliarcsec */
   uint16_t mag1, mag2;     /* UCAC fit model & aperture mags, .001 mag */
   uint8_t mag_sigma;
   uint8_t obj_type, double_star_flag;
   int8_t ra_sigma, dec_sigma;    /* sigmas in RA and dec at central epoch */
   uint8_t n_ucac_total;      /* Number of UCAC observations of this star */
   uint8_t n_ucac_used;      /* # UCAC observations _used_ for this star */
   uint8_t n_cats_used;      /* # catalogs (epochs) used for prop motion */
   uint16_t epoch_ra;        /* Central epoch for mean RA, minus 1900, .01y */
   uint16_t epoch_dec;       /* Central epoch for mean DE, minus 1900, .01y */
   int16_t pm_ra;            /* prop motion, .1 mas/yr = .01 arcsec/cy */
   int16_t pm_dec;           /* prop motion, .1 mas/yr = .01 arcsec/cy */
   int8_t pm_ra_sigma;       /* sigma in same units */
   int8_t pm_dec_sigma;
   uint32_t twomass_id;        /* 2MASS pts_key star identifier */
   uint16_t mag_j, mag_h, mag_k;  /* 2MASS J, H, K_s mags,  in millimags */
   uint8_t icq_flag[3];
   uint8_t e2mpho[3];          /* 2MASS error photometry (in centimags) */
   uint16_t apass_mag[5];      /* in millimags */
   int8_t apass_mag_sigma[5];  /* in centimags */
   uint8_t yale_gc_flags;      /* Yale SPM g-flag * 10 + c-flag */
   uint32_t catalog_flags;
   uint8_t leda_flag;          /* LEDA galaxy match flag */
   uint8_t twomass_ext_flag;   /* 2MASS extended source flag */
   uint32_t id_number;
   uint16_t ucac2_zone;
   uint32_t ucac2_number;
   };
#pragma pack( )

/* Note: sizeof( UCAC4_STAR) = 78 bytes */

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

         /* Function to do byte-reversal for non-Intel-order platforms. */
         /* NOT ACTUALLY TESTED ON SUCH MACHINES YET.  At least,  not   */
         /* to my knowledge... please let me know if you _do_ test it,  */
         /* fixes needed,  etc.!                                        */

#ifdef __BIG_ENDIAN
void flip_ucac4_star( UCAC4_STAR *star);
#endif

         /* Extracts data for a give RA/dec rectangle,  writes out result */
         /* as ASCII text to 'ofile'.  RA, dec, width, height in degrees. */
int extract_ucac4_stars( FILE *ofile, const double ra, const double dec,
                  const double width, const double height, const char *path,
                  const int output_format);

int extract_ucac4_info( const int zone, const long offset, UCAC4_STAR *star,
                     const char *path);
int write_ucac4_star_fortran_style( char *obuff, const UCAC4_STAR *star);
int write_ucac4_star( const int zone, const long offset, char *obuff,
                     const UCAC4_STAR *star, const int output_format);

      /* Fitting proper motions into 16-bit ints and their sigmas into */
      /* 8-bit values required some tricks for handling large values.  */
      /* The following functions are required to convert the stored    */
      /* values into "real" values.                                    */
int32_t get_actual_proper_motion( const UCAC4_STAR *star, const int get_dec_pm);
int get_actual_proper_motion_sigma( const int8_t pm_sigma);

#define UCAC4_ASCII_SIZE 280

         /* This flag suppresses stars that were matched with Tycho       */
         /* stars.  In some of my software,  stars are drawn from both    */
         /* UCAC-4 and Tycho.  By setting this flag in the output_format  */
         /* field,  I keep those stars from being plotted twice.          */
#define UCAC4_OMIT_TYCHO_STARS            0x1

         /* By default,  zero magnitudes and proper motions are written    */
         /* out as zeroes.  Setting this 'output_format' flag causes them  */
         /* to be written out as spaces.                                   */
#define UCAC4_WRITE_SPACES                0x2

         /* By default,  data is written out in a somewhat human-readable */
         /* form.  Use of this flag causes the data to be written exactly */
         /* as the FORTRAN code would do it, as a series of integers.     */
#define UCAC4_FORTRAN_STYLE               0x4

         /* 98.5% of UCAC4 stars were matched to 2MASS.  The remaining  */
         /* 1.5% are somewhat doubtful,  and by default,  any star      */
         /* lacking a 2MASS ID is omitted.  But you can include them    */
         /* if you wish,  using the following flag.                     */
#define UCAC4_INCLUDE_DOUBTFULS           0x8

         /* "Raw binary" means the data is written out in the same 78 byte */
         /* per star format in which it was read: no reformatting is done. */
#define UCAC4_RAW_BINARY                  0x10

     /* The "catalog_flags" member of the UCAC4_STAR structure is a        */
     /* nine-digit number,  with each digit signifying something about the */
     /* matching of that catalog to the star in question.  These macros    */
     /* may be convenient in getting the flag for a specific catalog:      */

#define ucac4_tycho_catflag( catflag)                 ((catflag) / 100000000)
#define ucac4_ac2000_catflag( catflag)               (((catflag) / 10000000) % 10)
#define ucac4_agk2_bonn_catflag( catflag)            (((catflag) / 1000000) % 10)
#define ucac4_agk2_hamburg_catflag( catflag)         (((catflag) / 100000) % 10)
#define ucac4_agk2_zone_astrog_catflag( catflag)     (((catflag) / 10000) % 10)
#define ucac4_agk2_black_birch_catflag( catflag)     (((catflag) / 1000) % 10)
#define ucac4_agk2_lick_astrog_catflag( catflag)     (((catflag) / 100) % 10)
#define ucac4_agk2_npm_lick_catflag( catflag)        (((catflag) / 10) % 10)
#define ucac4_agk2_yale_spm_catflag( catflag)         ((catflag) % 10)

#ifdef __cplusplus
}
#endif  /* #ifdef __cplusplus */

#endif  /* #ifndef UCAC4_H_INCLUDED */
