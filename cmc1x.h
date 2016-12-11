#ifndef CMC_1x_H_INCLUDED
#define CMC_1x_H_INCLUDED

#define CMC1x_REC struct cmc1x_rec
#define CMC1x_ASCII_RECORD_SIZE 102
#define CMC1x_BINARY_RECORD_SIZE 25

/* CMC-14 and CMC-15 are nearly identical in terms of data format.
CMC-15 goes deeper and extends the coverage southward from dec=-30
to dec=-40,  and adds a lot of stars and newer data to the original
area.  But in terms of data access,  they're handled in an almost
identical manner,  which is why they're called 'CMC-1x' herein. */

CMC1x_REC
   {
   int32_t ra, dec;        /* in .0001 seconds & milliarcseconds */
   int16_t mag_r;         /* All magnitudes are in .001 mags */
   int16_t epoch;         /* in days from 26 Mar 1999 */
   int16_t n_total;       /* total number of observations,  incl. bad ones */
   int16_t n_astro;       /* number accepted astrometric obs */
   int16_t n_photo;       /* number accepted photometric obs */
   int16_t sigma_ra;      /* std deviation of RA,  in .0001 seconds */
   int16_t sigma_dec;     /* std deviation of dec,  in .001 arcseconds */
   int16_t sigma_mag;     /* std deviation of mag_r,  in .001 mags */
   int16_t mag_j, mag_h, mag_ks;   /* 2MASS mags,  in .001 mags */
   int8_t photometric_flag;        /* TRUE iff n_photo = 0 */
   };

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

int cmc1x_ascii_to_struct( CMC1x_REC *rec, const char *buff);
int cmc1x_struct_to_ascii( char *buff, const CMC1x_REC *rec);
int cmc1x_binary_rec_to_struct( CMC1x_REC *rec, const char *buff);
int cmc1x_struct_to_binary_rec( char *buff, const CMC1x_REC *rec);
int extract_cmc1x_stars( FILE *ofile, const double ra_in_degrees,
                  const double dec_in_degrees,
                  const double width_in_degrees,
                  const double height_in_degrees,
                  const char *path, const int rejected);

#ifdef __cplusplus
}
#endif  /* #ifdef __cplusplus */

#endif  /* #ifndef CMC_1x_H_INCLUDED */
