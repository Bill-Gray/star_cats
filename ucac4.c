#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "ucac4.h"

/* Basic access functions for UCAC-4.  Public domain.  Please contact
pluto (at) projectpluto.com with comments/bug fixes.  */

/* History:
     2016 Jul 27:  (BJG) Uwe Lange pointed out that if you fed
         extract_ucac4_stars( ) an RA=0 rectangle,  you'd get half
         the area you would expect,  due to the use of 'ra > 0.'
         when 'ra >= 0.' was called for.  The wrapping around RA=0
         wouldn't be handled correctly.
     2012 Dec 18:  (BJG) Modified the way 'path_separator' is defined,
         following comments from Skip Gaede.
     2012 Oct  6:  (BJG) Changed the 'ucac4_offsets' table following
         a comment from Thomas Meyer.  Also fixed a compiler warning
         for 64-bit gcc compiles.
     2012 Mar 18:  (BJG) Created using the UCAC3 code as a template.

   UCAC4 consists of 900 zones,  each .2 degrees high in declination.  */

#ifdef CUMULATIVE_DESIGNATORS_USED
      /* ...unless we end up with a designation system that is a */
      /* cumulative count across the entire catalog,  instead of */
      /* a zone/number-within-zone designation scheme.  I don't  */
      /* know which will be used yet,  so I've #ifdeffed this    */
      /* out but am not deleting it yet.  (Updated 2012 Oct 6    */
      /* after Thomas Meyer pointed out that the counts changed  */
      /* in "final" UCAC4... I'd thought they were the same as   */
      /* for the last beta UCAC4.)                                */

static const int32_t ucac4_offsets[901] = {
        0,       206,       866,      2009,      3622,      5571,
     8119,     11083,     14581,     18489,     22819,     27668,
    32894,     38573,     44607,     51082,     57958,     65121,
    73127,     81477,     90378,     99900,    109802,    120446,
   131065,    141931,    153538,    165249,    177939,    190392,
   203260,    217159,    230951,    245734,    260431,    275436,
   291126,    306752,    323254,    339777,    356846,    374652,
   392624,    411941,    430902,    450363,    470606,    490358,
   511910,    533927,    556958,    580905,    605144,    630950,
   656199,    682348,    710490,    738329,    768199,    797923,
   828164,    859126,    889860,    922477,    954394,    987830,
  1022269,   1057078,   1094344,   1131834,   1171359,   1212536,
  1254465,   1299018,   1343515,   1389817,   1437317,   1485219,
  1537695,   1590495,   1645901,   1703202,   1761169,   1824506,
  1888223,   1954356,   2020470,   2087022,   2158100,   2229191,
  2305721,   2382212,   2458300,   2539123,   2619349,   2703210,
  2791533,   2880027,   2978574,   3078328,   3183748,   3290888,
  3399359,   3517005,   3631355,   3748916,   3869477,   3989210,
  4116605,   4242319,   4372398,   4501762,   4630481,   4766342,
  4900168,   5042819,   5189736,   5334359,   5489793,   5643980,
  5806385,   5977568,   6149435,   6333019,   6511317,   6692701,
  6879467,   7058247,   7239359,   7414409,   7595970,   7776344,
  7954946,   8141953,   8321021,   8500537,   8682095,   8862104,
  9056791,   9248932,   9451925,   9661614,   9868626,  10087593,
 10300711,  10528170,  10763808,  10996731,  11233228,  11454960,
 11680371,  11908532,  12123889,  12350099,  12571174,  12794587,
 13011373,  13221888,  13442290,  13657603,  13879927,  14101900,
 14313978,  14531700,  14740986,  14955694,  15171738,  15379548,
 15597692,  15805799,  16016969,  16228803,  16436326,  16654688,
 16865418,  17085213,  17309250,  17532749,  17765087,  17988124,
 18217891,  18439715,  18658340,  18884483,  19101745,  19321891,
 19539134,  19747341,  19959985,  20163861,  20371975,  20581215,
 20781853,  20988391,  21185410,  21384610,  21583477,  21774869,
 21970824,  22156223,  22349423,  22545048,  22735298,  22929646,
 23114618,  23302955,  23491044,  23676507,  23869997,  24053838,
 24244343,  24436722,  24625544,  24829905,  25021983,  25217864,
 25416228,  25608716,  25806921,  25995259,  26188247,  26377390,
 26561968,  26756231,  26945830,  27138463,  27339618,  27537932,
 27740262,  27932869,  28125583,  28312614,  28492434,  28681742,
 28866124,  29055607,  29240628,  29417290,  29600055,  29777098,
 29961261,  30150095,  30334590,  30528871,  30716228,  30907754,
 31098418,  31284004,  31477513,  31662832,  31854812,  32051240,
 32243043,  32440888,  32628388,  32823021,  33019133,  33212523,
 33415168,  33609490,  33810511,  34010171,  34206508,  34408580,
 34599431,  34796787,  34992977,  35185326,  35386116,  35578466,
 35778277,  35977036,  36175391,  36385122,  36589385,  36804864,
 37023530,  37240803,  37473506,  37695961,  37922800,  38147628,
 38367827,  38595180,  38809280,  39033072,  39256521,  39472742,
 39700062,  39918074,  40144228,  40372376,  40595563,  40831679,
 41062162,  41303620,  41542107,  41776968,  42024798,  42273897,
 42541624,  42817573,  43094175,  43373798,  43646660,  43933354,
 44220187,  44495569,  44774133,  45040789,  45308313,  45574155,
 45834129,  46098964,  46350968,  46600321,  46846171,  47085311,
 47330792,  47563051,  47796324,  48030511,  48253973,  48479827,
 48699017,  48920680,  49130155,  49331708,  49540601,  49744729,
 49953998,  50168503,  50382947,  50608721,  50829286,  51050975,
 51260138,  51465690,  51676431,  51877759,  52083594,  52290253,
 52491276,  52699971,  52902339,  53107484,  53306055,  53499449,
 53697396,  53890368,  54086859,  54282812,  54473399,  54669524,
 54864451,  55068329,  55272263,  55474160,  55684640,  55891717,
 56106626,  56313778,  56517087,  56719068,  56906189,  57098261,
 57286238,  57469990,  57661984,  57849177,  58039500,  58226686,
 58408804,  58597780,  58778477,  58962543,  59142925,  59317148,
 59493080,  59660242,  59829339,  60000639,  60165139,  60331580,
 60490701,  60652703,  60806469,  60954323,  61103407,  61242855,
 61382639,  61518319,  61649272,  61785467,  61915155,  62047604,
 62183916,  62317316,  62456318,  62591678,  62731080,  62864336,
 62996028,  63135877,  63270249,  63407189,  63549552,  63689231,
 63835367,  63976278,  64121582,  64259605,  64396036,  64545002,
 64689570,  64837067,  64989790,  65139787,  65294966,  65443898,
 65593770,  65737210,  65875713,  66017898,  66152505,  66286004,
 66420538,  66546986,  66673501,  66793457,  66915552,  67035640,
 67154315,  67277382,  67396894,  67518570,  67639362,  67758147,
 67883233,  68007652,  68133882,  68263054,  68393024,  68528287,
 68660846,  68798343,  68939883,  69077494,  69218568,  69356414,
 69494772,  69628182,  69752948,  69879422,  70000000,  70122489,
 70247042,  70369726,  70499032,  70625510,  70756301,  70888439,
 71016707,  71152238,  71284717,  71421988,  71560245,  71695931,
 71837567,  71974077,  72111270,  72244853,  72375100,  72509726,
 72638278,  72770367,  72904862,  73036546,  73175066,  73308626,
 73446524,  73583711,  73718553,  73859704,  73995358,  74132891,
 74273066,  74411863,  74557598,  74699794,  74845573,  74997682,
 75145826,  75298303,  75444061,  75591252,  75740192,  75885351,
 76036316,  76182707,  76332341,  76481212,  76626912,  76777290,
 76924016,  77072545,  77220806,  77367665,  77520082,  77668235,
 77817940,  77962496,  78101866,  78246948,  78388995,  78532783,
 78680569,  78823200,  78970749,  79115274,  79261629,  79408829,
 79554307,  79706677,  79856335,  80006796,  80159529,  80310091,
 80464841,  80615040,  80767205,  80916754,  81063021,  81212605,
 81357318,  81505366,  81656905,  81805384,  81958748,  82110157,
 82263902,  82414415,  82562970,  82714837,  82862328,  83011435,
 83160218,  83305036,  83454952,  83602274,  83751699,  83895816,
 84035853,  84176423,  84308765,  84445863,  84587199,  84725950,
 84871152,  85014777,  85159003,  85302572,  85442985,  85583778,
 85718442,  85857369,  85989969,  86120147,  86255433,  86386574,
 86517915,  86651270,  86781435,  86917762,  87051228,  87188041,
 87321820,  87454330,  87590888,  87717957,  87847315,  87980758,
 88112049,  88246916,  88379152,  88513103,  88645999,  88777173,
 88914085,  89048876,  89188978,  89329488,  89469047,  89613632,
 89751798,  89892003,  90035280,  90175660,  90318349,  90457592,
 90600846,  90743771,  90883448,  91026350,  91164045,  91302701,
 91441215,  91576464,  91717811,  91853028,  91990293,  92123993,
 92254550,  92392276,  92524773,  92660650,  92799218,  92936875,
 93079327,  93214018,  93351020,  93489413,  93622451,  93757977,
 93888024,  94019910,  94151201,  94281405,  94415382,  94545708,
 94680124,  94817225,  94951755,  95088074,  95217537,  95348768,
 95477415,  95603418,  95729780,  95850935,  95975067,  96103806,
 96229552,  96356562,  96480871,  96606848,  96727723,  96844893,
 96966776,  97084298,  97202021,  97322351,  97440931,  97563841,
 97683437,  97802395,  97919245,  98034818,  98152991,  98268305,
 98385043,  98504314,  98621035,  98742124,  98862327,  98984957,
 99107874,  99226773,  99349545,  99469159,  99591079,  99713559,
 99834417,  99960553, 100085711, 100216077, 100345899, 100475483,
100611206, 100743791, 100878171, 101013209, 101147596, 101285647,
101417654, 101550819, 101684614, 101815623, 101948331, 102078830,
102210533, 102340513, 102468448, 102600533, 102729514, 102860002,
102990083, 103117998, 103248830, 103375566, 103504685, 103631455,
103755256, 103880872, 104001271, 104123504, 104243544, 104362778,
104485925, 104606429, 104729076, 104851037, 104969592, 105091087,
105207601, 105324685, 105442071, 105558396, 105677009, 105791567,
105906882, 106022309, 106136833, 106254038, 106366316, 106478862,
106589608, 106697266, 106807861, 106913773, 107020853, 107125835,
107229554, 107338668, 107444628, 107551179, 107658962, 107764027,
107868817, 107967596, 108066211, 108164789, 108260531, 108354468,
108442540, 108530907, 108620626, 108706972, 108796077, 108881829,
108968489, 109054223, 109138734, 109226189, 109310137, 109395666,
109479094, 109559885, 109641403, 109719088, 109797253, 109874551,
109950285, 110027584, 110104144, 110181651, 110258477, 110332978,
110406118, 110474472, 110541762, 110607268, 110671051, 110734233,
110792647, 110849575, 110905502, 110958489, 111011073, 111060261,
111108535, 111154760, 111199322, 111244097, 111287136, 111330391,
111373309, 111415186, 111458920, 111501560, 111544337, 111589511,
111633699, 111678480, 111720970, 111763013, 111803903, 111843617,
111884637, 111923823, 111963614, 112003734, 112042548, 112082765,
112121146, 112160295, 112197992, 112234519, 112271997, 112308010,
112344241, 112379998, 112414452, 112449208, 112481869, 112514181,
112545824, 112576355, 112607582, 112638496, 112669658, 112700318,
112729873, 112759770, 112787250, 112814777, 112841198, 112867111,
112893427, 112918547, 112943934, 112968806, 112992599, 113016424,
113038694, 113061223, 113083362, 113104958, 113127541, 113149301,
113170825, 113192396, 113213449, 113234242, 113254229, 113274254,
113293981, 113313222, 113332882, 113351268, 113369624, 113387579,
113405111, 113422042, 113438132, 113453916, 113468857, 113483559,
113498262, 113512256, 113526014, 113539193, 113551968, 113564648,
113576520, 113588515, 113600106, 113611115, 113621938, 113632032,
113641983, 113651355, 113660500, 113669530, 113677889, 113686342,
113694302, 113702051, 113709363, 113715811, 113722038, 113727800,
113733234, 113738427, 113743252, 113747720, 113751753, 113755465,
113758958, 113761983, 113764935, 113767665, 113770070, 113772225,
113774103, 113775753, 113777249, 113778576, 113779410, 113779922,
113780093 };
#endif     // CUMULATIVE_DESIGNATORS_USED

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

void flip_ucac4_star( UCAC4_STAR *star)
{
   int i;

   swap_32( &star->ra);
   swap_32( &star->spd);
   swap_16( &star->mag1);
   swap_16( &star->mag2);
   swap_16( &star->epoch_ra);
   swap_16( &star->epoch_dec);
   swap_32( &star->pm_ra);
   swap_32( &star->pm_dec);
   swap_32( &star->twomass_id);
   swap_16( &star->mag_j);
   swap_16( &star->mag_h);
   swap_16( &star->mag_k);
   for( i = 0; i < 5; i++)
      swap_16( &star->apass_mag[i]);
   swap_32( &star->catalog_flags);
   swap_32( &star->id_number);
   swap_16( &star->ucac2_zone);
   swap_32( &star->ucac2_number);
}
#endif                   // #if __BYTE_ORDER == __BIG_ENDIAN
#endif                   // #ifdef __BYTE_ORDER

    /* Storing proper motions in a 16-bit int in units of .1 mas/year */
    /* allows storing of proper motions of up to 3.2766"/year.  But   */
    /* 25 stars in UCAC4 are faster than this.  For them,  the proper */
    /* motion is stored as 32767 in one or both axes,  and one must   */
    /* look up the real proper motion using the following function.   */

int32_t get_actual_proper_motion( const UCAC4_STAR *star, const int get_dec_pm)
{
   int32_t rval = 0;

   if( star->pm_ra != 32767 && star->pm_dec != 32767)
      rval = (get_dec_pm ? star->pm_dec : star->pm_ra);
   else
      {           /* table copied from /u4i/u4hpm.dat of first DVD */
      const int32_t pm_lookup_table[] = {
                       1,    41087,   31413,
                       2,    41558,   32586,
                  200137,   -37758,    7655,
                  200168,   -36004,    9521,
                  200169,   -36782,    4818,
                  200229,    39624,  -25374,
                  200400,    65051,  -57308,
                  200503,    56347,  -23377,
                  200530,    67682,   13275,
                  200895,   -22401,  -34203,
                  201050,    -7986,  103281,
                  201349,    22819,   53694,
                  201526,    -5803,  -47659,
                  201550,    40033,  -58151,
                  201567,    41683,   32691,
                  201633,   -44099,    9416,
                  201803,    34222,  -15989,
                  249921,   -10015,  -35427,
                  249984,    -9994,  -35419,
                  268357,     5713,  -36943,
                80118783,    32962,    5639,
                93157181,   -22393,  -34199,
               106363470,   -37060,  -11490,
               110589580,   -38420,  -27250,
               113038183,    10990,  -51230, 0 };
      int i;

      for( i = 0; pm_lookup_table[i]; i += 3)
         if( pm_lookup_table[i] == (int32_t)star->id_number)
            rval = pm_lookup_table[i + (get_dec_pm ? 2 : 1)];
      assert( rval);
      }
   return( rval);
}

int get_actual_proper_motion_sigma( const int8_t pm_sigma)
{
   int rval = pm_sigma + 128;
   const int16_t lookup_table[5] = { 275, 325, 375, 450, 500 };

   if( rval >= 251)
      rval = lookup_table[rval - 251];
   return( rval);
}


/* The following function writes out a UCAC4 star in the same ASCII */
/* format as the FORTRAN code.                                      */

int write_ucac4_star_fortran_style( char *obuff, const UCAC4_STAR *star)
{
/*
          WRITE (line,'(2i10,2i6,i3,i2,i3,2i4,3i3,2i6,2i7,2i4
     .                ,i11,3i6,6i3,5i6,5i4,i2,9i2,2i3,i10,i4,i7)')

 451313731,   544918,16176,16187,12,0, 0, 58, 76, 4, 4, 2, 9762, 9709,   -47,   121,139,140, 863323727,13612,12869,12680, 5, 5, 5, 3, 3, 3,20000,20000,20000,20000,20000,  0,  0,  0,  0,  0,4,0,0,0,0,0,0,0,0,1, 0, 0,  1000284,  0,     0

*/
   int i;
   char *tptr;

   sprintf( obuff, "%10d%10d%6d%6d%3d%2d%3d%4d%4d%3d%3d%3d%6d%6d%7d%7d%4d%4d",
        star->ra, star->spd, star->mag1, star->mag2,
        star->mag_sigma, star->obj_type, star->double_star_flag,
        star->ra_sigma + 128, star->dec_sigma + 128, star->n_ucac_total,
        star->n_ucac_used, star->n_cats_used,
        star->epoch_ra, star->epoch_dec,
        get_actual_proper_motion( star, 0),
        get_actual_proper_motion( star, 1),
        get_actual_proper_motion_sigma( star->pm_ra_sigma),
        get_actual_proper_motion_sigma( star->pm_dec_sigma));
   sprintf( obuff + strlen( obuff), "%11d%6d%6d%6d%3d%3d%3d%3d%3d%3d",
        star->twomass_id, star->mag_j, star->mag_h, star->mag_k,
        star->icq_flag[0], star->icq_flag[1], star->icq_flag[2],
        star->e2mpho[0], star->e2mpho[1], star->e2mpho[2]);
   for( i = 0; i < 5; i++)
      sprintf( obuff + strlen( obuff), "%6d", star->apass_mag[i]);
   for( i = 0; i < 5; i++)
      sprintf( obuff + strlen( obuff), "%4d", star->apass_mag_sigma[i]);
   sprintf( obuff + strlen( obuff), "%2d ", star->yale_gc_flags);
               /* Show catalog flags as separate digits: */
   tptr = obuff + strlen( obuff);
   sprintf( tptr, "%09d", star->catalog_flags);
   for( i = 8; i >= 0; i--)
      {
      tptr[i + i] = tptr[i];
      tptr[i + i + 1] = ' ';
      }
   tptr[17] = '\0';
   sprintf( obuff + strlen( obuff), "%3d%3d%10d%4d%7d\n",
       star->leda_flag, star->twomass_ext_flag,
       star->id_number, star->ucac2_zone, star->ucac2_number);
   return( 0);
}

/* This function,  like the above,  writes out the data for a UCAC-4 star */
/* into an ASCII buffer,  but it tries for a more human-readable format.  */

int write_ucac4_star( const int zone, const long offset, char *obuff,
                     const UCAC4_STAR *star, const int output_format)
{
   const long epoch_ra  = 190000 + star->epoch_ra;
   const long epoch_dec = 190000 + star->epoch_dec;
   int i;

   if( output_format & UCAC4_FORTRAN_STYLE)
      return( write_ucac4_star_fortran_style( obuff, star));

   sprintf( obuff, "%03d-%06ld %12.8lf %12.8lf %2d.%03d %2d.%03d %3d ",
               zone, offset,
               (double)star->ra / 3600000., (double)star->spd / 3600000. - 90.,
               star->mag1 / 1000, abs( star->mag1 % 1000),
               star->mag2 / 1000, abs( star->mag2 % 1000),
               star->mag_sigma);

   sprintf( obuff + strlen( obuff), "%2d %2d ",
               star->obj_type, star->double_star_flag);

   sprintf( obuff + strlen( obuff), "%4d.%02d %4d.%02d ",
               (int)epoch_ra / 100, (int)epoch_ra % 100,
               (int)epoch_dec / 100, (int)epoch_dec % 100);

   sprintf( obuff + strlen( obuff),
            "%3d %3d %2d %2d %2d ",
            star->ra_sigma + 128, star->dec_sigma + 128,
            (int)star->n_ucac_total, (int)star->n_ucac_used,
            (int)star->n_cats_used);

   if( star->pm_ra || star->pm_dec || !(output_format & UCAC4_WRITE_SPACES))
      sprintf( obuff + strlen( obuff), "%6d %6d %3d %3d ",
            get_actual_proper_motion( star, 0),
            get_actual_proper_motion( star, 1),
            get_actual_proper_motion_sigma( star->pm_ra_sigma),
            get_actual_proper_motion_sigma( star->pm_dec_sigma));
   else        /* no proper motion given,  keep these fields blank */
      strcat( obuff, "                      ");

   if( star->twomass_id || !(output_format & UCAC4_WRITE_SPACES))
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
      memset( obuff + 116, ' ', 53);
      obuff[169] = '\0';
      }

   for( i = 0; i < 5; i++)
      if( (star->apass_mag[i] && star->apass_mag[i] != 20000)
                   || !(output_format & UCAC4_WRITE_SPACES))
         sprintf( obuff + strlen( obuff), "%2d.%03d ",
                star->apass_mag[i] / 1000, star->apass_mag[i] % 1000);
         else
            strcat( obuff, "       ");
   for( i = 0; i < 5; i++)
      if( star->apass_mag_sigma[i] != 99
                          || !(output_format & UCAC4_WRITE_SPACES))
         sprintf( obuff + strlen( obuff), "%c%d.%02d ",
                   (star->apass_mag_sigma[i] < 0 ? '-' : ' '),
                   abs( star->apass_mag_sigma[i]) / 100,
                   abs( star->apass_mag_sigma[i]) % 100);
      else
         strcat( obuff, "      ");

   sprintf( obuff + strlen( obuff), "%09d", star->catalog_flags);
   sprintf( obuff + strlen( obuff),
            " %2d %03d %03d %9d", star->yale_gc_flags,
            star->leda_flag, star->twomass_ext_flag, star->id_number);
   if( star->ucac2_zone || !(output_format & UCAC4_WRITE_SPACES))
      sprintf( obuff + strlen( obuff), " %03d-%06d\n",
               star->ucac2_zone, star->ucac2_number);
   else
      strcat( obuff, "           \n");
   return( 0);
}

#if defined( __linux__) || defined( __unix__) || defined( __APPLE__)
   static const char *path_separator = "/", *read_only_permits = "r";
#elif defined( _WIN32) || defined( _WIN64) || defined( __WATCOMC__)
   static const char *path_separator = "\\", *read_only_permits = "rb";
#else
#error "Unknown platform; please report so it can be fixed!"
#endif

/* The layout of UCAC-4 is such that data files are in the 'u4b'
folders of the two DVDs. People may copy these retaining the path
structure,  or maybe they'll put all 900 files in one folder.  So if
you ask this function for,  say, zone_number = 314 and files in the
folder /data/ucac4,  the function will look for the data under the
following four names:

z314         (i.e.,  all data copied to the current folder)
u4b/z314     (i.e.,  you've copied everything to the u4b subfolder)
/data/ucac4/z314
/data/ucac4/u4b/z314

   ...stopping when it finds a file.  This will,  I hope,  cover all
likely situations.  If you make things any more complicated,  you've
only yourself to blame.  Note that the index file 'u4index.asc' is
similarly looked for in various possible folders;  and the code will
still work even if the index isn't found -- the result will just be
a tiny bit slower. */

static FILE *get_ucac4_zone_file( const int zone_number, const char *path)
{
   FILE *ifile;
   char filename[80];

   sprintf( filename, "u4b%sz%03d", path_separator, zone_number);
            /* First,  look for file in current path: */
   ifile = fopen( filename + 4, read_only_permits);
   if( !ifile)
      ifile = fopen( filename, read_only_permits);
         /* If file isn't there,  use the 'path' passed in as an argument: */
   if( !ifile && *path)
      {
      char filename2[80], *endptr;
      int i;

      strcpy( filename2, path);
      endptr = filename2 + strlen( filename2);
      if( endptr[-1] != *path_separator)
         *endptr++ = *path_separator;
      for( i = 0; !ifile && i < 2; i++)
         {
         strcpy( endptr, filename + 4 * (1 - i));
         ifile = fopen( filename2, read_only_permits);
         }
      }
   return( ifile);
}

int extract_ucac4_info( const int zone, const long offset, UCAC4_STAR *star,
                     const char *path)
{
   int rval;

   if( zone < 1 || zone > 900)     /* not a valid sequential number */
      rval = -1;
   else
      {
      FILE *ifile = get_ucac4_zone_file( zone, path);

      if( ifile)
         {
         if( fseek( ifile, (offset - 1) * sizeof( UCAC4_STAR), SEEK_SET))
            rval = -2;
         else if( !fread( star, sizeof( UCAC4_STAR), 1, ifile))
            rval = -3;
         else           /* success! */
            {
            rval = 0;
#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
            flip_ucac4_star( star);
#endif
#endif
            }
         fclose( ifile);
         }
      else
         rval = -4;
      }
   return( rval);
}

/* This looks for the index file u4index.asc first in the current
directory;  then in 'path';  then in the u4i subdirectory under 'path'.
One of the three will probably work... */

static FILE *get_ucac4_index_file( const char *path)
{
   FILE *index_file;
   const char *idx_filename = "u4index.asc";

                     /* Look for the index file in the local directory... */
   index_file = fopen( idx_filename, read_only_permits);
                     /* ...and if it's not there,  look for it in the same */
                     /* directory as the data: */
   if( !index_file)
      {
      char filename[100], *tptr;
      int i;

      strcpy( filename, path);
      if( filename[strlen( filename) - 1] != path_separator[0])
         strcat( filename, path_separator);
      tptr = filename + strlen( filename);
      for( i = 0; i < 2 && !index_file; i++)
         {
         if( i)      /* try 'u4i' folder */
            {
            strcpy( tptr, "u4i");
            strcat( tptr, path_separator);
            }
         strcat( filename, idx_filename);
         index_file = fopen( filename, read_only_permits);
//       printf( "Trying '%s': %p\n", filename, index_file);
         }
      }
   return( index_file);
}

/* The layout of the ASCII index is a bit peculiar.  There are 1440
lines per dec zone (of which there are,  of course,  900). Each line
contains 21 bytes,  except for the first,  which includes the dec
and is therefore six bytes longer. */

static long get_index_file_offset( const int zone, const int ra_start)
{
   int rval = (zone - 1) * (1440 * 21 + 6) + ra_start * 21;

   if( ra_start)
      rval += 6;
   return( rval);
}

/* RA, dec, width, height are in degrees */

/* A note on indexing:  within each zone,  we want to locate the stars
within a particular range in RA.  If an index is unavailable,  then
we have things narrowed down to somewhere between the first and
last records.  If an index is available,  our search can take
place within a narrower range.  But in either case,  the range is
refined by doing a secant search which narrows down the starting
point to within 'acceptable_limit' records,  currently set to
40;  i.e.,  it's possible that we will read in forty records that
are before the low end of the desired RA range.  The secant search
is slightly modified to ensure that each iteration knocks off at
least 1/8 of the current range.

   Records are then read in 'buffsize' stars at a time and,  if
they're in the desired RA/dec rectangle,  written out to 'ofile'. */

#include <time.h>

clock_t time_searching = 0;

#define UCAC4_FGETS_FAILED         -1
#define UCAC4_FREAD_FAILED				     -2
#define UCAC4_FSEEK_FAILED         -3
#define UCAC4_FSEEK2_FAILED        -4
#define UCAC4_FSEEK3_FAILED        -5
#define UCAC4_SSCANF_FAILED        -6
#define UCAC4_ALLOC_FAILED         -7

int extract_ucac4_stars( FILE *ofile, const double ra, const double dec,
                  const double width, const double height, const char *path,
                  const int output_format)
{
   const double dec1 = dec - height / 2., dec2 = dec + height / 2.;
   const double ra1 = ra - width / 2., ra2 = ra + width / 2.;
   const double zone_height = .2;    /* zones are .2 degrees each */
   int zone = (int)( (dec1  + 90.) / zone_height) + 1;
   const int end_zone = (int)( (dec2 + 90.) / zone_height) + 1;
   const int index_ra_resolution = 1440;  /* = .25 degrees */
   int ra_start = (int)( ra1 * (double)index_ra_resolution / 360.);
   int rval = 0;
   const int buffsize = 400;     /* read this many stars at a try */
   FILE *index_file = get_ucac4_index_file( path);
   UCAC4_STAR *stars = (UCAC4_STAR *)calloc( buffsize, sizeof( UCAC4_STAR));

   if( !stars)
      rval = UCAC4_ALLOC_FAILED;
   if( zone < 1)
      zone = 1;
   if( ra_start < 0)
      ra_start = 0;
   while( rval >= 0 && zone <= end_zone)
      {
      FILE *ifile = get_ucac4_zone_file( zone, path);

      if( ifile)
         {
         int keep_going = 1;
         int i, n_read;
         const int32_t max_ra  = (int32_t)( ra2 * 3600. * 1000.);
         const int32_t min_ra  = (int32_t)( ra1 * 3600. * 1000.);
         const int32_t min_spd = (int32_t)( (dec1 + 90.) * 3600. * 1000.);
         const int32_t max_spd = (int32_t)( (dec2 + 90.) * 3600. * 1000.);
         uint32_t offset, end_offset;
         const uint32_t acceptable_limit = 40;
         long index_file_offset = get_index_file_offset( zone, ra_start);
         clock_t t0 = clock( );
         static long cached_index_data[5] = {-1L, 0L, 0L, 0L, 0L};
         const uint32_t ra_range = (uint32_t)( 360 * 3600 * 1000);
         uint32_t ra_lo = (uint32_t)( ra_start * (ra_range / index_ra_resolution));
         uint32_t ra_hi = ra_lo + ra_range / index_ra_resolution;

         if( index_file_offset == cached_index_data[0])
            {
            offset = cached_index_data[1];
            end_offset = cached_index_data[2];
            }
         else
            {
            if( index_file)
               {
               char ibuff[50];
               unsigned long ul_offset, ul_end_offset;

               if( fseek( index_file, index_file_offset, SEEK_SET))
                  rval = UCAC4_FSEEK_FAILED;
               if( !fgets( ibuff, sizeof( ibuff), index_file))
                  rval = UCAC4_FGETS_FAILED;
               if( sscanf( ibuff, "%lu%lu", &ul_offset, &ul_end_offset) != 2)
                  rval = UCAC4_SSCANF_FAILED;
               offset = (uint32_t)ul_offset;
               end_offset = (uint32_t)ul_end_offset;
               end_offset += offset;
               cached_index_data[0] = index_file_offset;
               cached_index_data[1] = offset;
               cached_index_data[2] = end_offset;
               }
            else     /* no index:  binary-search within entire zone: */
               {
               offset = 0;
               if( fseek( ifile, 0L, SEEK_END))
                  rval = UCAC4_FSEEK2_FAILED;
               end_offset = ftell( ifile) / sizeof( UCAC4_STAR);
//             end_offset = ucac4_offsets[zone] - ucac4_offsets[zone - 1];
               ra_lo = 0;
               ra_hi = ra_range;
               }
            }
//       printf( "Seeking RA=%u between offsets %u to %u (%u)\n",
//                   min_ra, offset, end_offset, end_offset - offset);
//                   /* Secant-search within the known limits: */
         while( rval >= 0 && end_offset - offset > acceptable_limit)
            {
            UCAC4_STAR star;
            uint32_t delta = end_offset - offset, toffset;
            uint32_t minimum_bite = delta / 8 + 1;
            uint64_t tval = (uint64_t)delta *
                        (uint64_t)( min_ra - ra_lo) / (uint64_t)( ra_hi - ra_lo);

            if( tval < minimum_bite)
               tval = minimum_bite;
            else if( tval > delta - minimum_bite)
               tval = delta - minimum_bite;
            toffset = offset + (uint32_t)tval;
            if( fseek( ifile, toffset * sizeof( UCAC4_STAR), SEEK_SET))
               rval = UCAC4_FSEEK3_FAILED;
            if( fread( &star, sizeof( UCAC4_STAR), 1, ifile) != 1)
               rval = UCAC4_FREAD_FAILED;
            if( star.ra < min_ra)
               {
               offset = toffset;
               ra_lo = star.ra;
               }
            else
               {
               end_offset = toffset;
               ra_hi = star.ra;
               }
            }
         time_searching += clock( ) - t0;
         fseek( ifile, offset * sizeof( UCAC4_STAR), SEEK_SET);

         while( rval >= 0 && keep_going &&
                  (n_read = fread( stars, sizeof( UCAC4_STAR), buffsize, ifile)) > 0)
            for( i = 0; i < n_read && keep_going; i++)
               {
               UCAC4_STAR star = stars[i];

#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
               flip_ucac4_star( &star);
#endif
#endif
               if( star.ra > max_ra)
                  keep_going = 0;
               else if( star.ra > min_ra && star.spd > min_spd
                                           && star.spd < max_spd)
                  if( !(output_format & UCAC4_OMIT_TYCHO_STARS) ||
                           !ucac4_tycho_catflag( star.catalog_flags))
                     if( star.twomass_id ||
                           (output_format & UCAC4_INCLUDE_DOUBTFULS))
                        {
                        rval++;
                        if( ofile)
                           {
                           if( output_format & UCAC4_RAW_BINARY)
                              fwrite( &star, 1, sizeof( UCAC4_STAR), ofile);
                           else
                              {
                              char buff[UCAC4_ASCII_SIZE];

                              write_ucac4_star( zone, offset + 1, buff, &star,
                                                            output_format);
                              fwrite( buff, 1, strlen( buff), ofile);
                              }
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
   free( stars);

            /* We need some special handling for cases where the area
               to be extracted crosses RA=0 or RA=24: */
   if( rval >= 0 && ra >= 0. && ra < 360.)
      {
      if( ra1 < 0.)      /* left side crosses over RA=0h */
         rval += extract_ucac4_stars( ofile, ra+360., dec, width, height,
                                          path, output_format);
      if( ra2 > 360.)    /* right side crosses over RA=24h */
         rval += extract_ucac4_stars( ofile, ra-360., dec, width, height,
                                          path, output_format);
      }
   return( rval);
}
