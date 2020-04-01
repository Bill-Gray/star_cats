# star_cats
C code to access star catalogue data.  Thus far,  this means UCACs 2,
3,  and 4;  URAT1;  Dave Tholen's Gaia32 (compressed,  re-sorted Gaia DR1
and,  later,  DR2 with just the bits needed for asteroid astrometry);
and for CMC-14 and CMC-15.

Code for other catalogues will be added as I get around to it (I have
code for UCAC1,  Ax.0,  Bx.0,  GSC-1.x,  and more...  just not posted yet;
I've focussed on catalogues people are most likely to use.)

Some general notes on how the star catalogue extraction works :

Almost all of the catalogs come to us divided into zones by dec,
with the stars within each zone sorted by RA.  The number of such
dec zones vary;  A1.0 used 24 zones each 7.5 degrees high,
the compressed Gaia-DR1 and DR2 use 180 zones of one degree
each,  and URAT1 uses 900 zones of .2 degrees each.  (Gaia
originally comes to us in HEALPix form,  a decent choice for
some imaging purposes and a terrible one for a star catalog;
I'm glad Dave Tholen reprocessed it into declination zones.)

All of the star catalog extraction code at the above site uses
the following method (sometimes with little flourishes,  as
I'll describe below) :

```
Figure out which declination zones cover the region you want.
For each zone,  do the following :
   Do a binary search to find the "starting point" in RA within
      the zone file.
   Keep reading stars from the zone file until you find one
      that's out of bounds in RA.
   Each star you read in will then be in the proper bounds in
      RA; if it's also in the correct bounds in dec,  you have
      one more star to add to your output.
```
There is also a little bit of logic needed to handle situations
where the desired region spans RA=0.  For that,  the above procedure
is done twice,  once at the "low" RA end,  and again at the "high"
RA end.

For some catalogs,  an index is available that may shrink the
size of the binary search you have to do.  If the index gives you,
say,  the starting point for data within each zone file at two-degree
intervals,  and your desired rectangle in RA spans from 314.1592
to 320.9321 degrees,  you would use the index to find out where
RA=314 degrees and RA=316 degrees are in the zone,  and binary-search
between those values (instead of searching the entire zone file).

In a few cases,  I've gone a step beyond that.  In the above
case,  if the stars are uniformly distributed over that section,
RA=314.1592 will be much closer to the RA=314 point than the RA=316
point,  and we can benefit (very slightly) by interpolating
linearly.  Similarly,  if you don't have an index,  you can
start out by assuming RA=314.1592 is going to be closer to the
end of the zone file than it is to the beginning.  But I am
not really convinced I have ever gotten much benefit over the
simple "binary search the entire zone file" procedure.  See
`gaia32.c` and the "notes on indexing" comments for further
explanation.

