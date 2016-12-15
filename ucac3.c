#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ucac3.h"

/* History: */

/*   25 Sep 2009:  (BJG) You can now pass a NULL 'ofile' (output file pointer)
   to the extractor code.  I did that because I was attempting to do a
   matching process in which I didn't care about the actual star extracted;
   I just wanted to see if there were star(s) within a particular RA/dec
   rectangle.   */

/*    9 Sep 2009:  (BJG) Switched to new 3UCzzz-nnnnnn numbering convention.
   I've added flags to select FORTRAN-style output (to match the style used
   by the example Fortran code),  to suppress Tycho-2 stars (so that,  when
   shown in my planetarium software,  you don't get "false doubles" from
   Tycho-2 stars that get repeated in UCAC-3),  and a "write spaces" flag
   that causes blank fields to be written as spaces rather than zeroes.
   Soon to be added:  flag to suppress stars not matched in 2MASS,  most
   of which appear to be spurious.   */

/*    13 Aug 2009: (BJG) Changed from 'ucac2' to 'ucac3'.  Made almost
   totally new structures for UCAC3_STAR and switched from 'short'/'long'/'char'
   to more reliable 'int16_t'/'int32_t'/'int8_t'.  Indexing now uses a
   new 'u3index.bin' file,  made by the 'makebin' utility;  and a binary
   search scheme should improve speed,  at least slightly.   */

/*     3 Jan 2007: (BJG) Added a function to get the UCAC-2 data for a
   specific UCAC-2 numbered star.  Also,  I found that all stars in
   'z000' (zone from the south celestial pole to dec -89.5) were
   given wrong numbers in the output from 'extract_ucac2_stars()'
   (offset by one),  and fixed this.   */

/*    22 Jan 2004: (BJG) Revised to handle the Bright Star Supplement.
   That required looking at a different index and different files with
   different heights,  and displaying epochs to .01 year precision
   instead of .001 year precision.  I've given BSS stars UCAC2 numbers
   starting at 50000000 (fifty million),  to distinguish them from
   "normal" UCAC2 stars.  extract_ucac2_stars() now takes a Boolean
   argument to distinguish between "extract from 'normal' UCAC-2" and
   "extract from the supplement".

      Also,  since the BSS includes some negative-magnitude stars,
   I had to add some 'abs()' statements.

      Also,  the test main() routine now has to call extract_ucac2_stars()
   twice:  once to get 'normal' UCAC-2 stars,  then a second time to get
   supplement stars.  Most programs using this source ought to do the same. */

/*    24 Oct 2003:  (BJG)  While running a little program to extract
   data for high proper-motion stars from UCAC2,  I learned that in
   some cases,  an extra digit is needed for the RA,  dec,  and
   "goodness of fit" parameters for proper motion.  (The latter can,
   on rare conditions,  be as high as 1250,  thus requiring four
   digits instead of the usual three.)  Note that this means each
   field expands by one character,  and the total line length expands
   by four bytes.  */

/*     4 Aug 2003:  (BJG)  Revised to have goodness of fit run from 0
   to 100,  instead of 0 to 20,  reflecting the fact that it's given
   in .05 units. */

/*    17 May 2003:  (BJG)  Renamed extract_ucac_stars() to
   extract_ucac2_stars() to evade confusion with UCAC-1.  Altered same
   function so that the 'path' to the data can end in a path separator;
   i.e.,  either d:\u2\ or d:\u2 would work on most DOS/Windows boxes.
   Put an #ifdef TEST_PROGRAM around the test main()... necessary because
   I'm gonna use this code in Charon and Guide and elsewhere. */

/* 13 Jun 2003:  (BJG) Learned that in the final format,  the "reference
   flag" byte is replaced by a "position error" one,  with the same 127
   offset.  So in the structure def for UCAC2_STAR,  changed "reference_flag"
   to "e_posn",  and modified "write_ucac2_star()" accordingly... this
   does mean the ASCII output has an extra byte in it,  and everything
   afterward is pushed over by one column. */

/* 13 Jun 2003: (BJG) 'u2index.unf' has been replaced by 'u2index.da',
   with a much more straightforward layout.  Changed the code accordingly. */

/* 16 Jun 2003: (BJG) Run the test program without enough command line
   data,  and it gives you an error message/usage directions.           */

/* On non-Intel-ordered machines,  we need the 'swap_long',  'swap_short',
   and 'flip_ucac2_star' functions. */

/* NOTE that this offset table has been incremented by one relative to the
corresponding table for UCAC-2!  That's what was supplied with UCAC-3...
and it happens to make things just slightly easier: */

static long ucac3_offsets[361] = {
        1,      1259,      5087,     11572,     20547,     31955,
    45619,     61631,     80399,    102269,    126197,    152603,
   180958,    212191,    245313,    280385,    318125,    358548,
   401532,    447130,    497280,    551509,    608424,    671399,
   738054,    807448,    879719,    958813,   1044761,   1138972,
  1240684,   1349646,   1469585,   1600944,   1744865,   1894731,
  2056681,   2228014,   2409266,   2609780,   2834413,   3074582,
  3330403,   3598891,   3881859,   4171579,   4478571,   4807632,
  5160913,   5547338,   5949247,   6353908,   6749935,   7155702,
  7554145,   7962845,   8402663,   8876229,   9367994,   9877917,
 10370689,  10859916,  11346861,  11823540,  12308467,  12788699,
 13230137,  13693391,  14161347,  14626573,  15102278,  15603501,
 16108490,  16603994,  17085055,  17529733,  17982385,  18440600,
 18885688,  19318553,  19739085,  20167830,  20582886,  20998317,
 21418536,  21845530,  22273579,  22706186,  23131670,  23552133,
 23981820,  24429198,  24862457,  25275380,  25684928,  26087372,
 26489336,  26908397,  27333219,  27756951,  28177946,  28615132,
 29046887,  29487600,  29928386,  30369245,  30797426,  31233047,
 31670325,  32123396,  32591793,  33086118,  33586340,  34090156,
 34580995,  35074335,  35571894,  36080297,  36606310,  37139587,
 37710383,  38327805,  38952450,  39589770,  40192138,  40782812,
 41356280,  41905639,  42405475,  42919154,  43404070,  43864868,
 44323481,  44809958,  45305440,  45770270,  46227659,  46687638,
 47144129,  47586379,  48024402,  48458872,  48905466,  49365345,
 49836553,  50293004,  50721275,  51129828,  51525289,  51938818,
 52336739,  52730817,  53099953,  53474035,  53831697,  54165528,
 54479053,  54776550,  55067055,  55365803,  55667248,  55960001,
 56260820,  56570648,  56881331,  57191685,  57515671,  57852104,
 58183415,  58493553,  58780683,  59066142,  59333936,  59598269,
 59865298,  60130000,  60405378,  60693443,  60988353,  61297603,
 61602679,  61888233,  62156906,  62428378,  62710270,  63000113,
 63297480,  63599986,  63901940,  64193183,  64479746,  64763626,
 65062139,  65357950,  65658241,  65966074,  66283977,  66613662,
 66932206,  67255863,  67577390,  67901333,  68223394,  68550137,
 68875926,  69188732,  69502177,  69820626,  70139098,  70459644,
 70786602,  71116591,  71449292,  71766637,  72087002,  72422009,
 72762187,  73095444,  73422888,  73724945,  74041880,  74353588,
 74654239,  74968457,  75288284,  75598847,  75898648,  76186723,
 76474112,  76767080,  77067760,  77365062,  77655006,  77952816,
 78249706,  78543707,  78844073,  79155588,  79468086,  79784821,
 80099302,  80410846,  80719364,  81027112,  81323223,  81622133,
 81923802,  82227105,  82531464,  82833525,  83129540,  83422903,
 83719928,  84025412,  84317721,  84600945,  84877351,  85159800,
 85432948,  85701455,  85968522,  86232302,  86501282,  86758526,
 87020151,  87284140,  87558017,  87830244,  88102517,  88376980,
 88663330,  88955356,  89249137,  89552236,  89853860,  90150084,
 90444395,  90736495,  91028887,  91317780,  91603891,  91886186,
 92161221,  92428805,  92700094,  92968200,  93232098,  93489247,
 93745276,  94001317,  94252173,  94496971,  94733475,  94967338,
 95206104,  95443085,  95666113,  95881314,  96078851,  96273498,
 96467687,  96657916,  96847083,  97030848,  97198194,  97368787,
 97539736,  97707904,  97862703,  98007415,  98139596,  98260508,
 98371187,  98473126,  98570669,  98665159,  98759305,  98859331,
 98955661,  99044875,  99134339,  99222679,  99310317,  99393280,
 99473737,  99552272,  99626235,  99696358,  99766347,  99831403,
 99894372,  99953228, 100010756, 100065527, 100116349, 100165973,
100214848, 100263293, 100309068, 100353587, 100395691, 100435931,
100472622, 100505951, 100537833, 100567475, 100594822, 100620246,
100643307, 100664312, 100683511, 100701141, 100715633, 100727974,
100738493, 100746909, 100753745, 100759267, 100763371, 100765503,
100766421 };

#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
static void swap_32( int32_t *ival)
{
   int8_t temp, *zval = (int8_t *)ival;

   temp = zval[0];
   zval[0] = zval[3];
   zval[3] = temp;
   temp = zval[1];
   zval[1] = zval[2];
   zval[2] = temp;
}

static void swap_16( int16_t *ival)
{
   int8_t temp, *zval = (int8_t *)ival;

   temp = zval[0];
   zval[0] = zval[1];
   zval[1] = temp;
}

void flip_ucac3_star( UCAC3_STAR *star)
{
   swap_32( &star->ra);
   swap_32( &star->spd);
   swap_16( &star->mag1);
   swap_16( &star->mag2);
   swap_16( &star->mag_sigma);
   swap_16( &star->ra_sigma);
   swap_16( &star->dec_sigma);
   swap_16( &star->epoch_ra);
   swap_16( &star->epoch_dec);
   swap_32( &star->pm_ra);
   swap_32( &star->pm_dec);
   swap_16( &star->pm_ra_sigma);
   swap_16( &star->pm_dec_sigma);
   swap_32( &star->twomass_id);
   swap_16( &star->mag_j);
   swap_16( &star->mag_h);
   swap_16( &star->mag_k);
   swap_16( &star->sc_bmag);
   swap_16( &star->sc_rmag);
   swap_16( &star->sc_imag);
   swap_32( &star->running_number);
}
#endif                   // #if __BYTE_ORDER == __BIG_ENDIAN
#endif                   // #ifdef __BYTE_ORDER

/* The following function writes out a UCAC3 star in a standardized
   ASCII format that looks like this (column header plus sample lines):
  UCAC3 id   RA(deg)       dec(deg)  mag1   mag2  e_mag otdb  epRA  epDec   eRA eDe nU uU uT #cats pmRA   pmDe eRp eDp  2MASS ID  magJ   magH   magKs   e_2mass_ph 2massflg mag_B  mag_R  mag_I  cl Bq Rq Iq cats_used g1j1 LED ext running#
 000029117 264.98744028 -87.92151139 15.217 15.323 136  0 0 1988.24 1988.56  26  35  5  5  3   3    -20    -58  22  30  970461604 13.638 13.301 13.208 003 003 004 05 05 05 15.794 14.566 13.087 12 00 00 00 0000000011 0 3 000 000     39709
 000020598   1.45297833 -87.89830667 12.804 12.788  75  0 5 1996.77 1996.75  19  12  9  9  4   4     51    -29  22  13 1180995133 11.633 11.391 11.364 002 002 003 05 05 05 12.746 11.919 11.747 02 01 00 01 0010000066 0 3 000 000     40564
 000020599   1.46899333 -87.57657583 16.755 16.481 298  0 0 1977.99 1978.11  45  34  2  1  3   3    197      2  51  38 1180994468 14.380 14.078 13.790 004 005 007 05 05 05 16.605 15.433 14.894 02 00 00 00 0000000011 0 4 000 000     52974
 000031955   0.03352889 -87.26049250 16.613 15.960  -1  0 0 1976.49 1976.40  51  51  1  1  2   2     14    150  66  67 1128887328 14.398 13.902 13.926 004 006 005 05 05 05  0.000  0.000  0.000 00 00 00 00 0000000005 0 4 000 000     67055
 100742309 148.50973444  87.47187639 15.856 15.811  98  0 0 2003.95 2003.95  32  37  9  6  2   2      0      0 -10 -10  981885060 11.741 11.084 10.858 003 003 002 05 05 05 17.003 15.031 12.750 02 00 00 00 0000000010 0 0 000 000 139036953
 100742312 148.53571444  87.32957583 14.226 14.266  47  0 0 2003.97 2003.97  18  12 15 14  2   2      0      0 -10 -10  981884799 12.676 12.320 12.204 002 003 003 05 05 05 14.462 13.456 13.192 02 00 00 00 0000000010 0 0 000 000 139028955
 100742313 148.53750556  87.48247972 14.141 14.120  46  0 0 2003.99 2003.99  14  12 21 18  2   2      0      0 -10 -10  981885087 12.288 11.750 11.648 003 003 002 05 05 05 14.605 13.102 12.544 12 00 00 00 0000000010 0 0 000 000 139037639
*/

/* Guess at what the offset will be for BSS in UCAC-3: */
#define BSS_OFFSET 900000000

static int write_ucac3_star_fortran_style( char *obuff, const UCAC3_STAR *star)
{
/*
              WRITE (12,'(i10,1x,i9,1x,2(i5,1x),i3,1x,i2,1x,
     .             i1,1x,2(i3,1x),2(i2,1x),2(i3,1x),2(i5,1x),
     .             2(i6,1x),2(i3,1x),i10,1x,3(i5,1x),3i2.2,1x,3(i3,1x),
     .             3(i5,1x),4(i2,1x),10i1,1x, 2i1,1x,2(i3,1x),i9)')
     .          ran,spdn,im1,im2,sigmag,objt,dsf,sigra,sigdc,na1,nu1
     .         ,us1,cn1,cepra,cepdc,pmrac,pmdc,sigpmr,sigpmd,id,jmag
     .         ,hmag,kmag,icqflg,e2mpho,smB,smR2,smI,clbl,qfB,qfR2
     .         ,qfI,catflg, g1, c1, leda, x2m, rn
     29026  73839932 16171 17113  -1  0 0  70  70  1  1   3   3  8281  8314
    -15   -123  76  74  707049880 14924 14171 14038 050505   5   5   6
    18252 16613 15871  2  0  1  0 0000000011 04   0   0   4230674
    104136  73844316 15357 15322 129  0 0  23  25  7  6   3   3  9396  9310     26     -1  24  25  707049882 13971 13671 13617 050505   4   3   4 15797 14923 14439  2  1  1  1 0000000011 03   0   0   4231533
    105446  74730614 16601 16253 246  0 0  46  72  2  2   3   3  8045  8261     78    -63  45  63  707050323 14829 14435 14489 050506   3   5  11 16898 15909 15403  2  0  1  0 0000000011 03   0   0   4378716
    126163  74917720 14646 14631 124  0 0  18  15  6  6   4   4  9540  9366    157    -82  23  22  707050408 13368 13014 12914 050505   3   2   3 15154 14217 13827 12  1  1  1 0000001011 03   0   0   4409774
*/
   int i;

   sprintf( obuff, "%10d%10d%6d%6d%4d%3d%2d",
                      star->ra, star->spd, star->mag1, star->mag2,
                      star->mag_sigma, star->obj_type, star->double_star_flag);
   sprintf( obuff + strlen( obuff), "%4d%4d%3d%3d%4d%4d%6d%6d",
            star->ra_sigma, star->dec_sigma,
            (int)star->n_ucac_total, (int)star->n_ucac_used,
            (int)star->n_cats_used, (int)star->n_cats_total,
            star->epoch_ra, star->epoch_dec);
   sprintf( obuff + strlen( obuff), "%7d%7d%4d%4d%11d",
            star->pm_ra, star->pm_dec,
            star->pm_ra_sigma, star->pm_dec_sigma, star->twomass_id);
   sprintf( obuff + strlen( obuff), "%6d%6d%6d %02d%02d%02d",
            star->mag_j, star->mag_h, star->mag_k,
            star->icq_flag[0], star->icq_flag[1], star->icq_flag[2]);
   sprintf( obuff + strlen( obuff), "%4d%4d%4d%6d%6d%6d%3d%3d%3d%3d ",
            star->e2mpho[0], star->e2mpho[1], star->e2mpho[2],
            star->sc_bmag, star->sc_rmag, star->sc_imag,
            star->sc_class, star->sc_bmag_quality,
            star->sc_rmag_quality, star->sc_imag_quality);
   for( i = 0; i < 10; i++)
      {
      char tbuff[2];

      tbuff[0] = (char)( '0' + star->catflag[i]);
      tbuff[1] = '\0';
      strcat( obuff, tbuff);
      }

   sprintf( obuff + strlen( obuff), " %d%d%4d%4d%9d\n",
          star->yale_g1, star->yale_c1, star->leda_flag,
          star->twomass_ext_flag, star->running_number);
   return( 0);
}

#ifdef NOW_OBSOLETE
static int ucac3_sequential_to_zone_offset( const long ucac3_sequential,
            long *offset)
{
   int zone;

   for( zone = 361; zone && ucac3_offsets[zone - 1] > ucac3_sequential; zone--)
      ;
   if( zone == 361)      /* number is greater than number of UCAC-3 stars */
      zone = 0;
   if( offset && zone)
      *offset = ucac3_sequential - ucac3_offsets[zone - 1];
   return( zone);
}
#endif

int write_ucac3_star( const int zone, const long offset, char *obuff,
                     const UCAC3_STAR *star, const int output_format)
{
   const long epoch_ra  = 190000 + star->epoch_ra;
   const long epoch_dec = 190000 + star->epoch_dec;
   char tstr[11];
   int i;

   if( output_format & UCAC3_FORTRAN_STYLE)
      return( write_ucac3_star_fortran_style( obuff, star));

   sprintf( obuff, "%03d-%06ld %12.8lf %12.8lf %2d.%03d %2d.%03d %3d ",
               zone, offset,
               (double)star->ra / 3600000., (double)star->spd / 3600000. - 90.,
               star->mag1 / 1000, abs( star->mag1 % 1000),
               star->mag2 / 1000, abs( star->mag2 % 1000),
               star->mag_sigma);

   sprintf( obuff + strlen( obuff), "%2d %1d ",
               star->obj_type, star->double_star_flag);

   sprintf( obuff + strlen( obuff), "%4d.%02d %4d.%02d ",
               (int)epoch_ra / 100, (int)epoch_ra % 100,
               (int)epoch_dec / 100, (int)epoch_dec % 100);

   sprintf( obuff + strlen( obuff),
            "%3d %3d %2d %2d %2d %3d ",
            star->ra_sigma, star->dec_sigma,
            (int)star->n_ucac_total, (int)star->n_ucac_used,
            (int)star->n_cats_used, (int)star->n_cats_total);

   if( star->pm_ra || star->pm_dec || !(output_format & UCAC3_WRITE_SPACES))
      sprintf( obuff + strlen( obuff), "%6d %6d %3d %3d ",
            star->pm_ra, star->pm_dec,
            star->pm_ra_sigma, star->pm_dec_sigma);
   else        /* no proper motion given,  keep these fields blank */
      strcat( obuff, "                      ");

   if( star->twomass_id || !(output_format & UCAC3_WRITE_SPACES))
      {
      sprintf( obuff + strlen( obuff),
            "%10ld %2d.%03d %2d.%03d %2d.%03d ",
            (long)star->twomass_id,
            star->mag_j / 1000, abs( star->mag_j % 1000),
            star->mag_h / 1000, abs( star->mag_h % 1000),
            star->mag_k / 1000, abs( star->mag_k % 1000));

      sprintf( obuff + strlen( obuff), "%03d %03d %03d %02d %02d %02d ",
            star->e2mpho[0], star->e2mpho[1], star->e2mpho[2],
            star->icq_flag[0], star->icq_flag[1], star->icq_flag[2]);
      }
   else        /* no 2MASS data given;  keep these fields blank */
      {
      memset( obuff + 119, ' ', 53);
      obuff[172] = '\0';
      }

   sprintf( obuff + strlen( obuff),
            "%2d.%03d %2d.%03d %2d.%03d ",
            star->sc_bmag / 1000, star->sc_bmag % 1000,
            star->sc_rmag / 1000, star->sc_rmag % 1000,
            star->sc_imag / 1000, star->sc_imag % 1000);

   sprintf( obuff + strlen( obuff),
            "%02d %2d %2d %2d ",
            star->sc_class, star->sc_bmag_quality,
            star->sc_rmag_quality, star->sc_imag_quality);

   if( output_format & UCAC3_WRITE_SPACES)
      for( i = 0; i < 3; i++)
         if( !memcmp( obuff + i * 7 + 172, " 0.000", 6))
            {
            memset( obuff + i * 7 + 172, ' ', 6);
            obuff[197 + i * 3] = ' ';   /* blank out quality flag,  too */
            }

   for( i = 0; i < 10; i++)
      tstr[i] = (char)( '0' + star->catflag[i]);
   tstr[10] = '\0';
   strcat( obuff, tstr);

   sprintf( obuff + strlen( obuff),
            " %1d %1d %03d %03d %9d\n", star->yale_g1, star->yale_c1,
            star->leda_flag, star->twomass_ext_flag, star->running_number);

   return( 0);
}

#if defined( __linux__) || defined( __unix__) || defined( __APPLE__)
   static const char *path_separator = "/", *read_only_permits = "r";
#elif defined( _WIN32) || defined( _WIN64) || defined( __WATCOMC__)
   static const char *path_separator = "\\", *read_only_permits = "rb";
#else
#error "Unknown platform; please report so it can be fixed!"
#endif

static FILE *get_ucac3_zone_file( const int zone_number,
              const int is_supplement, const char *path)
{
   FILE *ifile;
   const char *filename_format = (is_supplement ? "s%02d" : "z%03d");
   char filename[80];

   sprintf( filename, filename_format, zone_number);
   ifile = fopen( filename, read_only_permits);
   if( !ifile)
      {
      strcpy( filename, path);
      if( filename[strlen( filename) - 1] != path_separator[0])
         strcat( filename, path_separator);
      sprintf( filename + strlen( filename), filename_format, zone_number);
      ifile = fopen( filename, read_only_permits);
      }
   return( ifile);
}

#define UCAC3_BAD_ZONE              -1
#define UCAC3_BAD_FSEEK             -2
#define UCAC3_BAD_FREAD             -3
#define UCAC3_FILE_NOT_FOUND        -4

int extract_ucac3_info( const int zone, const long offset, UCAC3_STAR *star,
                     const char *path)
{
   int rval;

   if( zone < 1 || zone > 360)     /* not a valid sequential number */
      rval = UCAC3_BAD_ZONE;
   else
      {
      FILE *ifile = get_ucac3_zone_file( zone, 0, path);

      if( ifile)
         {
         if( fseek( ifile, (offset - 1) * sizeof( UCAC3_STAR), SEEK_SET))
            rval = UCAC3_BAD_FSEEK;
         else if( !fread( star, sizeof( UCAC3_STAR), 1, ifile))
            rval = UCAC3_BAD_FREAD;
         else           /* success! */
            {
            rval = 0;
#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
            flip_ucac3_star( &star);
#endif
#endif
            }
         fclose( ifile);
         }
      else
         rval = UCAC3_FILE_NOT_FOUND;
      }
   return( rval);
}

static FILE *get_ucac3_index_file( const char *path, const int is_supplement)
{
   FILE *index_file;
   const char *idx_filename = (is_supplement ? "bs3idx.bin" : "u3index.bin");

                     /* Look for the index file in the local directory... */
   index_file = fopen( idx_filename, read_only_permits);
                     /* ...and if it's not there,  look for it in the same */
                     /* directory as the data: */
   if( !index_file)
      {
      char filename[100];

      strcpy( filename, path);
      if( filename[strlen( filename) - 1] != path_separator[0])
         strcat( filename, path_separator);
      strcat( filename, idx_filename);
      index_file = fopen( filename, read_only_permits);
      }
   return( index_file);
}

#define UCAC3_FSEEK_FAILED          -1
#define UCAC3_READ1_FAILED          -2
#define UCAC3_READ2_FAILED          -3
#define UCAC3_READ3_FAILED          -4

int extract_ucac3_stars( FILE *ofile, const double ra, const double dec,
                  const double width, const double height, const char *path,
                  const int is_supplement, const int output_format)
{
   const double dec1 = dec - height / 2., dec2 = dec + height / 2.;
   const double ra1 = ra - width / 2., ra2 = ra + width / 2.;
   const double zone_height = (is_supplement ? 5. : .5);
   int zone = (int)( (dec1  + 90.) / zone_height) + 1;
   const int end_zone = (int)( (dec2 + 90.) / zone_height) + 1;
   int ra_start = (int)( ra1 / 1.5);
   int rval = 0;
   static long cached_index_data[3] = {-1L, 0L, 0L};
   FILE *index_file = NULL;

   if( zone < 1)
      zone = 1;
   if( ra_start < 0)
      ra_start = 0;
   while( rval >= 0 && zone <= end_zone)
      {
      FILE *ifile = get_ucac3_zone_file( zone, is_supplement, path);

      if( ifile)
         {
         int keep_going = 1;
         UCAC3_STAR star;
         const int32_t max_ra  = (int32_t)( ra2 * 3600. * 1000.);
         const int32_t min_ra  = (int32_t)( ra1 * 3600. * 1000.);
         const int32_t min_spd = (int32_t)( (dec1 + 90.) * 3600. * 1000.);
         const int32_t max_spd = (int32_t)( (dec2 + 90.) * 3600. * 1000.);
         uint32_t offset, end_offset, toffset, step;
         const long index_offset = ((zone - 1L) * 241L + ra_start) * sizeof( uint32_t);

         if( index_offset == cached_index_data[0])
            {
            offset = cached_index_data[1];
            end_offset = cached_index_data[2];
            }
         else
            {
            if( !index_file)
               index_file = get_ucac3_index_file( path, is_supplement);
            if( index_file)
               {
               if( fseek( index_file, index_offset, SEEK_SET))
                  rval = UCAC3_FSEEK_FAILED;
               if( !fread( &offset, sizeof( uint32_t), 1, index_file))
                  rval = UCAC3_READ1_FAILED;
               if( !fread( &end_offset, 1, sizeof( uint32_t), index_file))
                  rval = UCAC3_READ2_FAILED;
#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
               swap_32( &offset);
               swap_32( &end_offset);
#endif
#endif
               cached_index_data[0] = index_offset;
               cached_index_data[1] = offset;
               cached_index_data[2] = end_offset;
               }
            else     /* no index:  binary-search within entire zone: */
               {
               offset = 0;
               end_offset = ucac3_offsets[zone] - ucac3_offsets[zone - 1];
               }
            }
                     /* Binary-search within the known limits: */
         for( step = 0x800000; step; step >>= 1)
            if( (toffset = offset + step) < end_offset)
               {
               fseek( ifile, toffset * sizeof( UCAC3_STAR), SEEK_SET);
               if( !fread( &star, sizeof( UCAC3_STAR), 1, ifile))
                  rval = UCAC3_READ3_FAILED;
               if( star.ra < min_ra)
                  offset = toffset;
//             printf( "Offset now %ld (tried %ld)\n",
//                       (long)( offset + offset0),
//                       (long)( toffset + offset0));
               }
         fseek( ifile, offset * sizeof( UCAC3_STAR), SEEK_SET);

         while( fread( &star, 1, sizeof( UCAC3_STAR), ifile) && keep_going)
            {
#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
            flip_ucac3_star( &star);
#endif
#endif
            if( star.ra > max_ra)
               keep_going = 0;
            else if( star.ra > min_ra && star.spd > min_spd
                                        && star.spd < max_spd)
               if( !(output_format & UCAC3_OMIT_TYCHO_STARS) ||
                        !star.catflag[UCAC3_CATFLAG_TYCHO])
                  if( star.twomass_id ||
                        (output_format & UCAC3_INCLUDE_DOUBTFULS))
                     {
                     rval++;
                     if( ofile)
                        {
                        char buff[UCAC3_ASCII_SIZE];

                        write_ucac3_star( zone, offset + 1, buff, &star,
                                                         output_format);
                        fwrite( buff, 1, strlen( buff), ofile);
                        }
                     }
            offset++;
            }
         fclose( ifile);
         }
      zone++;
      }
   if( index_file)
      fclose( index_file);

            /* We need some special handling for cases where the area
               to be extracted crosses RA=0 or RA=24: */
   if( rval >= 0 && ra > 0. && ra < 360.)
      {
      if( ra1 < 0.)      /* left side crosses over RA=0h */
         rval += extract_ucac3_stars( ofile, ra+360., dec, width, height,
                                          path, is_supplement, output_format);
      if( ra2 > 360.)    /* right side crosses over RA=24h */
         rval += extract_ucac3_stars( ofile, ra-360., dec, width, height,
                                          path, is_supplement, output_format);
      }
   return( rval);
}
