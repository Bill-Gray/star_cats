#include <stdint.h>

      /* Forcing byte-aligned packing is probably not _essential_,  but... */
#pragma pack( 1)

#define UCAC3_STAR struct ucac3_star

UCAC3_STAR
   {
   int32_t ra, spd;         /* RA/dec at J2000.0,  ICRS,  in milliarcsec  */
   int16_t mag1, mag2;      /* UCAC fit model & aperture magnitudes, .001 mag */
   int16_t mag_sigma;
   int8_t obj_type, double_star_flag;
   int16_t ra_sigma, dec_sigma;    /* sigmas in RA and dec at central epoch */
   int8_t n_ucac_total;       /* Number of UCAC observations of this star */
   int8_t n_ucac_used;        /* # UCAC observations _used_ for this star */
   int8_t n_cats_used;        /* # catalogs (epochs) used for prop motion */
   int8_t n_cats_total;       /* # catalogs (epochs) used,  total         */
   int16_t epoch_ra;          /* Central epoch for mean RA, minus 1900, .01y */
   int16_t epoch_dec;         /* Central epoch for mean DE, minus 1900, .01y */
   int32_t pm_ra;             /* prop motion, .1 mas/yr = .01 arcsec/cy */
   int32_t pm_dec;            /* prop motion, .1 mas/yr = .01 arcsec/cy */
   int16_t pm_ra_sigma;       /* sigma in same units */
   int16_t pm_dec_sigma;
   int32_t twomass_id;        /* 2MASS pts_key star identifier */
   int16_t mag_j, mag_h, mag_k;  /* 2MASS J, H, K_s mags,  in millimags */
   int8_t icq_flag[3];
   int8_t e2mpho[3];          /* 2MASS error photometry (in centimags) */
   int16_t sc_bmag;           /* SuperCosmos B magnitude */
   int16_t sc_rmag;           /* SuperCosmos R2 magnitude */
   int16_t sc_imag;           /* SuperCosmos I magnitude */
   int8_t sc_class;           /* SuperCosmos star/galaxy classif/quality flag */
   int8_t sc_bmag_quality;    /* SuperCosmos quality flag for B mag */
   int8_t sc_rmag_quality;    /* SuperCosmos quality flag for R2 mag */
   int8_t sc_imag_quality;    /* SuperCosmos quality flag for I mag */
   int8_t catflag[10];        /* flags for matches in 10 major catalogues */
   int8_t yale_g1;            /* flag for Yale SPM type */
   int8_t yale_c1;            /* Yale input cat type */
   int8_t leda_flag;          /* LEDA galaxy match flag */
   int8_t twomass_ext_flag;   /* 2MASS extended source flag */
   int32_t running_number;    /* MPOS star number; IDs high prop motion stars */
   };
#pragma pack( )

/* Note: sizeof( UCAC3_STAR) = 84 bytes */

         /* Function to do byte-reversal for non-Intel-order platforms. */
         /* NOT ACTUALLY TESTED ON SUCH MACHINES YET.                   */
#ifdef WRONG_ENDIAN
void flip_ucac3_star( UCAC3_STAR *star);
#endif

         /* Extracts data for a give RA/dec rectangle,  writes out result */
         /* as ASCII text to 'ofile'.                                     */
int extract_ucac3_stars( FILE *ofile, const double ra, const double dec,
                  const double width, const double height, const char *path,
                  const int is_supplement, const int output_format);

int extract_ucac3_info( const int zone, const long offset, UCAC3_STAR *star,
                     const char *path);
int write_ucac3_star( const int zone, const long offset, char *obuff,
                     const UCAC3_STAR *star, const int output_format);

#define UCAC3_ASCII_SIZE 250

         /* This flag suppresses stars that were matched with Tycho stars. */
         /* In some of my software,  stars are drawn from both UCAC-3 and  */
         /* Tycho.  By setting this flag in the output_format field,  I keep */
         /* those stars from being plotted twice.                           */
#define UCAC3_OMIT_TYCHO_STARS            0x1

         /* By default,  zero magnitudes and proper motions are written out */
         /* as zeroes.  Setting this 'output_format' flag causes them to be  */
         /* written out as spaces.                                          */
#define UCAC3_WRITE_SPACES                0x2
#define UCAC3_FORTRAN_STYLE               0x4

         /* 98.5% of UCAC-3 stars were matched to 2MASS.  The remaining  */
         /* 1.5% are mostly spurious doubles,  and by default,  any star */
         /* lacking a 2MASS ID is omitted.  But you can include them if  */
         /* you wish,  using the following flag.                         */
#define UCAC3_INCLUDE_DOUBTFULS           0x8

#define UCAC3_CATFLAG_HIP           0
#define UCAC3_CATFLAG_TYCHO         1
#define UCAC3_CATFLAG_AC2000        2
#define UCAC3_CATFLAG_AGK2B         3
#define UCAC3_CATFLAG_AGK2H         4
#define UCAC3_CATFLAG_ZA            5
#define UCAC3_CATFLAG_BY            6
#define UCAC3_CATFLAG_LICK          7
#define UCAC3_CATFLAG_SC            8
#define UCAC3_CATFLAG_SPM           9
