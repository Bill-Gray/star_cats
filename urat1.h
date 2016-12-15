#ifndef URAT1_H_INCLUDED
#define URAT1_H_INCLUDED

/* Basic access functions for URAT1.  Public domain.  Please contact
pluto (at) projectpluto.com with comments/bug fixes.  */

#include <stdint.h>

      /* Raw structures are read herein,  so the following structure  */
      /* must be packed on byte boundaries:                           */
#pragma pack( 1)

#define URAT1_STAR struct urat1_star

URAT1_STAR
   {      /*    byte_off units   */
   int32_t ra;    /*   0 mas      mean RA on ICRF at URAT mean obs.epoch  */
   int32_t spd;   /*   4 mas      mean South Pole Distance = Dec + 90 deg */
   int16_t sigs;  /*   8 mas      position error per coord. from scatter  */
   int16_t sigm;  /*  10 mas      position error per coord. from model    */
   int8_t  nst;   /*  12 --       tot. number of sets the star is in      */
   int8_t  nsu;   /*  13 --       number of sets used for mean position   */
   int16_t epoc;  /*  14 myr      mean URAT obs. epoch - 2000.0           */
   uint16_t mmag; /*  16 mmag     mean URAT model fit magnitude           */
   int16_t sigp;  /*  18 mmag     URAT photometry error                   */
   int8_t  nsm;   /*  20 --       number of sets used for URAT magnitude  */
   int8_t  ref;   /*  21 --       largest reference star flag             */
   int16_t nit;   /*  22 --       total number of images (observations)   */
   int16_t niu;   /*  24 --       number of images used for mean position */
   int8_t  ngt;   /*  26 --       total number of 1st order grating obs.  */
   int8_t  ngu;   /*  27 --       number of 1st order grating pairs used  */
   int16_t pmr;   /*  28 .1mas/yr proper motion RA*cosDec (from 2MASS)    */
   int16_t pmd;   /*  30 .1mas/yr proper motion Dec                       */
   int16_t pme;   /*  32 .1mas/yr proper motion error per coordinate      */
   int8_t  mfm;   /*  34 --       match flag URAT with 2MASS              */
   int8_t  mfa;   /*  35 --       match flag URAT with APASS              */
   int32_t id2;   /*  36 --       unique 2MASS star identification number */
   uint16_t twomass_mag[3];
                  /*  40 mmag     2MASS J mag                             */
                  /*  42 mmag     2MASS H mag                             */
                  /*  44 mmag     2MASS K mag                             */
   int16_t twomass_mag_sigma[3];
                  /*  46 mmag     error 2MASS J mag                       */
                  /*  48 mmag     error 2MASS H mag                       */
                  /*  50 mmag     error 2MASS K mag                       */
   int8_t icc_flag[3];
                  /*  52 --       ICC flag 2MASS J                        */
                  /*  53 --       ICC flag 2MASS H                        */
                  /*  54 --       ICC flag 2MASS K                        */
   int8_t photo_flag[3];
                  /*  55 --       photometry quality flag 2MASS J         */
                  /*  56 --       photometry quality flag 2MASS H         */
                  /*  57 --       photometry quality flag 2MASS K         */
   uint16_t apass_mag[5];
                  /*  58 mmag     APASS B mag                             */
                  /*  60 mmag     APASS V mag                             */
                  /*  62 mmag     APASS g mag                             */
                  /*  64 mmag     APASS r mag                             */
                  /*  66 mmag     APASS i mag                             */
   int16_t apass_mag_sigma[5];
                  /*  68 mmag     error APASS B mag                       */
                  /*  70 mmag     error APASS V mag                       */
                  /*  72 mmag     error APASS g mag                       */
                  /*  74 mmag     error APASS r mag                       */
                  /*  76 mmag     error APASS i mag                       */
   int8_t  ann;   /*  78 --       APASS numb. of nights                   */
   int8_t  ano;   /*  79 --       APASS numb. of observ.                  */
   };
#pragma pack( )

/* Note: sizeof( URAT1_STAR) = 80 bytes */

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

         /* Function to do byte-reversal for non-Intel-order platforms. */
         /* NOT ACTUALLY TESTED ON SUCH MACHINES YET.  At least,  not   */
         /* to my knowledge... please let me know if you _do_ test it,  */
         /* fixes needed,  etc.!                                        */

#ifdef __BIG_ENDIAN
void flip_urat1_star( URAT1_STAR *star);
#endif

         /* Extracts data for a give RA/dec rectangle,  writes out result */
         /* as ASCII text to 'ofile'.  RA, dec, width, height in degrees. */
int extract_urat1_stars( FILE *ofile, const double ra, const double dec,
                  const double width, const double height, const char *path,
                  const int output_format);

int extract_urat1_info( const int zone, const long offset, URAT1_STAR *star,
                     const char *path);
int write_urat1_star( const int zone, const long offset, char *obuff,
                     const URAT1_STAR *star, const int output_format);

#define URAT1_ASCII_SIZE 280

         /* By default,  zero magnitudes and proper motions are written    */
         /* out as zeroes.  Setting this 'output_format' flag causes them  */
         /* to be written out as spaces.                                   */
#define URAT1_WRITE_SPACES                0x2

         /* By default,  data is written out in a somewhat human-readable */
         /* form.  Use of this flag causes the data to be written exactly */
         /* as the FORTRAN code would do it, as a series of integers.     */
#define URAT1_FORTRAN_STYLE               0x4

         /* Use this to get the Fortran-style output,  but with IDs of the */
         /* form zzz-nnnnnn at the end of each line :                      */
#define URAT1_FORTRAN_STYLE_WITH_IDS      0xc

         /* "Raw binary" means the data is written out in the same 78 byte */
         /* per star format in which it was read: no reformatting is done. */
#define URAT1_RAW_BINARY                  0x10

#ifdef __cplusplus
}
#endif  /* #ifdef __cplusplus */

#endif  /* #ifndef URAT1_H_INCLUDED */
