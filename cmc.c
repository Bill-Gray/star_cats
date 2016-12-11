#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cmc1x.h"

/*
Quite a few CMC-1x fields are of the form 'nn.nnn';  magnitudes,  for
example,  and the seconds in RA and arcseconds in dec.  This function
will extract said fields as integers,  as if the decimal point wasn't there. */

static int32_t extract_number_from_rec( const char *iptr)
{
   long rval = atol( iptr);

   while( *iptr == ' ')
      iptr++;
   while( *iptr >= '0' && *iptr <= '9')
      iptr++;
   if( *iptr == '.')
      {
      iptr++;
      while( *iptr >= '0' && *iptr <= '9')
         {
         rval = rval * 10 + *iptr - '0';
         iptr++;
         }
      }
   return( (int32_t)rval);
}

/* The 'n_astro', 'n_total',  and 'n_photo' fields are two-digit integers.
Just using atoi( ) causes trouble,  since the fields run together.  Hence
this convenience function,  used _only_ for parsing those three fields. */

static inline int16_t get_two_digit_number( const char *tptr)
{
   int16_t rval = (int16_t)( tptr[1] - '0');

   if( tptr[0] != ' ')
      rval = (int16_t)(( tptr[0] - '0') * 10 + rval);
   return( rval);
}

int cmc1x_ascii_to_struct( CMC1x_REC *rec, const char *buff)
{
   rec->ra = (int32_t)( extract_number_from_rec( buff + 22)  /* .0001 secs */
                    + 600000L * atol( buff + 19)       /* minutes */
                  + 36000000L * atol( buff + 16));     /* hours */
   rec->dec = (int32_t)( extract_number_from_rec( buff + 37) /* 0".001 */
                     + 60000L * atol( buff + 34)      /* arcminutes */
                   + 3600000L * atol( buff + 31));    /* degrees */
   if( buff[30] == '-')
      rec->dec = -rec->dec;
   rec->mag_r = (int16_t)extract_number_from_rec( buff + 44);
   rec->n_total = get_two_digit_number( buff + 51);
   rec->n_astro = get_two_digit_number( buff + 53);
   rec->n_photo = get_two_digit_number( buff + 55);
   rec->sigma_ra = (int16_t)extract_number_from_rec( buff + 57);
   rec->sigma_dec = (int16_t)extract_number_from_rec( buff + 63);
   rec->sigma_mag = (int16_t)extract_number_from_rec( buff + 69);
   rec->epoch = (int16_t)atol( buff + 76);
   rec->mag_j  = (int16_t)extract_number_from_rec( buff + 81);
   rec->mag_h  = (int16_t)extract_number_from_rec( buff + 88);
   rec->mag_ks = (int16_t)extract_number_from_rec( buff + 95);
   rec->photometric_flag = (rec->n_photo == 0);
   return( 0);
}

/* Several CMC-1x fields (sigmas and mags) are stored to three places.
This function will output values given in thousandths,  with decimal
place intact and in a fixed six-character field,  replicating the
original CMC-1x record. */

static void put_three_digits( char *buff, const int ival)
{
   sprintf( buff, "%2d.%03d", ival / 1000, ival % 1000);
}

int cmc1x_struct_to_ascii( char *buff, const CMC1x_REC *rec)
{
   const long abs_dec = abs( rec->dec);
   int i, j;

   for( i = 0; i < CMC1x_ASCII_RECORD_SIZE; i++)
      buff[i] = ' ';
   sprintf( buff + 16, "%02ld %02ld %02ld.%04ld ",
            rec->ra / 36000000L, (rec->ra / 600000L) % 60L,
            (rec->ra / 10000L) % 60L, rec->ra % 10000L);
   sprintf( buff + 29, "  %02ld %02ld %02ld.%03ld ",
            abs_dec / 3600000L, (abs_dec / 60000L) % 60L,
            (abs_dec / 1000L) % 60L, abs_dec % 1000L);
   buff[30] = (rec->dec >= 0L ? '+' : '-');
               /* assemble identifier from RA/dec string: */
   for( i = 0, j = 16; j < 39; j++)
      if( buff[j] != ' ' && (j < 26 || j > 28))
         buff[i++] = buff[j];
   if( buff[30] == '+')
      buff[30] = ' ';
   put_three_digits( buff + 44, rec->mag_r);
   sprintf( buff + 50, "%3d%2d%2d", rec->n_total, rec->n_astro, rec->n_photo);
   put_three_digits( buff + 57, rec->sigma_ra);
   put_three_digits( buff + 63, rec->sigma_dec);
   put_three_digits( buff + 69, rec->sigma_mag);
   sprintf( buff + 75, "%5d", rec->epoch);
   put_three_digits( buff + 81, rec->mag_j);
   put_three_digits( buff + 88, rec->mag_h);
   put_three_digits( buff + 95, rec->mag_ks);

   for( i = 0; i < CMC1x_ASCII_RECORD_SIZE; i++)
      if( !buff[i])
         buff[i] = ' ';
   if( rec->photometric_flag)
      buff[50] = ':';
   buff[CMC1x_ASCII_RECORD_SIZE - 1] = 10;    /* record terminates in a LF */
   return( 0);
}

/* Functions to read/store long and short integers on non-Intel platforms.
No idea if they work,  since I lack such a platform to try it on.  Following
this are versions of the same four functions for Intel-order platforms. */

#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
   #define USING_BIG_ENDIAN
#endif
#endif

#ifdef USING_BIG_ENDIAN
static void put_int32( char *buff, const int32_t ival)
{
   int i;

   for( i = 3; i >= 0; i--, ival >>= 8)
      buff[i] = (char)ival;
}

static void put_int16( char *buff, const int16_t ival)
{
   *buff++ = (char)(ival >> 8);
   *buff   = (char)ival;
}

static int32_t get_int32( const char *buff)
{
   int32_t rval;

   ((char *)&rval)[3] = *buff++;
   ((char *)&rval)[2] = *buff++;
   ((char *)&rval)[1] = *buff++;
   ((char *)&rval)[0] = *buff;
   return( rval);
}

static int16_t get_int16( const char *buff)
{
   int16_t rval = 0;

   ((char *)&rval)[1] = *buff++;
   ((char *)&rval)[0] = *buff++;
   return( rval);
}
#else
static void put_int32( char *buff, int32_t ival)
{
   *(int32_t *)buff = ival;
}

static void put_int16( char *buff, const int16_t ival)
{
   *(int16_t *)buff = (int16_t)ival;
}

static int32_t get_int32( const char *buff)
{
   return( *(int32_t *)buff);
}

static int16_t get_int16( const char *buff)
{
   return( *(int16_t *)buff);
}
#endif

/* A CMC-1x structure can be stored in 25 bytes without significant trouble.
I did two somewhat strange things to compact the data:

   First,  I stored n_astro and n_photo in a single byte,  four bits each.
That caused a slight problem: there were sixteen astrometric and
photometric observations for some stars. I therefore stored bit 0x10 of
n_astro and n_photo as the high bits of sigma_ra and sigma_dec,
respectively.

   Next,  since the 'epoch' is always less than 2500 days [IN CMC-14,
NO LONGER TRUE IN CMC-15!],  it's [IT WAS] possible to store epoch and
n_total in the same short integer,  as long as n_total < 65535 / 2500 =
26.  As it happens,  the maximum n_total is 16, so this works.

   Further to this:  2500 days after the CMC "epoch" of 1999 Mar 26 gets
one as far as 2006 Jan 28.  CMC-14 ceased observing before then.  CMC-15
includes plenty of further dates,  and we should be somewhat prepared for
CMC-16 and CMC-17.  Fortunately,  the top two bits of of the 'ra' field
are unused (10000*60*60*24 < 2^30),  allowing us to quadruple the range
of epochs to 10000 days,  or to 2026 August 11.  That should be enough
for anybody.

   Also,  CMC-15 has records with n_total >= 26.  In such cases,  we add
180 degrees (180 * 60 * 60 * 1000 milliarcseconds) to the dec,  and
store n_total - 26.

   Further compression is quite possible,  especially if fixed-length
records are abandoned,  but I didn't have a tremendous need to make the
data much smaller.              */

int cmc1x_struct_to_binary_rec( char *buff, const CMC1x_REC *rec)
{
   put_int32( buff, rec->ra + ((rec->epoch / 2500) << 30));
   put_int32( buff + 4, rec->dec + (rec->n_total >= 26 ? 180 * 3600 * 1000 : 0));
   put_int16( buff + 8, rec->mag_r);
   put_int16( buff + 10, (int16_t)
           (rec->sigma_ra  | ((rec->n_astro & 0x10) ? 0x4000 : 0)));;
   put_int16( buff + 12, (int16_t)
           (rec->sigma_dec | ((rec->n_photo & 0x10) ? 0x4000 : 0)));
   put_int16( buff + 14, rec->sigma_mag);
   put_int16( buff + 16, (int16_t)( rec->epoch % 2500 + (rec->n_total % 26) * 2500));
   put_int16( buff + 18, rec->mag_j);
   put_int16( buff + 20, rec->mag_h);
   put_int16( buff + 22, rec->mag_ks);
   buff[24] = (char)( (rec->n_astro & 0xf) * 16 + (rec->n_photo & 0xf));
   return( 0);
}

int cmc1x_binary_rec_to_struct( CMC1x_REC *rec, const char *buff)
{
   const uint16_t packed_epoch_n_total =
                   (uint16_t)get_int16( buff + 16);

   rec->ra = get_int32( buff);
   rec->dec = get_int32( buff + 4);
   rec->mag_r = get_int16( buff + 8);
   rec->sigma_ra = get_int16( buff + 10);
   rec->sigma_dec = get_int16( buff + 12);
   rec->sigma_mag = get_int16( buff + 14);
   rec->epoch = (int16_t)( packed_epoch_n_total % 2500u);
   rec->epoch += (int16_t)( ((uint32_t)rec->ra >> 30) * 2500);
   rec->ra &= 0x3fffffff;
   rec->mag_j = get_int16( buff + 18);
   rec->mag_h = get_int16( buff + 20);
   rec->mag_ks = get_int16( buff + 22);
   rec->n_astro = (int16_t)( (unsigned char)buff[24] >> 4);
   rec->n_photo = ( (unsigned char)buff[24] & 0xf);
   if( rec->sigma_ra & 0x4000)
      {
      rec->sigma_ra ^= 0x4000;
      rec->n_astro |= 0x10;
      }
   if( rec->sigma_dec & 0x4000)
      {
      rec->sigma_dec ^= 0x4000;
      rec->n_photo |= 0x10;
      }
   rec->n_total = (packed_epoch_n_total / 2500u);
   if( rec->dec > 90 * 3600 * 1000)
      {
      rec->dec -= 180 * 3600 * 1000;
      rec->n_total += 26;
      }
   rec->photometric_flag = (rec->n_photo == 0);
   return( 0);
}
