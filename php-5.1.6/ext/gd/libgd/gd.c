
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "gd.h"
#include "gdhelpers.h"

#include "php.h"

#ifdef _MSC_VER
# if _MSC_VER >= 1300
/* in MSVC.NET these are available but only for __cplusplus and not _MSC_EXTENSIONS */
#  if !defined(_MSC_EXTENSIONS) && defined(__cplusplus)
#   define HAVE_FABSF 1
extern float fabsf(float x);
#   define HAVE_FLOORF 1
extern float floorf(float x);
#  endif /*MSVC.NET */
# endif /* MSC */
#endif
#ifndef HAVE_FABSF
# define HAVE_FABSF 0
#endif
#ifndef HAVE_FLOORF
# define HAVE_FLOORF 0
#endif
#if HAVE_FABSF == 0
/* float fabsf(float x); */
# define fabsf(x) ((float)(fabs(x)))
#endif
#if HAVE_FLOORF == 0
/* float floorf(float x);*/
#define floorf(x) ((float)(floor(x)))
#endif

#ifdef _OSD_POSIX		/* BS2000 uses the EBCDIC char set instead of ASCII */
#define CHARSET_EBCDIC
#define __attribute__(any)	/*nothing */
#endif
/*_OSD_POSIX*/

#ifndef CHARSET_EBCDIC
#define ASC(ch)  ch
#else /*CHARSET_EBCDIC */
#define ASC(ch) gd_toascii[(unsigned char)ch]
static const unsigned char gd_toascii[256] =
{
/*00 */ 0x00, 0x01, 0x02, 0x03, 0x85, 0x09, 0x86, 0x7f,
  0x87, 0x8d, 0x8e, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,	/*................ */
/*10 */ 0x10, 0x11, 0x12, 0x13, 0x8f, 0x0a, 0x08, 0x97,
  0x18, 0x19, 0x9c, 0x9d, 0x1c, 0x1d, 0x1e, 0x1f,	/*................ */
/*20 */ 0x80, 0x81, 0x82, 0x83, 0x84, 0x92, 0x17, 0x1b,
  0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x05, 0x06, 0x07,	/*................ */
/*30 */ 0x90, 0x91, 0x16, 0x93, 0x94, 0x95, 0x96, 0x04,
  0x98, 0x99, 0x9a, 0x9b, 0x14, 0x15, 0x9e, 0x1a,	/*................ */
/*40 */ 0x20, 0xa0, 0xe2, 0xe4, 0xe0, 0xe1, 0xe3, 0xe5,
  0xe7, 0xf1, 0x60, 0x2e, 0x3c, 0x28, 0x2b, 0x7c,	/* .........`.<(+| */
/*50 */ 0x26, 0xe9, 0xea, 0xeb, 0xe8, 0xed, 0xee, 0xef,
  0xec, 0xdf, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0x9f,	/*&.........!$*);. */
/*60 */ 0x2d, 0x2f, 0xc2, 0xc4, 0xc0, 0xc1, 0xc3, 0xc5,
  0xc7, 0xd1, 0x5e, 0x2c, 0x25, 0x5f, 0x3e, 0x3f,
/*-/........^,%_>?*/
/*70 */ 0xf8, 0xc9, 0xca, 0xcb, 0xc8, 0xcd, 0xce, 0xcf,
  0xcc, 0xa8, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22,	/*..........:#@'=" */
/*80 */ 0xd8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69, 0xab, 0xbb, 0xf0, 0xfd, 0xfe, 0xb1,	/*.abcdefghi...... */
/*90 */ 0xb0, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
  0x71, 0x72, 0xaa, 0xba, 0xe6, 0xb8, 0xc6, 0xa4,	/*.jklmnopqr...... */
/*a0 */ 0xb5, 0xaf, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
  0x79, 0x7a, 0xa1, 0xbf, 0xd0, 0xdd, 0xde, 0xae,	/*..stuvwxyz...... */
/*b0 */ 0xa2, 0xa3, 0xa5, 0xb7, 0xa9, 0xa7, 0xb6, 0xbc,
  0xbd, 0xbe, 0xac, 0x5b, 0x5c, 0x5d, 0xb4, 0xd7,	/*...........[\].. */
/*c0 */ 0xf9, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x48, 0x49, 0xad, 0xf4, 0xf6, 0xf2, 0xf3, 0xf5,	/*.ABCDEFGHI...... */
/*d0 */ 0xa6, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
  0x51, 0x52, 0xb9, 0xfb, 0xfc, 0xdb, 0xfa, 0xff,	/*.JKLMNOPQR...... */
/*e0 */ 0xd9, 0xf7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
  0x59, 0x5a, 0xb2, 0xd4, 0xd6, 0xd2, 0xd3, 0xd5,	/*..STUVWXYZ...... */
/*f0 */ 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0xb3, 0x7b, 0xdc, 0x7d, 0xda, 0x7e	/*0123456789.{.}.~ */
};
#endif /*CHARSET_EBCDIC */

/* 2.0.10: cast instead of floor() yields 35% performance improvement. Thanks to John Buckman. */
#define floor_cast(exp) ((long) exp)

extern int gdCosT[];
extern int gdSinT[];

static void gdImageBrushApply(gdImagePtr im, int x, int y);
static void gdImageTileApply(gdImagePtr im, int x, int y);
static void gdImageAntiAliasedApply(gdImagePtr im, int x, int y);
static int gdLayerOverlay(int dst, int src);
static int gdAlphaOverlayColor(int src, int dst, int max);
int gdImageGetTrueColorPixel(gdImagePtr im, int x, int y);

void php_gd_error_ex(int type, const char *format, ...)
{
	va_list args;

	TSRMLS_FETCH();

	va_start(args, format);
	php_verror(NULL, "", type, format, args TSRMLS_CC);
	va_end(args);
}

void php_gd_error(const char *format, ...)
{
	va_list args;

	TSRMLS_FETCH();

	va_start(args, format);
	php_verror(NULL, "", E_WARNING, format, args TSRMLS_CC);
	va_end(args);
}

gdImagePtr gdImageCreate (int sx, int sy)
{
	int i;
	gdImagePtr im;
	im = (gdImage *) gdMalloc(sizeof(gdImage));
	memset(im, 0, sizeof(gdImage));
	/* Row-major ever since gd 1.3 */
	im->pixels = (unsigned char **) gdMalloc(sizeof(unsigned char *) * sy);
	im->AA_opacity = (unsigned char **) gdMalloc(sizeof(unsigned char *) * sy);
	im->polyInts = 0;
	im->polyAllocated = 0;
	im->brush = 0;
	im->tile = 0;
	im->style = 0;
	for (i = 0; i < sy; i++) {
		/* Row-major ever since gd 1.3 */
		im->pixels[i] = (unsigned char *) gdCalloc(sx, sizeof(unsigned char));
		im->AA_opacity[i] = (unsigned char *) gdCalloc(sx, sizeof(unsigned char));
	}
	im->sx = sx;
	im->sy = sy;
	im->colorsTotal = 0;
	im->transparent = (-1);
	im->interlace = 0;
	im->thick = 1;
	im->AA = 0;
	im->AA_polygon = 0;
	for (i = 0; i < gdMaxColors; i++) {
		im->open[i] = 1;
		im->red[i] = 0;
		im->green[i] = 0;
		im->blue[i] = 0;
	}
	im->trueColor = 0;
	im->tpixels = 0;
	im->cx1 = 0;
	im->cy1 = 0;
	im->cx2 = im->sx - 1;
	im->cy2 = im->sy - 1;
	return im;
}

gdImagePtr gdImageCreateTrueColor (int sx, int sy)
{
	int i;
	gdImagePtr im;
	im = (gdImage *) gdMalloc(sizeof(gdImage));
	memset(im, 0, sizeof(gdImage));
	im->tpixels = (int **) gdMalloc(sizeof(int *) * sy);
	im->AA_opacity = (unsigned char **) gdMalloc(sizeof(unsigned char *) * sy);
	im->polyInts = 0;
	im->polyAllocated = 0;
	im->brush = 0;
	im->tile = 0;
	im->style = 0;
	for (i = 0; i < sy; i++) {
		im->tpixels[i] = (int *) gdCalloc(sx, sizeof(int));
		im->AA_opacity[i] = (unsigned char *) gdCalloc(sx, sizeof(unsigned char));
	}
	im->sx = sx;
	im->sy = sy;
	im->transparent = (-1);
	im->interlace = 0;
	im->trueColor = 1;
	/* 2.0.2: alpha blending is now on by default, and saving of alpha is
	 * off by default. This allows font antialiasing to work as expected
	 * on the first try in JPEGs -- quite important -- and also allows
	 * for smaller PNGs when saving of alpha channel is not really
	 * desired, which it usually isn't!
	 */
	im->saveAlphaFlag = 0;
	im->alphaBlendingFlag = 1;
	im->thick = 1;
	im->AA = 0;
	im->AA_polygon = 0;
	im->cx1 = 0;
	im->cy1 = 0;
	im->cx2 = im->sx - 1;
	im->cy2 = im->sy - 1;
	return im;
}

void gdImageDestroy (gdImagePtr im)
{
	int i;
	if (im->pixels) {
		for (i = 0; i < im->sy; i++) {
			gdFree(im->pixels[i]);
		}
		gdFree(im->pixels);
	}
	if (im->tpixels) {
		for (i = 0; i < im->sy; i++) {
			gdFree(im->tpixels[i]);
		}
		gdFree(im->tpixels);
	}
	if (im->AA_opacity) {
		for (i = 0; i < im->sy; i++) {
			gdFree(im->AA_opacity[i]);
		}
		gdFree(im->AA_opacity);
	}
	if (im->polyInts) {
		gdFree(im->polyInts);
	}
	if (im->style) {
		gdFree(im->style);
	}
	gdFree(im);
}

int gdImageColorClosest (gdImagePtr im, int r, int g, int b)
{
	return gdImageColorClosestAlpha (im, r, g, b, gdAlphaOpaque);
}

int gdImageColorClosestAlpha (gdImagePtr im, int r, int g, int b, int a)
{
	int i;
	long rd, gd, bd, ad;
	int ct = (-1);
	int first = 1;
	long mindist = 0;

	if (im->trueColor) {
		return gdTrueColorAlpha(r, g, b, a);
	}
	for (i = 0; i < im->colorsTotal; i++) {
		long dist;
		if (im->open[i]) {
			continue;
		}
		rd = im->red[i] - r;
		gd = im->green[i] - g;
		bd = im->blue[i] - b;
		/* gd 2.02: whoops, was - b (thanks to David Marwood) */
		ad = im->alpha[i] - a;
		dist = rd * rd + gd * gd + bd * bd + ad * ad;
		if (first || (dist < mindist)) {
			mindist = dist;
			ct = i;
			first = 0;
		}
	}
	return ct;
}

/* This code is taken from http://www.acm.org/jgt/papers/SmithLyons96/hwb_rgb.html, an article
 * on colour conversion to/from RBG and HWB colour systems.
 * It has been modified to return the converted value as a * parameter.
 */

#define RETURN_HWB(h, w, b) {HWB->H = h; HWB->W = w; HWB->B = b; return HWB;}
#define RETURN_RGB(r, g, b) {RGB->R = r; RGB->G = g; RGB->B = b; return RGB;}
#define HWB_UNDEFINED -1
#define SETUP_RGB(s, r, g, b) {s.R = r/255.0f; s.G = g/255.0f; s.B = b/255.0f;}

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#define MIN3(a,b,c) ((a)<(b)?(MIN(a,c)):(MIN(b,c)))
#ifndef MAX
#define MAX(a,b) ((a)<(b)?(b):(a))
#endif
#define MAX3(a,b,c) ((a)<(b)?(MAX(b,c)):(MAX(a,c)))


/*
 * Theoretically, hue 0 (pure red) is identical to hue 6 in these transforms. Pure
 * red always maps to 6 in this implementation. Therefore UNDEFINED can be
 * defined as 0 in situations where only unsigned numbers are desired.
 */
typedef struct
{
	float R, G, B;
}
RGBType;
typedef struct
{
	float H, W, B;
}
HWBType;

static HWBType * RGB_to_HWB (RGBType RGB, HWBType * HWB)
{
	/*
	 * RGB are each on [0, 1]. W and B are returned on [0, 1] and H is
	 * returned on [0, 6]. Exception: H is returned UNDEFINED if W == 1 - B.
	 */

	float R = RGB.R, G = RGB.G, B = RGB.B, w, v, b, f;
	int i;

	w = MIN3 (R, G, B);
	v = MAX3 (R, G, B);
	b = 1 - v;
	if (v == w) {
		RETURN_HWB(HWB_UNDEFINED, w, b);
	}
	f = (R == w) ? G - B : ((G == w) ? B - R : R - G);
	i = (R == w) ? 3 : ((G == w) ? 5 : 1);

	RETURN_HWB(i - f / (v - w), w, b);
}

static float HWB_Diff (int r1, int g1, int b1, int r2, int g2, int b2)
{
	RGBType RGB1, RGB2;
	HWBType HWB1, HWB2;
	float diff;

	SETUP_RGB(RGB1, r1, g1, b1);
	SETUP_RGB(RGB2, r2, g2, b2);

	RGB_to_HWB(RGB1, &HWB1);
	RGB_to_HWB(RGB2, &HWB2);

	/*
	 * I made this bit up; it seems to produce OK results, and it is certainly
	 * more visually correct than the current RGB metric. (PJW)
	 */

	if ((HWB1.H == HWB_UNDEFINED) || (HWB2.H == HWB_UNDEFINED)) {
		diff = 0.0f;	/* Undefined hues always match... */
	} else {
		diff = fabsf(HWB1.H - HWB2.H);
		if (diff > 3.0f) {
			diff = 6.0f - diff;	/* Remember, it's a colour circle */
		}
	}

	diff = diff * diff + (HWB1.W - HWB2.W) * (HWB1.W - HWB2.W) + (HWB1.B - HWB2.B) * (HWB1.B - HWB2.B);

	return diff;
}


#if 0
/*
 * This is not actually used, but is here for completeness, in case someone wants to
 * use the HWB stuff for anything else...
 */
static RGBType * HWB_to_RGB (HWBType HWB, RGBType * RGB)
{
	/*
	 * H is given on [0, 6] or UNDEFINED. W and B are given on [0, 1].
	 * RGB are each returned on [0, 1].
	 */

	float h = HWB.H, w = HWB.W, b = HWB.B, v, n, f;
	int i;

	v = 1 - b;
	if (h == HWB_UNDEFINED) {
		RETURN_RGB(v, v, v);
	}
	i = floor(h);
	f = h - i;
	if (i & 1) {
		f = 1 - f; /* if i is odd */
	}
	n = w + f * (v - w);		/* linear interpolation between w and v */
	switch (i) {
		case 6:
		case 0:
			RETURN_RGB(v, n, w);
		case 1:
			RETURN_RGB(n, v, w);
		case 2:
			RETURN_RGB(w, v, n);
		case 3:
			RETURN_RGB(w, n, v);
		case 4:
			RETURN_RGB(n, w, v);
		case 5:
			RETURN_RGB(v, w, n);
	}

	return RGB;
}
#endif

int gdImageColorClosestHWB (gdImagePtr im, int r, int g, int b)
{
	int i;
	/* long rd, gd, bd; */
	int ct = (-1);
	int first = 1;
	float mindist = 0;
	if (im->trueColor) {
		return gdTrueColor(r, g, b);
	}
	for (i = 0; i < im->colorsTotal; i++) {
		float dist;
		if (im->open[i]) {
			continue;
		}
		dist = HWB_Diff(im->red[i], im->green[i], im->blue[i], r, g, b);
		if (first || (dist < mindist)) {
			mindist = dist;
			ct = i;
			first = 0;
		}
	}
	return ct;
}

int gdImageColorExact (gdImagePtr im, int r, int g, int b)
{
	return gdImageColorExactAlpha (im, r, g, b, gdAlphaOpaque);
}

int gdImageColorExactAlpha (gdImagePtr im, int r, int g, int b, int a)
{
	int i;
	if (im->trueColor) {
		return gdTrueColorAlpha(r, g, b, a);
	}
	for (i = 0; i < im->colorsTotal; i++) {
		if (im->open[i]) {
			continue;
		}
		if ((im->red[i] == r) && (im->green[i] == g) && (im->blue[i] == b) && (im->alpha[i] == a)) {
			return i;
		}
	}
	return -1;
}

int gdImageColorAllocate (gdImagePtr im, int r, int g, int b)
{
	return gdImageColorAllocateAlpha (im, r, g, b, gdAlphaOpaque);
}

int gdImageColorAllocateAlpha (gdImagePtr im, int r, int g, int b, int a)
{
	int i;
	int ct = (-1);
	if (im->trueColor) {
		return gdTrueColorAlpha(r, g, b, a);
	}
	for (i = 0; i < im->colorsTotal; i++) {
		if (im->open[i]) {
			ct = i;
			break;
		}
	}
	if (ct == (-1)) {
		ct = im->colorsTotal;
		if (ct == gdMaxColors) {
			return -1;
		}
		im->colorsTotal++;
	}
	im->red[ct] = r;
	im->green[ct] = g;
	im->blue[ct] = b;
	im->alpha[ct] = a;
	im->open[ct] = 0;

	return ct;
}

/*
 * gdImageColorResolve is an alternative for the code fragment:
 *
 *      if ((color=gdImageColorExact(im,R,G,B)) < 0)
 *        if ((color=gdImageColorAllocate(im,R,G,B)) < 0)
 *          color=gdImageColorClosest(im,R,G,B);
 *
 * in a single function.    Its advantage is that it is guaranteed to
 * return a color index in one search over the color table.
 */

int gdImageColorResolve (gdImagePtr im, int r, int g, int b)
{
	return gdImageColorResolveAlpha(im, r, g, b, gdAlphaOpaque);
}

int gdImageColorResolveAlpha (gdImagePtr im, int r, int g, int b, int a)
{
  int c;
  int ct = -1;
  int op = -1;
  long rd, gd, bd, ad, dist;
  long mindist = 4 * 255 * 255;	/* init to max poss dist */
  if (im->trueColor)
    {
      return gdTrueColorAlpha (r, g, b, a);
    }

  for (c = 0; c < im->colorsTotal; c++)
    {
      if (im->open[c])
	{
	  op = c;		/* Save open slot */
	  continue;		/* Color not in use */
	}
      if (c == im->transparent)
        {
          /* don't ever resolve to the color that has
           * been designated as the transparent color */
          continue;
	}
      rd = (long) (im->red[c] - r);
      gd = (long) (im->green[c] - g);
      bd = (long) (im->blue[c] - b);
      ad = (long) (im->alpha[c] - a);
      dist = rd * rd + gd * gd + bd * bd + ad * ad;
      if (dist < mindist)
	{
	  if (dist == 0)
	    {
	      return c;		/* Return exact match color */
	    }
	  mindist = dist;
	  ct = c;
	}
    }
  /* no exact match.  We now know closest, but first try to allocate exact */
  if (op == -1)
    {
      op = im->colorsTotal;
      if (op == gdMaxColors)
	{			/* No room for more colors */
	  return ct;		/* Return closest available color */
	}
      im->colorsTotal++;
    }
  im->red[op] = r;
  im->green[op] = g;
  im->blue[op] = b;
  im->alpha[op] = a;
  im->open[op] = 0;
  return op;			/* Return newly allocated color */
}

void gdImageColorDeallocate (gdImagePtr im, int color)
{
	if (im->trueColor) {
		return;
	}
	/* Mark it open. */
	im->open[color] = 1;
}

void gdImageColorTransparent (gdImagePtr im, int color)
{
	if (!im->trueColor) {
		if (im->transparent != -1) {
			im->alpha[im->transparent] = gdAlphaOpaque;
		}
		if (color > -1 && color<im->colorsTotal && color<=gdMaxColors) {
			im->alpha[color] = gdAlphaTransparent;
		} else {
			return;
		}
	}
	im->transparent = color;
}

void gdImagePaletteCopy (gdImagePtr to, gdImagePtr from)
{
	int i;
	int x, y, p;
	int xlate[256];
	if (to->trueColor || from->trueColor) {
		return;
	}

	for (i = 0; i < 256; i++) {
		xlate[i] = -1;
	}

	for (x = 0; x < to->sx; x++) {
		for (y = 0; y < to->sy; y++) {
			p = gdImageGetPixel(to, x, y);
			if (xlate[p] == -1) {
				/* This ought to use HWB, but we don't have an alpha-aware version of that yet. */
				xlate[p] = gdImageColorClosestAlpha (from, to->red[p], to->green[p], to->blue[p], to->alpha[p]);
			}
			gdImageSetPixel(to, x, y, xlate[p]);
		}
	}

	for (i = 0; i < from->colorsTotal; i++) {
		to->red[i] = from->red[i];
		to->blue[i] = from->blue[i];
		to->green[i] = from->green[i];
		to->alpha[i] = from->alpha[i];
		to->open[i] = 0;
	}

	for (i = from->colorsTotal; i < to->colorsTotal; i++) {
		to->open[i] = 1;
	}

	to->colorsTotal = from->colorsTotal;
}

/* 2.0.10: before the drawing routines, some code to clip points that are
 * outside the drawing window.  Nick Atty (nick@canalplan.org.uk)
 *
 * This is the Sutherland Hodgman Algorithm, as implemented by
 * Duvanenko, Robbins and Gyurcsik - SH(DRG) for short.  See Dr Dobb's
 * Journal, January 1996, pp107-110 and 116-117
 *
 * Given the end points of a line, and a bounding rectangle (which we
 * know to be from (0,0) to (SX,SY)), adjust the endpoints to be on
 * the edges of the rectangle if the line should be drawn at all,
 * otherwise return a failure code
 */

/* this does "one-dimensional" clipping: note that the second time it
 *  is called, all the x parameters refer to height and the y to width
 *  - the comments ignore this (if you can understand it when it's
 *  looking at the X parameters, it should become clear what happens on
 *  the second call!)  The code is simplified from that in the article,
 *  as we know that gd images always start at (0,0)
 */

static int clip_1d(int *x0, int *y0, int *x1, int *y1, int maxdim) {
	double m;      /* gradient of line */

	if (*x0 < 0) {  /* start of line is left of window */
		if(*x1 < 0) { /* as is the end, so the line never cuts the window */
			return 0;
		}
		m = (*y1 - *y0)/(double)(*x1 - *x0); /* calculate the slope of the line */
		/* adjust x0 to be on the left boundary (ie to be zero), and y0 to match */
		*y0 -= (int)(m * *x0);
		*x0 = 0;
		/* now, perhaps, adjust the far end of the line as well */
		if (*x1 > maxdim) {
			*y1 += (int)(m * (maxdim - *x1));
			*x1 = maxdim;
		}
		return 1;
	}
	if (*x0 > maxdim) { /* start of line is right of window - complement of above */
		if (*x1 > maxdim) { /* as is the end, so the line misses the window */
			return 0;
		}
		m = (*y1 - *y0)/(double)(*x1 - *x0);  /* calculate the slope of the line */
		*y0 += (int)(m * (maxdim - *x0)); /* adjust so point is on the right boundary */
		*x0 = maxdim;
		/* now, perhaps, adjust the end of the line */
		if (*x1 < 0) {
			*y1 -= (int)(m * *x1);
			*x1 = 0;
		}
		return 1;
	}
	/* the final case - the start of the line is inside the window */
	if (*x1 > maxdim) { /* other end is outside to the right */
		m = (*y1 - *y0)/(double)(*x1 - *x0);  /* calculate the slope of the line */
		*y1 += (int)(m * (maxdim - *x1));
		*x1 = maxdim;
		return 1;
	}
	if (*x1 < 0) { /* other end is outside to the left */
		m = (*y1 - *y0)/(double)(*x1 - *x0);  /* calculate the slope of the line */
		*y1 -= (int)(m * *x1);
		*x1 = 0;
		return 1;
	}
	/* only get here if both points are inside the window */
	return 1;
}

void gdImageSetPixel (gdImagePtr im, int x, int y, int color)
{
	int p;
	switch (color) {
		case gdStyled:
			if (!im->style) {
				/* Refuse to draw if no style is set. */
				return;
			} else {
				p = im->style[im->stylePos++];
			}
			if (p != gdTransparent) {
				gdImageSetPixel(im, x, y, p);
			}
			im->stylePos = im->stylePos % im->styleLength;
			break;
		case gdStyledBrushed:
			if (!im->style) {
				/* Refuse to draw if no style is set. */
				return;
			}
			p = im->style[im->stylePos++];
			if (p != gdTransparent && p != 0) {
				gdImageSetPixel(im, x, y, gdBrushed);
			}
			im->stylePos = im->stylePos % im->styleLength;
			break;
		case gdBrushed:
			gdImageBrushApply(im, x, y);
			break;
		case gdTiled:
			gdImageTileApply(im, x, y);
			break;
		case gdAntiAliased:
			gdImageAntiAliasedApply(im, x, y);
			break;
		default:
			if (gdImageBoundsSafe(im, x, y)) {
				if (im->trueColor) {
					switch (im->alphaBlendingFlag) {
						default:
						case gdEffectReplace:
							im->tpixels[y][x] = color;
							break;
						case gdEffectAlphaBlend:
							im->tpixels[y][x] = gdAlphaBlend(im->tpixels[y][x], color);
							break;
						case gdEffectNormal:
							im->tpixels[y][x] = gdAlphaBlend(im->tpixels[y][x], color);
							break;
						case gdEffectOverlay :
							im->tpixels[y][x] = gdLayerOverlay(im->tpixels[y][x], color);
							break;
					}
				} else {
					im->pixels[y][x] = color;
				}
			}
			break;
	}
}

int gdImageGetTrueColorPixel (gdImagePtr im, int x, int y)
{
	int p = gdImageGetPixel(im, x, y);

	if (!im->trueColor)  {
		return gdTrueColorAlpha(im->red[p], im->green[p], im->blue[p], (im->transparent == p) ? gdAlphaTransparent : gdAlphaOpaque);
	} else {
		return p;
	}
}

static void gdImageBrushApply (gdImagePtr im, int x, int y)
{
	int lx, ly;
	int hy, hx;
	int x1, y1, x2, y2;
	int srcx, srcy;

	if (!im->brush) {
		return;
	}

	hy = gdImageSY(im->brush) / 2;
	y1 = y - hy;
	y2 = y1 + gdImageSY(im->brush);
	hx = gdImageSX(im->brush) / 2;
	x1 = x - hx;
	x2 = x1 + gdImageSX(im->brush);
	srcy = 0;

	if (im->trueColor) {
		if (im->brush->trueColor) {
			for (ly = y1; ly < y2; ly++) {
				srcx = 0;
				for (lx = x1; (lx < x2); lx++) {
					int p;
					p = gdImageGetTrueColorPixel(im->brush, srcx, srcy);
					/* 2.0.9, Thomas Winzig: apply simple full transparency */
					if (p != gdImageGetTransparent(im->brush)) {
						gdImageSetPixel(im, lx, ly, p);
					}
					srcx++;
				}
				srcy++;
			}
		} else {
			/* 2.0.12: Brush palette, image truecolor (thanks to Thorben Kundinger for pointing out the issue) */
			for (ly = y1; ly < y2; ly++) {
				srcx = 0;
				for (lx = x1; lx < x2; lx++) {
					int p, tc;
					p = gdImageGetPixel(im->brush, srcx, srcy);
					tc = gdImageGetTrueColorPixel(im->brush, srcx, srcy);
					/* 2.0.9, Thomas Winzig: apply simple full transparency */
					if (p != gdImageGetTransparent(im->brush)) {
						gdImageSetPixel(im, lx, ly, tc);
					}
					srcx++;
				}
				srcy++;
			}
		}
	} else {
		for (ly = y1; ly < y2; ly++) {
			srcx = 0;
			for (lx = x1; lx < x2; lx++) {
				int p;
				p = gdImageGetPixel(im->brush, srcx, srcy);
				/* Allow for non-square brushes! */
				if (p != gdImageGetTransparent(im->brush)) {
					/* Truecolor brush. Very slow on a palette destination. */
					if (im->brush->trueColor) {
						gdImageSetPixel(im, lx, ly, gdImageColorResolveAlpha(im, gdTrueColorGetRed(p),
													 gdTrueColorGetGreen(p),
													 gdTrueColorGetBlue(p),
													 gdTrueColorGetAlpha(p)));
					} else {
						gdImageSetPixel(im, lx, ly, im->brushColorMap[p]);
					}
				}
				srcx++;
			}
			srcy++;
		}
	}
}

static void gdImageTileApply (gdImagePtr im, int x, int y)
{
	int srcx, srcy;
	int p;
	if (!im->tile) {
		return;
	}
	srcx = x % gdImageSX(im->tile);
	srcy = y % gdImageSY(im->tile);
	if (im->trueColor) {
		p = gdImageGetTrueColorPixel(im->tile, srcx, srcy);
		gdImageSetPixel(im, x, y, p);
	} else {
		p = gdImageGetPixel(im->tile, srcx, srcy);
		/* Allow for transparency */
		if (p != gdImageGetTransparent(im->tile)) {
			if (im->tile->trueColor) {
				/* Truecolor tile. Very slow on a palette destination. */
				gdImageSetPixel(im, x, y, gdImageColorResolveAlpha(im,
											gdTrueColorGetRed(p),
											gdTrueColorGetGreen(p),
											gdTrueColorGetBlue(p),
											gdTrueColorGetAlpha(p)));
			} else {
				gdImageSetPixel(im, x, y, im->tileColorMap[p]);
			}
		}
	}
}


static int gdImageTileGet (gdImagePtr im, int x, int y)
{
	int srcx, srcy;
	int tileColor,p;
	if (!im->tile) {
		return -1;
	}
	srcx = x % gdImageSX(im->tile);
	srcy = y % gdImageSY(im->tile);
	p = gdImageGetPixel(im->tile, srcx, srcy);

	if (im->trueColor) {
		if (im->tile->trueColor) {
			tileColor = p;
		} else {
			tileColor = gdTrueColorAlpha( gdImageRed(im->tile,p), gdImageGreen(im->tile,p), gdImageBlue (im->tile,p), gdImageAlpha (im->tile,p));
		}
	} else {
		if (im->tile->trueColor) {
			tileColor = gdImageColorResolveAlpha(im, gdTrueColorGetRed (p), gdTrueColorGetGreen (p), gdTrueColorGetBlue (p), gdTrueColorGetAlpha (p));
		} else {
			tileColor = p;
			tileColor = gdImageColorResolveAlpha(im, gdImageRed (im->tile,p), gdImageGreen (im->tile,p), gdImageBlue (im->tile,p), gdImageAlpha (im->tile,p));
		}
	}
	return tileColor;
}


static void gdImageAntiAliasedApply (gdImagePtr im, int px, int py)
{
	float p_dist, p_alpha;
	unsigned char opacity;

	/*
	 * Find the perpendicular distance from point C (px, py) to the line
	 * segment AB that is being drawn.  (Adapted from an algorithm from the
	 * comp.graphics.algorithms FAQ.)
	 */

	int LAC_2, LBC_2;

	int Ax_Cx = im->AAL_x1 - px;
	int Ay_Cy = im->AAL_y1 - py;

	int Bx_Cx = im->AAL_x2 - px;
	int By_Cy = im->AAL_y2 - py;

	/* 2.0.13: bounds check! AA_opacity is just as capable of
	 * overflowing as the main pixel array. Arne Jorgensen.
	 * 2.0.14: typo fixed. 2.0.15: moved down below declarations
	 * to satisfy non-C++ compilers.
	 */
	if (!gdImageBoundsSafe(im, px, py)) {
		return;
	}

	/* Get the squares of the lengths of the segemnts AC and BC. */
	LAC_2 = (Ax_Cx * Ax_Cx) + (Ay_Cy * Ay_Cy);
	LBC_2 = (Bx_Cx * Bx_Cx) + (By_Cy * By_Cy);

	if (((im->AAL_LAB_2 + LAC_2) >= LBC_2) && ((im->AAL_LAB_2 + LBC_2) >= LAC_2)) {
		/* The two angles are acute.  The point lies inside the portion of the
		 * plane spanned by the line segment.
		 */
		p_dist = fabs ((float) ((Ay_Cy * im->AAL_Bx_Ax) - (Ax_Cx * im->AAL_By_Ay)) / im->AAL_LAB);
	} else {
		/* The point is past an end of the line segment.  It's length from the
		 * segment is the shorter of the lengths from the endpoints, but call
		 * the distance -1, so as not to compute the alpha nor draw the pixel.
		 */
		p_dist = -1;
	}

	if ((p_dist >= 0) && (p_dist <= (float) (im->thick))) {
		p_alpha = pow (1.0 - (p_dist / 1.5), 2);

		if (p_alpha > 0) {
			if (p_alpha >= 1) {
				opacity = 255;
			} else {
				opacity = (unsigned char) (p_alpha * 255.0);
			}
			if (!im->AA_polygon || (im->AA_opacity[py][px] < opacity)) {
				im->AA_opacity[py][px] = opacity;
			}
		}
	}
}


int gdImageGetPixel (gdImagePtr im, int x, int y)
{
	if (gdImageBoundsSafe(im, x, y)) {
		if (im->trueColor) {
			return im->tpixels[y][x];
		} else {
			return im->pixels[y][x];
		}
	} else {
		return 0;
	}
}

void gdImageAABlend (gdImagePtr im)
{
	float p_alpha, old_alpha;
	int color = im->AA_color, color_red, color_green, color_blue;
	int old_color, old_red, old_green, old_blue;
	int p_color, p_red, p_green, p_blue;
	int px, py;

	color_red = gdImageRed(im, color);
	color_green = gdImageGreen(im, color);
	color_blue = gdImageBlue(im, color);

	/* Impose the anti-aliased drawing on the image. */
	for (py = 0; py < im->sy; py++) {
		for (px = 0; px < im->sx; px++) {
			if (im->AA_opacity[py][px] != 0) {
				old_color = gdImageGetPixel(im, px, py);

				if ((old_color != color) && ((old_color != im->AA_dont_blend) || (im->AA_opacity[py][px] == 255))) {
					/* Only blend with different colors that aren't the dont_blend color. */
					p_alpha = (float) (im->AA_opacity[py][px]) / 255.0;
					old_alpha = 1.0 - p_alpha;

					if (p_alpha >= 1.0) {
						p_color = color;
					} else {
						old_red = gdImageRed(im, old_color);
						old_green = gdImageGreen(im, old_color);
						old_blue = gdImageBlue(im, old_color);

						p_red = (int) (((float) color_red * p_alpha) + ((float) old_red * old_alpha));
						p_green = (int) (((float) color_green * p_alpha) + ((float) old_green * old_alpha));
						p_blue = (int) (((float) color_blue * p_alpha) + ((float) old_blue * old_alpha));
						p_color = gdImageColorResolve(im, p_red, p_green, p_blue);
					}
					gdImageSetPixel(im, px, py, p_color);
				}
			}
		}
		/* Clear the AA_opacity array behind us. */
		memset(im->AA_opacity[py], 0, im->sx);
	}
}


/* Bresenham as presented in Foley & Van Dam */
void gdImageLine (gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
	int dx, dy, incr1, incr2, d, x, y, xend, yend, xdirflag, ydirflag;
	int wid;
	int w, wstart;
	int thick = im->thick;

	/* 2.0.10: Nick Atty: clip to edges of drawing rectangle, return if no points need to be drawn */
	if (!clip_1d(&x1,&y1,&x2,&y2,gdImageSX(im)) || !clip_1d(&y1,&x1,&y2,&x2,gdImageSY(im))) {
		return;
	}

	/* gdAntiAliased passed as color: set anti-aliased line (AAL) global vars. */
	if (color == gdAntiAliased) {
		im->AAL_x1 = x1;
		im->AAL_y1 = y1;
		im->AAL_x2 = x2;
		im->AAL_y2 = y2;

		/* Compute what we can for point-to-line distance calculation later. */
		im->AAL_Bx_Ax = x2 - x1;
		im->AAL_By_Ay = y2 - y1;
		im->AAL_LAB_2 = (im->AAL_Bx_Ax * im->AAL_Bx_Ax) + (im->AAL_By_Ay * im->AAL_By_Ay);
		im->AAL_LAB = sqrt (im->AAL_LAB_2);

		/* For AA, we must draw pixels outside the width of the line.  Keep in
		 * mind that this will be curtailed by cos/sin of theta later.
		 */
		thick += 4;
	}

	dx = abs(x2 - x1);
	dy = abs(y2 - y1);
	if (dy <= dx) {
		/* More-or-less horizontal. use wid for vertical stroke */
		/* Doug Claar: watch out for NaN in atan2 (2.0.5) */
		if ((dx == 0) && (dy == 0)) {
			wid = 1;
		} else {
			wid = (int)(thick * cos (atan2 (dy, dx)));
			if (wid == 0) {
				wid = 1;
			}
		}
		d = 2 * dy - dx;
		incr1 = 2 * dy;
		incr2 = 2 * (dy - dx);
		if (x1 > x2) {
			x = x2;
			y = y2;
			ydirflag = (-1);
			xend = x1;
		} else {
			x = x1;
			y = y1;
			ydirflag = 1;
			xend = x2;
		}

		/* Set up line thickness */
		wstart = y - wid / 2;
		for (w = wstart; w < wstart + wid; w++) {
			gdImageSetPixel(im, x, w, color);
		}

		if (((y2 - y1) * ydirflag) > 0) {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += incr1;
				} else {
					y++;
					d += incr2;
				}
				wstart = y - wid / 2;
				for (w = wstart; w < wstart + wid; w++) {
					gdImageSetPixel (im, x, w, color);
				}
			}
		} else {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += incr1;
				} else {
					y--;
					d += incr2;
				}
				wstart = y - wid / 2;
				for (w = wstart; w < wstart + wid; w++) {
					gdImageSetPixel (im, x, w, color);
				}
			}
		}
	} else {
		/* More-or-less vertical. use wid for horizontal stroke */
		/* 2.0.12: Michael Schwartz: divide rather than multiply;
		   TBB: but watch out for /0! */
		double as = sin(atan2(dy, dx));
		if (as != 0) {
			if (!(wid = thick / as)) {
				wid = 1;
			}
		} else {
			wid = 1;
		}

		d = 2 * dx - dy;
		incr1 = 2 * dx;
		incr2 = 2 * (dx - dy);
		if (y1 > y2) {
			y = y2;
			x = x2;
			yend = y1;
			xdirflag = (-1);
		} else {
			y = y1;
			x = x1;
			yend = y2;
			xdirflag = 1;
		}

		/* Set up line thickness */
		wstart = x - wid / 2;
		for (w = wstart; w < wstart + wid; w++) {
			gdImageSetPixel (im, w, y, color);
		}

		if (((x2 - x1) * xdirflag) > 0) {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += incr1;
				} else {
					x++;
					d += incr2;
				}
				wstart = x - wid / 2;
				for (w = wstart; w < wstart + wid; w++) {
					gdImageSetPixel (im, w, y, color);
				}
			}
		} else {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += incr1;
				} else {
					x--;
					d += incr2;
				}
				wstart = x - wid / 2;
				for (w = wstart; w < wstart + wid; w++) {
					gdImageSetPixel (im, w, y, color);
				}
			}
		}
	}

	/* If this is the only line we are drawing, go ahead and blend. */
	if (color == gdAntiAliased && !im->AA_polygon) {
		gdImageAABlend(im);
	}
}


/*
 * Added on 2003/12 by Pierre-Alain Joye (pajoye@pearfr.org)
 * */
#define BLEND_COLOR(a, nc, c, cc) \
nc = (cc) + (((((c) - (cc)) * (a)) + ((((c) - (cc)) * (a)) >> 8) + 0x80) >> 8);

inline static void gdImageSetAAPixelColor(gdImagePtr im, int x, int y, int color, int t)
{
	int dr,dg,db,p,r,g,b;
	dr = gdTrueColorGetRed(color);
	dg = gdTrueColorGetGreen(color);
	db = gdTrueColorGetBlue(color);

	p = gdImageGetPixel(im,x,y);
	r = gdTrueColorGetRed(p);
	g = gdTrueColorGetGreen(p);
	b = gdTrueColorGetBlue(p);

	BLEND_COLOR(t, dr, r, dr);
	BLEND_COLOR(t, dg, g, dg);
	BLEND_COLOR(t, db, b, db);
	im->tpixels[y][x]=gdTrueColorAlpha(dr, dg, db,  gdAlphaOpaque);
}

/*
 * Added on 2003/12 by Pierre-Alain Joye (pajoye@pearfr.org)
 **/
void gdImageAALine (gdImagePtr im, int x1, int y1, int x2, int y2, int col)
{
	/* keep them as 32bits */
	long x, y, inc;
	long dx, dy,tmp;

	if (y1 < 0 && y2 < 0) {
		return;
	}
	if (y1 < 0) {
		x1 += (y1 * (x1 - x2)) / (y2 - y1);
		y1 = 0;
	}
	if (y2 < 0) {
		x2 += (y2 * (x1 - x2)) / (y2 - y1);
		y2 = 0;
	}

	/* bottom edge */
	if (y1 >= im->sy && y2 >= im->sy) {
		return;
	}
	if (y1 >= im->sy) {
		x1 -= ((im->sy - y1) * (x1 - x2)) / (y2 - y1);
		y1 = im->sy - 1;
	}
	if (y2 >= im->sy) {
		x2 -= ((im->sy - y2) * (x1 - x2)) / (y2 - y1);
		y2 = im->sy - 1;
	}

	/* left edge */
	if (x1 < 0 && x2 < 0) {
		return;
	}
	if (x1 < 0) {
		y1 += (x1 * (y1 - y2)) / (x2 - x1);
		x1 = 0;
	}
	if (x2 < 0) {
		y2 += (x2 * (y1 - y2)) / (x2 - x1);
		x2 = 0;
	}
	/* right edge */
	if (x1 >= im->sx && x2 >= im->sx) {
		return;
	}
	if (x1 >= im->sx) {
		y1 -= ((im->sx - x1) * (y1 - y2)) / (x2 - x1);
		x1 = im->sx - 1;
	}
	if (x2 >= im->sx) {
		y2 -= ((im->sx - x2) * (y1 - y2)) / (x2 - x1);
		x2 = im->sx - 1;
	}

	dx = x2 - x1;
	dy = y2 - y1;

	if (dx == 0 && dy == 0) {
		return;
	}
	if (abs(dx) > abs(dy)) {
		if (dx < 0) {
			tmp = x1;
			x1 = x2;
			x2 = tmp;
			tmp = y1;
			y1 = y2;
			y2 = tmp;
			dx = x2 - x1;
			dy = y2 - y1;
		}
		x = x1 << 16;
		y = y1 << 16;
		inc = (dy * 65536) / dx;
		while ((x >> 16) < x2) {
			gdImageSetAAPixelColor(im, x >> 16, y >> 16, col, (y >> 8) & 0xFF);
			if ((y >> 16) + 1 < im->sy) {
				gdImageSetAAPixelColor(im, x >> 16, (y >> 16) + 1,col, (~y >> 8) & 0xFF);
			}
			x += (1 << 16);
			y += inc;
		}
	} else {
		if (dy < 0) {
			tmp = x1;
			x1 = x2;
			x2 = tmp;
			tmp = y1;
			y1 = y2;
			y2 = tmp;
			dx = x2 - x1;
			dy = y2 - y1;
		}
		x = x1 << 16;
		y = y1 << 16;
		inc = (dx * 65536) / dy;
		while ((y>>16) < y2) {
			gdImageSetAAPixelColor(im, x >> 16, y >> 16, col, (x >> 8) & 0xFF);
			if ((x >> 16) + 1 < im->sx) {
				gdImageSetAAPixelColor(im, (x >> 16) + 1, (y >> 16),col, (~x >> 8) & 0xFF);
			}
			x += inc;
			y += (1<<16);
		}
	}
}

static void dashedSet (gdImagePtr im, int x, int y, int color, int *onP, int *dashStepP, int wid, int vert);

void gdImageDashedLine (gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
	int dx, dy, incr1, incr2, d, x, y, xend, yend, xdirflag, ydirflag;
	int dashStep = 0;
	int on = 1;
	int wid;
	int vert;
	int thick = im->thick;

	dx = abs(x2 - x1);
	dy = abs(y2 - y1);
	if (dy <= dx) {
		/* More-or-less horizontal. use wid for vertical stroke */
		/* 2.0.12: Michael Schwartz: divide rather than multiply;
		TBB: but watch out for /0! */
		double as = sin(atan2(dy, dx));
		if (as != 0) {
			wid = thick / as;
		} else {
			wid = 1;
		}
		wid = (int)(thick * sin(atan2(dy, dx)));
		vert = 1;

		d = 2 * dy - dx;
		incr1 = 2 * dy;
		incr2 = 2 * (dy - dx);
		if (x1 > x2) {
			x = x2;
			y = y2;
			ydirflag = (-1);
			xend = x1;
		} else {
			x = x1;
			y = y1;
			ydirflag = 1;
			xend = x2;
		}
		dashedSet(im, x, y, color, &on, &dashStep, wid, vert);
		if (((y2 - y1) * ydirflag) > 0) {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += incr1;
				} else {
					y++;
					d += incr2;
				}
				dashedSet(im, x, y, color, &on, &dashStep, wid, vert);
			}
		} else {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += incr1;
				} else {
					y--;
					d += incr2;
				}
				dashedSet(im, x, y, color, &on, &dashStep, wid, vert);
			}
		}
	} else {
		/* 2.0.12: Michael Schwartz: divide rather than multiply;
		TBB: but watch out for /0! */
		double as = sin (atan2 (dy, dx));
		if (as != 0) {
			wid = thick / as;
		} else {
			wid = 1;
		}
		vert = 0;

		d = 2 * dx - dy;
		incr1 = 2 * dx;
		incr2 = 2 * (dx - dy);
		if (y1 > y2) {
			y = y2;
			x = x2;
			yend = y1;
			xdirflag = (-1);
		} else {
			y = y1;
			x = x1;
			yend = y2;
			xdirflag = 1;
		}
		dashedSet(im, x, y, color, &on, &dashStep, wid, vert);
		if (((x2 - x1) * xdirflag) > 0) {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += incr1;
				} else {
					x++;
					d += incr2;
				}
				dashedSet(im, x, y, color, &on, &dashStep, wid, vert);
			}
		} else {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += incr1;
				} else {
					x--;
					d += incr2;
				}
				dashedSet(im, x, y, color, &on, &dashStep, wid, vert);
			}
		}
	}
}

static void dashedSet (gdImagePtr im, int x, int y, int color, int *onP, int *dashStepP, int wid, int vert)
{
	int dashStep = *dashStepP;
	int on = *onP;
	int w, wstart;

	dashStep++;
	if (dashStep == gdDashSize) {
		dashStep = 0;
		on = !on;
	}
	if (on) {
		if (vert) {
			wstart = y - wid / 2;
			for (w = wstart; w < wstart + wid; w++) {
				gdImageSetPixel(im, x, w, color);
			}
		} else {
			wstart = x - wid / 2;
			for (w = wstart; w < wstart + wid; w++) {
				gdImageSetPixel(im, w, y, color);
			}
		}
	}
	*dashStepP = dashStep;
	*onP = on;
}

void gdImageChar (gdImagePtr im, gdFontPtr f, int x, int y, int c, int color)
{
	int cx, cy;
	int px, py;
	int fline;
	cx = 0;
	cy = 0;
#ifdef CHARSET_EBCDIC
	c = ASC (c);
#endif /*CHARSET_EBCDIC */
	if ((c < f->offset) || (c >= (f->offset + f->nchars))) {
		return;
	}
	fline = (c - f->offset) * f->h * f->w;
	for (py = y; (py < (y + f->h)); py++) {
		for (px = x; (px < (x + f->w)); px++) {
			if (f->data[fline + cy * f->w + cx]) {
				gdImageSetPixel(im, px, py, color);
			}
			cx++;
		}
		cx = 0;
		cy++;
	}
}

void gdImageCharUp (gdImagePtr im, gdFontPtr f, int x, int y, int c, int color)
{
	int cx, cy;
	int px, py;
	int fline;
	cx = 0;
	cy = 0;
#ifdef CHARSET_EBCDIC
	c = ASC (c);
#endif /*CHARSET_EBCDIC */
	if ((c < f->offset) || (c >= (f->offset + f->nchars))) {
		return;
	}
	fline = (c - f->offset) * f->h * f->w;
	for (py = y; py > (y - f->w); py--) {
		for (px = x; px < (x + f->h); px++) {
			if (f->data[fline + cy * f->w + cx]) {
				gdImageSetPixel(im, px, py, color);
			}
			cy++;
		}
		cy = 0;
		cx++;
	}
}

void gdImageString (gdImagePtr im, gdFontPtr f, int x, int y, unsigned char *s, int color)
{
	int i;
	int l;
	l = strlen ((char *) s);
	for (i = 0; (i < l); i++) {
		gdImageChar(im, f, x, y, s[i], color);
		x += f->w;
	}
}

void gdImageStringUp (gdImagePtr im, gdFontPtr f, int x, int y, unsigned char *s, int color)
{
	int i;
	int l;
	l = strlen ((char *) s);
	for (i = 0; (i < l); i++) {
		gdImageCharUp(im, f, x, y, s[i], color);
		y -= f->w;
	}
}

static int strlen16 (unsigned short *s);

void gdImageString16 (gdImagePtr im, gdFontPtr f, int x, int y, unsigned short *s, int color)
{
	int i;
	int l;
	l = strlen16(s);
	for (i = 0; (i < l); i++) {
		gdImageChar(im, f, x, y, s[i], color);
		x += f->w;
	}
}

void gdImageStringUp16 (gdImagePtr im, gdFontPtr f, int x, int y, unsigned short *s, int color)
{
	int i;
	int l;
	l = strlen16(s);
	for (i = 0; i < l; i++) {
		gdImageCharUp(im, f, x, y, s[i], color);
		y -= f->w;
	}
}

static int strlen16 (unsigned short *s)
{
	int len = 0;
	while (*s) {
		s++;
		len++;
	}
	return len;
}

#ifndef HAVE_LSQRT
/* If you don't have a nice square root function for longs, you can use
   ** this hack
 */
long lsqrt (long n)
{
 	long result = (long) sqrt ((double) n);
	return result;
}
#endif

/* s and e are integers modulo 360 (degrees), with 0 degrees
   being the rightmost extreme and degrees changing clockwise.
   cx and cy are the center in pixels; w and h are the horizontal
   and vertical diameter in pixels. Nice interface, but slow.
   See gd_arc_f_buggy.c for a better version that doesn't
   seem to be bug-free yet. */

void gdImageArc (gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color)
{
	if ((s % 360) == (e % 360)) {
		gdImageEllipse(im, cx, cy, w, h, color);
	} else {
		gdImageFilledArc(im, cx, cy, w, h, s, e, color, gdNoFill);
	}
}

void gdImageFilledArc (gdImagePtr im, int cx, int cy, int w, int h, int s, int e, int color, int style)
{
	gdPoint pts[3];
	int i;
	int lx = 0, ly = 0;
	int fx = 0, fy = 0;

	while (s<0) {
		s += 360;
	}

	while (e < s) {
		e += 360;
	}

	for (i = s; i <= e; i++) {
		int x, y;
		x = ((long) gdCosT[i % 360] * (long) w / (2 * 1024)) + cx;
		y = ((long) gdSinT[i % 360] * (long) h / (2 * 1024)) + cy;
		if (i != s) {
			if (!(style & gdChord)) {
				if (style & gdNoFill) {
					gdImageLine(im, lx, ly, x, y, color);
				} else {
					/* This is expensive! */
					pts[0].x = lx;
					pts[0].y = ly;
					pts[1].x = x;
					pts[1].y = y;
					pts[2].x = cx;
					pts[2].y = cy;
					gdImageFilledPolygon(im, pts, 3, color);
				}
			}
		} else {
			fx = x;
			fy = y;
		}
		lx = x;
		ly = y;
	}
	if (style & gdChord) {
		if (style & gdNoFill) {
			if (style & gdEdged) {
				gdImageLine(im, cx, cy, lx, ly, color);
				gdImageLine(im, cx, cy, fx, fy, color);
			}
			gdImageLine(im, fx, fy, lx, ly, color);
		} else {
			pts[0].x = fx;
			pts[0].y = fy;
			pts[1].x = lx;
			pts[1].y = ly;
			pts[2].x = cx;
			pts[2].y = cy;
			gdImageFilledPolygon(im, pts, 3, color);
		}
	} else {
		if (style & gdNoFill) {
			if (style & gdEdged) {
				gdImageLine(im, cx, cy, lx, ly, color);
				gdImageLine(im, cx, cy, fx, fy, color);
			}
		}
	}
}


/**
 * Integer Ellipse functions (gdImageEllipse and gdImageFilledEllipse)
 * Function added by Pierre-Alain Joye 02/08/2003 (paj@pearfr.org)
 * See the ellipse function simplification for the equation
 * as well as the midpoint algorithm.
 */

void gdImageEllipse(gdImagePtr im, int mx, int my, int w, int h, int c)
{
	int x=0,mx1=0,mx2=0,my1=0,my2=0;
	long aq,bq,dx,dy,r,rx,ry,a,b;

	a=w>>1;
	b=h>>1;
	gdImageSetPixel(im,mx+a, my, c);
	gdImageSetPixel(im,mx-a, my, c);
	mx1 = mx-a;my1 = my;
	mx2 = mx+a;my2 = my;

	aq = a * a;
	bq = b * b;
	dx = aq << 1;
	dy = bq << 1;
	r  = a * bq;
	rx = r << 1;
	ry = 0;
	x = a;
	while (x > 0){
		if (r > 0) {
			my1++;my2--;
			ry +=dx;
			r  -=ry;
		}
		if (r <= 0){
			x--;
			mx1++;mx2--;
			rx -=dy;
			r  +=rx;
		}
		gdImageSetPixel(im,mx1, my1, c);
		gdImageSetPixel(im,mx1, my2, c);
		gdImageSetPixel(im,mx2, my1, c);
		gdImageSetPixel(im,mx2, my2, c);
	}
}

void gdImageFilledEllipse (gdImagePtr im, int mx, int my, int w, int h, int c)
{
	int x=0,mx1=0,mx2=0,my1=0,my2=0;
	long aq,bq,dx,dy,r,rx,ry,a,b;
	int i;
	int old_y1,old_y2;

	a=w>>1;
	b=h>>1;

	gdImageLine(im, mx-a, my, mx+a, my, c);

	mx1 = mx-a;my1 = my;
	mx2 = mx+a;my2 = my;

	aq = a * a;
	bq = b * b;
	dx = aq << 1;
	dy = bq << 1;
	r  = a * bq;
	rx = r << 1;
	ry = 0;
	x = a;
	old_y2=-2;
	old_y1=-2;
	while (x > 0){
		if (r > 0) {
			my1++;my2--;
			ry +=dx;
			r  -=ry;
		}
		if (r <= 0){
			x--;
			mx1++;mx2--;
			rx -=dy;
			r  +=rx;
		}
		if(old_y2!=my2){
			for(i=mx1;i<=mx2;i++){
				gdImageSetPixel(im,i,my1,c);
			}
		}
		if(old_y2!=my2){
			for(i=mx1;i<=mx2;i++){
				gdImageSetPixel(im,i,my2,c);
			}
		}
		old_y2 = my2;
		old_y1 = my1;
	}
}

void gdImageFillToBorder (gdImagePtr im, int x, int y, int border, int color)
{
	int lastBorder;
	/* Seek left */
	int leftLimit = -1, rightLimit;
	int i, restoreAlphaBleding=0;

	if (border < 0) {
		/* Refuse to fill to a non-solid border */
		return;
	}

	if (im->alphaBlendingFlag) {
		restoreAlphaBleding = 1;
		im->alphaBlendingFlag = 0;
	}

	if (x >= im->sx) {
		x = im->sx - 1;
	}
	if (y >= im->sy) {
		y = im->sy - 1;
	}

	for (i = x; i >= 0; i--) {
		if (gdImageGetPixel(im, i, y) == border) {
			break;
		}
		gdImageSetPixel(im, i, y, color);
		leftLimit = i;
	}
	if (leftLimit == -1) {
		if (restoreAlphaBleding) {
			im->alphaBlendingFlag = 1;
		}
		return;
	}
	/* Seek right */
	rightLimit = x;
	for (i = (x + 1); i < im->sx; i++) {
		if (gdImageGetPixel(im, i, y) == border) {
			break;
		}
		gdImageSetPixel(im, i, y, color);
		rightLimit = i;
	}
	/* Look at lines above and below and start paints */
	/* Above */
	if (y > 0) {
		lastBorder = 1;
		for (i = leftLimit; i <= rightLimit; i++) {
			int c = gdImageGetPixel(im, i, y - 1);
			if (lastBorder) {
				if ((c != border) && (c != color)) {
					gdImageFillToBorder(im, i, y - 1, border, color);
					lastBorder = 0;
				}
			} else if ((c == border) || (c == color)) {
				lastBorder = 1;
			}
		}
	}

	/* Below */
	if (y < ((im->sy) - 1)) {
		lastBorder = 1;
		for (i = leftLimit; i <= rightLimit; i++) {
			int c = gdImageGetPixel(im, i, y + 1);

			if (lastBorder) {
				if ((c != border) && (c != color)) {
					gdImageFillToBorder(im, i, y + 1, border, color);
					lastBorder = 0;
				}
			} else if ((c == border) || (c == color)) {
				lastBorder = 1;
			}
		}
	}
	if (restoreAlphaBleding) {
		im->alphaBlendingFlag = 1;
	}
}

/*
 * set the pixel at (x,y) and its 4-connected neighbors
 * with the same pixel value to the new pixel value nc (new color).
 * A 4-connected neighbor:  pixel above, below, left, or right of a pixel.
 * ideas from comp.graphics discussions.
 * For tiled fill, the use of a flag buffer is mandatory. As the tile image can
 * contain the same color as the color to fill. To do not bloat normal filling
 * code I added a 2nd private function.
 */

/* horizontal segment of scan line y */
struct seg {int y, xl, xr, dy;};

/* max depth of stack */
#define FILL_MAX 1200000
#define FILL_PUSH(Y, XL, XR, DY) \
    if (sp<stack+FILL_MAX*10 && Y+(DY)>=0 && Y+(DY)<wy2) \
    {sp->y = Y; sp->xl = XL; sp->xr = XR; sp->dy = DY; sp++;}

#define FILL_POP(Y, XL, XR, DY) \
    {sp--; Y = sp->y+(DY = sp->dy); XL = sp->xl; XR = sp->xr;}

void _gdImageFillTiled(gdImagePtr im, int x, int y, int nc);

void gdImageFill(gdImagePtr im, int x, int y, int nc)
{
	int l, x1, x2, dy;
	int oc;   /* old pixel value */
	int wx2,wy2;

	int alphablending_bak;

	/* stack of filled segments */
	/* struct seg stack[FILL_MAX],*sp = stack;; */
	struct seg *stack;
	struct seg *sp;

	alphablending_bak = im->alphaBlendingFlag;	
	im->alphaBlendingFlag = 0;

	if (nc==gdTiled){
		_gdImageFillTiled(im,x,y,nc);
		im->alphaBlendingFlag = alphablending_bak;
		return;
	}

	wx2=im->sx;wy2=im->sy;
	oc = gdImageGetPixel(im, x, y);
	if (oc==nc || x<0 || x>wx2 || y<0 || y>wy2) {
		im->alphaBlendingFlag = alphablending_bak;	
		return;
	}

	stack = (struct seg *)safe_emalloc(sizeof(struct seg), ((int)(im->sy*im->sx)/4), 1);
	sp = stack;

	/* required! */
	FILL_PUSH(y,x,x,1);
	/* seed segment (popped 1st) */
 	FILL_PUSH(y+1, x, x, -1);
	while (sp>stack) {
		FILL_POP(y, x1, x2, dy);

		for (x=x1; x>=0 && gdImageGetPixel(im,x, y)==oc; x--) {
			gdImageSetPixel(im,x, y, nc);
		}
		if (x>=x1) {
			goto skip;
		}
		l = x+1;

                /* leak on left? */
		if (l<x1) {
			FILL_PUSH(y, l, x1-1, -dy);
		}
		x = x1+1;
		do {
			for (; x<=wx2 && gdImageGetPixel(im,x, y)==oc; x++) {
				gdImageSetPixel(im, x, y, nc);
			}
			FILL_PUSH(y, l, x-1, dy);
			/* leak on right? */
			if (x>x2+1) {
				FILL_PUSH(y, x2+1, x-1, -dy);
			}
skip:			for (x++; x<=x2 && (gdImageGetPixel(im, x, y)!=oc); x++);

			l = x;
		} while (x<=x2);
	}
	efree(stack);
	im->alphaBlendingFlag = alphablending_bak;	
}

void _gdImageFillTiled(gdImagePtr im, int x, int y, int nc)
{
	int i,l, x1, x2, dy;
	int oc;   /* old pixel value */
	int tiled;
	int wx2,wy2;
	/* stack of filled segments */
	struct seg *stack;
	struct seg *sp;

	int **pts;
	if(!im->tile){
		return;
	}

	wx2=im->sx;wy2=im->sy;
	tiled = nc==gdTiled;

	nc =  gdImageTileGet(im,x,y);
	pts = (int **) ecalloc(sizeof(int *) * im->sy, sizeof(int));

	for (i=0; i<im->sy;i++) {
		pts[i] = (int *) ecalloc(im->sx, sizeof(int));
	}

	stack = (struct seg *)safe_emalloc(sizeof(struct seg), ((int)(im->sy*im->sx)/4), 1);
	sp = stack;

	oc = gdImageGetPixel(im, x, y);

	/* required! */
	FILL_PUSH(y,x,x,1);
	/* seed segment (popped 1st) */
 	FILL_PUSH(y+1, x, x, -1);
	while (sp>stack) {
		FILL_POP(y, x1, x2, dy);
		for (x=x1; x>=0 && (!pts[y][x] && gdImageGetPixel(im,x,y)==oc); x--) {
			if (pts[y][x]){
				/* we should never be here */
				break;
			}
			nc = gdImageTileGet(im,x,y);
			pts[y][x]=1;
			gdImageSetPixel(im,x, y, nc);
		}
		if (x>=x1) {
			goto skip;
		}
		l = x+1;

		/* leak on left? */
		if (l<x1) {
			FILL_PUSH(y, l, x1-1, -dy);
		}
		x = x1+1;
		do {
			for (; x<=wx2 && (!pts[y][x] && gdImageGetPixel(im,x, y)==oc) ; x++) {
				if (pts[y][x]){
					/* we should never be here */
					break;
				}
				nc = gdImageTileGet(im,x,y);
				pts[y][x]=1;
				gdImageSetPixel(im, x, y, nc);
			}
			FILL_PUSH(y, l, x-1, dy);
			/* leak on right? */
			if (x>x2+1) {
				FILL_PUSH(y, x2+1, x-1, -dy);
			}
skip:			for (x++; x<=x2 && (pts[y][x] || gdImageGetPixel(im,x, y)!=oc); x++);
			l = x;
		} while (x<=x2);
	}
	for (i=0; i<im->sy;i++) {
		efree(pts[i]);
	}
	efree(pts);
	efree(stack);
}



void gdImageRectangle (gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
	int x1h = x1, x1v = x1, y1h = y1, y1v = y1, x2h = x2, x2v = x2, y2h = y2, y2v = y2;
	int thick = im->thick;
	int half1 = 1;
	int t;

	if (y2 < y1) {
		t=y1;
		y1 = y2;
		y2 = t;

		t = x1;
		x1 = x2;
		x2 = t;
	}

	x1h = x1; x1v = x1; y1h = y1; y1v = y1; x2h = x2; x2v = x2; y2h = y2; y2v = y2;
	if (thick > 1) {
		int cx, cy, x1ul, y1ul, x2lr, y2lr;
		int half = thick >> 1;
		half1 = thick - half;
		x1ul = x1 - half;
		y1ul = y1 - half;
		
		x2lr = x2 + half;
		y2lr = y2 + half;

		cy = y1ul + thick;
		while (cy-- > y1ul) {
			cx = x1ul - 1;
			while (cx++ < x2lr) {
				gdImageSetPixel(im, cx, cy, color);
			}
		}

		cy = y2lr - thick;
		while (cy++ < y2lr) {
			cx = x1ul - 1;
			while (cx++ < x2lr) {
				gdImageSetPixel(im, cx, cy, color);
			}
		}

		cy = y1ul + thick - 1;
		while (cy++ < y2lr -thick) {
			cx = x1ul - 1;
			while (cx++ < x1ul + thick) {
				gdImageSetPixel(im, cx, cy, color);
			}
		}

		cy = y1ul + thick - 1;
		while (cy++ < y2lr -thick) {
			cx = x2lr - thick - 1;
			while (cx++ < x2lr) {
				gdImageSetPixel(im, cx, cy, color);
			}
		}

		return;
	} else {
		y1v = y1h + 1;
		y2v = y2h - 1;
		gdImageLine(im, x1h, y1h, x2h, y1h, color);
		gdImageLine(im, x1h, y2h, x2h, y2h, color);
		gdImageLine(im, x1v, y1v, x1v, y2v, color);
		gdImageLine(im, x2v, y1v, x2v, y2v, color);
	}
}

void gdImageFilledRectangle (gdImagePtr im, int x1, int y1, int x2, int y2, int color)
{
	int x, y;

	/* Nick Atty: limit the points at the edge.  Note that this also
	 * nicely kills any plotting for rectangles completely outside the
	 * window as it makes the tests in the for loops fail
	 */
	if (x1 < 0) {
		x1 = 0;
	}
	if (x1 > gdImageSX(im)) {
		x1 = gdImageSX(im);
	}
	if(y1 < 0) {
		y1 = 0;
	}
	if (y1 > gdImageSY(im)) {
		y1 = gdImageSY(im);
	}
	if (x1 > x2) {
		x = x1;
		x1 = x2;
		x2 = x;
	}
	if (y1 > y2) {
		y = y1;
		y1 = y2;
		y2 = y;
	}

	for (y = y1; (y <= y2); y++) {
		for (x = x1; (x <= x2); x++) {
			gdImageSetPixel (im, x, y, color);
		}
	}
}

void gdImageCopy (gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int w, int h)
{
	int c;
	int x, y;
	int tox, toy;
	int i;
	int colorMap[gdMaxColors];

	if (dst->trueColor) {
		/* 2.0: much easier when the destination is truecolor. */
		/* 2.0.10: needs a transparent-index check that is still valid if
		 * the source is not truecolor. Thanks to Frank Warmerdam.
		 */

		if (src->trueColor) {
			for (y = 0; (y < h); y++) {
				for (x = 0; (x < w); x++) {
					int c = gdImageGetTrueColorPixel (src, srcX + x, srcY + y);
					gdImageSetPixel (dst, dstX + x, dstY + y, c);
				}
			}
		} else {
			/* source is palette based */
			for (y = 0; (y < h); y++) {
				for (x = 0; (x < w); x++) {
					int c = gdImageGetPixel (src, srcX + x, srcY + y);
					if (c != src->transparent) {
						gdImageSetPixel(dst, dstX + x, dstY + y, gdTrueColorAlpha(src->red[c], src->green[c], src->blue[c], src->alpha[c]));
					}
				}
			}
		}
		return;
	}

	/* Destination is palette based */
	if (src->trueColor) { /* But source is truecolor (Ouch!) */
		toy = dstY;
		for (y = srcY; (y < (srcY + h)); y++) {
			tox = dstX;
			for (x = srcX; x < (srcX + w); x++) {
				int nc;
				c = gdImageGetPixel (src, x, y);

				/* Get best match possible. */
				nc = gdImageColorResolveAlpha(dst, gdTrueColorGetRed(c), gdTrueColorGetGreen(c), gdTrueColorGetBlue(c), gdTrueColorGetAlpha(c));

				gdImageSetPixel(dst, tox, toy, nc);
				tox++;
			}
			toy++;
		}
		return;
	}

	/* Palette based to palette based */
	for (i = 0; i < gdMaxColors; i++) {
		colorMap[i] = (-1);
	}
	toy = dstY;
	for (y = srcY; y < (srcY + h); y++) {
		tox = dstX;
		for (x = srcX; x < (srcX + w); x++) {
			int nc;
			int mapTo;
			c = gdImageGetPixel (src, x, y);
			/* Added 7/24/95: support transparent copies */
			if (gdImageGetTransparent (src) == c) {
				tox++;
				continue;
			}
			/* Have we established a mapping for this color? */
			if (src->trueColor) {
				/* 2.05: remap to the palette available in the destination image. This is slow and
				 * works badly, but it beats crashing! Thanks to Padhrig McCarthy.
				 */
				mapTo = gdImageColorResolveAlpha (dst, gdTrueColorGetRed (c), gdTrueColorGetGreen (c), gdTrueColorGetBlue (c), gdTrueColorGetAlpha (c));
			} else if (colorMap[c] == (-1)) {
				/* If it's the same image, mapping is trivial */
				if (dst == src) {
					nc = c;
				} else {
					/* Get best match possible. This function never returns error. */
					nc = gdImageColorResolveAlpha (dst, src->red[c], src->green[c], src->blue[c], src->alpha[c]);
				}
				colorMap[c] = nc;
				mapTo = colorMap[c];
			} else {
				mapTo = colorMap[c];
			}
			gdImageSetPixel (dst, tox, toy, mapTo);
			tox++;
		}
		toy++;
	}
}

/* This function is a substitute for real alpha channel operations,
   so it doesn't pay attention to the alpha channel. */
void gdImageCopyMerge (gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int w, int h, int pct)
{
	int c, dc;
	int x, y;
	int tox, toy;
	int ncR, ncG, ncB;
	toy = dstY;
	
	for (y = srcY; y < (srcY + h); y++) {
		tox = dstX;
		for (x = srcX; x < (srcX + w); x++) {
			int nc;
			c = gdImageGetPixel(src, x, y);
			/* Added 7/24/95: support transparent copies */
			if (gdImageGetTransparent(src) == c) {
				tox++;
				continue;
			}
			/* If it's the same image, mapping is trivial */
			if (dst == src) {
				nc = c;
			} else {
				dc = gdImageGetPixel(dst, tox, toy);

 				ncR = (int)(gdImageRed (src, c) * (pct / 100.0) + gdImageRed (dst, dc) * ((100 - pct) / 100.0));
 				ncG = (int)(gdImageGreen (src, c) * (pct / 100.0) + gdImageGreen (dst, dc) * ((100 - pct) / 100.0));
 				ncB = (int)(gdImageBlue (src, c) * (pct / 100.0) + gdImageBlue (dst, dc) * ((100 - pct) / 100.0));

				/* Find a reasonable color */
				nc = gdImageColorResolve (dst, ncR, ncG, ncB);
			}
			gdImageSetPixel (dst, tox, toy, nc);
			tox++;
		}
		toy++;
	}
}

/* This function is a substitute for real alpha channel operations,
   so it doesn't pay attention to the alpha channel. */
void gdImageCopyMergeGray (gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int w, int h, int pct)
{
	int c, dc;
	int x, y;
	int tox, toy;
	int ncR, ncG, ncB;
	float g;
	toy = dstY;

	for (y = srcY; (y < (srcY + h)); y++) {
		tox = dstX;
		for (x = srcX; (x < (srcX + w)); x++) {
			int nc;
			c = gdImageGetPixel (src, x, y);

			/* Added 7/24/95: support transparent copies */
			if (gdImageGetTransparent(src) == c) {
				tox++;
				continue;
			}

			/*
			 * If it's the same image, mapping is NOT trivial since we
			 * merge with greyscale target, but if pct is 100, the grey
			 * value is not used, so it becomes trivial. pjw 2.0.12.
			 */
			if (dst == src && pct == 100) {
				nc = c;
			} else {
				dc = gdImageGetPixel(dst, tox, toy);
				g = (0.29900f * gdImageRed(dst, dc)) + (0.58700f * gdImageGreen(dst, dc)) + (0.11400f * gdImageBlue(dst, dc));

				ncR = (int)(gdImageRed (src, c) * (pct / 100.0f) + g * ((100 - pct) / 100.0));
				ncG = (int)(gdImageGreen (src, c) * (pct / 100.0f) + g * ((100 - pct) / 100.0));
				ncB = (int)(gdImageBlue (src, c) * (pct / 100.0f) + g * ((100 - pct) / 100.0));


				/* First look for an exact match */
				nc = gdImageColorExact(dst, ncR, ncG, ncB);
				if (nc == (-1)) {
					/* No, so try to allocate it */
					nc = gdImageColorAllocate(dst, ncR, ncG, ncB);
					/* If we're out of colors, go for the closest color */
					if (nc == (-1)) {
						nc = gdImageColorClosest(dst, ncR, ncG, ncB);
					}
				}
			}
			gdImageSetPixel(dst, tox, toy, nc);
			tox++;
		}
		toy++;
	}
}

void gdImageCopyResized (gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int dstW, int dstH, int srcW, int srcH)
{
	int c;
	int x, y;
	int tox, toy;
	int ydest;
	int i;
	int colorMap[gdMaxColors];
	/* Stretch vectors */
	int *stx, *sty;
	/* We only need to use floating point to determine the correct stretch vector for one line's worth. */
	double accum;
	stx = (int *) gdMalloc (sizeof (int) * srcW);
	sty = (int *) gdMalloc (sizeof (int) * srcH);
	accum = 0;

	/* Fixed by Mao Morimoto 2.0.16 */
	for (i = 0; (i < srcW); i++) {
		stx[i] = dstW * (i+1) / srcW - dstW * i / srcW ;
	}
	for (i = 0; (i < srcH); i++) {
		sty[i] = dstH * (i+1) / srcH - dstH * i / srcH ;
	}
	for (i = 0; (i < gdMaxColors); i++) {
		colorMap[i] = (-1);
	}
	toy = dstY;
	for (y = srcY; (y < (srcY + srcH)); y++) {
		for (ydest = 0; (ydest < sty[y - srcY]); ydest++) {
			tox = dstX;
			for (x = srcX; (x < (srcX + srcW)); x++) {
				int nc = 0;
				int mapTo;
				if (!stx[x - srcX]) {
					continue;
				}
				if (dst->trueColor) {
					/* 2.0.9: Thorben Kundinger: Maybe the source image is not a truecolor image */
					if (!src->trueColor) {
					  	int tmp = gdImageGetPixel (src, x, y);
		  				mapTo = gdImageGetTrueColorPixel (src, x, y);
					  	if (gdImageGetTransparent (src) == tmp) {
							/* 2.0.21, TK: not tox++ */
							tox += stx[x - srcX];
					  		continue;
					  	}
					} else {
						/* TK: old code follows */
					  	mapTo = gdImageGetTrueColorPixel (src, x, y);
						/* Added 7/24/95: support transparent copies */
						if (gdImageGetTransparent (src) == mapTo) {
							/* 2.0.21, TK: not tox++ */
							tox += stx[x - srcX];
							continue;
						}
					}
				} else {
					c = gdImageGetPixel (src, x, y);
					/* Added 7/24/95: support transparent copies */
					if (gdImageGetTransparent (src) == c) {
					      tox += stx[x - srcX];
					      continue;
					}
					if (src->trueColor) {
					      /* Remap to the palette available in the destination image. This is slow and works badly. */
					      mapTo = gdImageColorResolveAlpha(dst, gdTrueColorGetRed(c),
					      					    gdTrueColorGetGreen(c),
					      					    gdTrueColorGetBlue(c),
					      					    gdTrueColorGetAlpha (c));
					} else {
						/* Have we established a mapping for this color? */
						if (colorMap[c] == (-1)) {
							/* If it's the same image, mapping is trivial */
							if (dst == src) {
								nc = c;
							} else {
								/* Find or create the best match */
								/* 2.0.5: can't use gdTrueColorGetRed, etc with palette */
								nc = gdImageColorResolveAlpha(dst, gdImageRed(src, c),
												   gdImageGreen(src, c),
												   gdImageBlue(src, c),
												   gdImageAlpha(src, c));
							}
							colorMap[c] = nc;
						}
						mapTo = colorMap[c];
					}
				}
				for (i = 0; (i < stx[x - srcX]); i++) {
					gdImageSetPixel (dst, tox, toy, mapTo);
					tox++;
				}
			}
			toy++;
		}
	}
	gdFree (stx);
	gdFree (sty);
}

/* When gd 1.x was first created, floating point was to be avoided.
   These days it is often faster than table lookups or integer
   arithmetic. The routine below is shamelessly, gloriously
   floating point. TBB */

void gdImageCopyResampled (gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX, int srcY, int dstW, int dstH, int srcW, int srcH)
{
	int x, y;
	double sy1, sy2, sx1, sx2;
	if (!dst->trueColor) {
		gdImageCopyResized (dst, src, dstX, dstY, srcX, srcY, dstW, dstH, srcW, srcH);
		return;
	}
	for (y = dstY; (y < dstY + dstH); y++) {
		sy1 = ((double) y - (double) dstY) * (double) srcH / (double) dstH;
		sy2 = ((double) (y + 1) - (double) dstY) * (double) srcH / (double) dstH;
		for (x = dstX; (x < dstX + dstW); x++) {
			double sx, sy;
			double spixels = 0;
			double red = 0.0, green = 0.0, blue = 0.0, alpha = 0.0;
			double alpha_factor, alpha_sum = 0.0, contrib_sum = 0.0;
			sx1 = ((double) x - (double) dstX) * (double) srcW / dstW;
			sx2 = ((double) (x + 1) - (double) dstX) * (double) srcW / dstW;
			sy = sy1;
			do {
				double yportion;
				if (floor_cast(sy) == floor_cast(sy1)) {
					yportion = 1.0f - (sy - floor_cast(sy));
					if (yportion > sy2 - sy1) {
						yportion = sy2 - sy1;
					}
					sy = floor_cast(sy);
				} else if (sy == floorf(sy2)) {
					yportion = sy2 - floor_cast(sy2);
				} else {
					yportion = 1.0f;
				}
				sx = sx1;
				do {
					double xportion;
					double pcontribution;
					int p;
					if (floorf(sx) == floor_cast(sx1)) {
						xportion = 1.0f - (sx - floor_cast(sx));
						if (xportion > sx2 - sx1) {
							xportion = sx2 - sx1;
						}
						sx = floor_cast(sx);
					} else if (sx == floorf(sx2)) {
						xportion = sx2 - floor_cast(sx2);
					} else {
						xportion = 1.0f;
					}
					pcontribution = xportion * yportion;
					p = gdImageGetTrueColorPixel(src, (int) sx + srcX, (int) sy + srcY);

					alpha_factor = ((gdAlphaMax - gdTrueColorGetAlpha(p))) * pcontribution;
					red += gdTrueColorGetRed (p) * alpha_factor;
					green += gdTrueColorGetGreen (p) * alpha_factor;
					blue += gdTrueColorGetBlue (p) * alpha_factor;
					alpha += gdTrueColorGetAlpha (p) * pcontribution;
					alpha_sum += alpha_factor;
					contrib_sum += pcontribution;
					spixels += xportion * yportion;
					sx += 1.0f;
				}
				while (sx < sx2);

				sy += 1.0f;
			}

			while (sy < sy2);

			if (spixels != 0.0f) {
				red /= spixels;
				green /= spixels;
				blue /= spixels;
				alpha /= spixels;
			}
			if ( alpha_sum != 0.0f) {
				if( contrib_sum != 0.0f) {
					alpha_sum /= contrib_sum;
				}
				red /= alpha_sum;
				green /= alpha_sum;
				blue /= alpha_sum;
			}
			/* Clamping to allow for rounding errors above */
			if (red > 255.0f) {
				red = 255.0f;
			}
			if (green > 255.0f) {
				green = 255.0f;
			}
			if (blue > 255.0f) {
				blue = 255.0f;
			}
			if (alpha > gdAlphaMax) {
				alpha = gdAlphaMax;
			}
			gdImageSetPixel(dst, x, y, gdTrueColorAlpha ((int) red, (int) green, (int) blue, (int) alpha));
		}
	}
}


/*
 * Rotate function Added on 2003/12
 * by Pierre-Alain Joye (pajoye@pearfr.org)
 **/
/* Begin rotate function */
#ifdef ROTATE_PI
#undef ROTATE_PI
#endif /* ROTATE_PI */

#define ROTATE_DEG2RAD  3.1415926535897932384626433832795/180
void gdImageSkewX (gdImagePtr dst, gdImagePtr src, int uRow, int iOffset, double dWeight, int clrBack, int ignoretransparent)
{
	typedef int (*FuncPtr)(gdImagePtr, int, int);
	int i, r, g, b, a, clrBackR, clrBackG, clrBackB, clrBackA;
	FuncPtr f;

	int pxlOldLeft, pxlLeft=0, pxlSrc;

	/* Keep clrBack as color index if required */
	if (src->trueColor) {
		pxlOldLeft = clrBack;
		f = gdImageGetTrueColorPixel;
	} else {
		pxlOldLeft = clrBack;
		clrBackR = gdImageRed(src, clrBack);
		clrBackG = gdImageGreen(src, clrBack);
		clrBackB = gdImageBlue(src, clrBack);
		clrBackA = gdImageAlpha(src, clrBack);
		clrBack =  gdTrueColorAlpha(clrBackR, clrBackG, clrBackB, clrBackA);
		f = gdImageGetPixel;
	}

	for (i = 0; i < iOffset; i++) {
		gdImageSetPixel (dst, i, uRow, clrBack);
	}

	if (i < dst->sx) {
		gdImageSetPixel (dst, i, uRow, clrBack);
	}

	for (i = 0; i < src->sx; i++) {
		pxlSrc = f (src,i,uRow);

		r = (int)(gdImageRed(src,pxlSrc) * dWeight);
		g = (int)(gdImageGreen(src,pxlSrc) * dWeight);
		b = (int)(gdImageBlue(src,pxlSrc) * dWeight);
		a = (int)(gdImageAlpha(src,pxlSrc) * dWeight);

		pxlLeft = gdImageColorAllocateAlpha(src, r, g, b, a);

		if (pxlLeft == -1) {
			pxlLeft = gdImageColorClosestAlpha(src, r, g, b, a);
		}

		r = gdImageRed(src,pxlSrc) - (gdImageRed(src,pxlLeft) - gdImageRed(src,pxlOldLeft));
		g = gdImageGreen(src,pxlSrc) - (gdImageGreen(src,pxlLeft) - gdImageGreen(src,pxlOldLeft));
		b = gdImageBlue(src,pxlSrc) - (gdImageBlue(src,pxlLeft) - gdImageBlue(src,pxlOldLeft));
		a = gdImageAlpha(src,pxlSrc) - (gdImageAlpha(src,pxlLeft) - gdImageAlpha(src,pxlOldLeft));

        if (r>255) {
        	r = 255;
        }

		if (g>255) {
			g = 255;
		}

		if (b>255) {
			b = 255;
		}

		if (a>127) {
			a = 127;
		}

		if (ignoretransparent && pxlSrc == dst->transparent) {
			pxlSrc = dst->transparent;
		} else {
			pxlSrc = gdImageColorAllocateAlpha(dst, r, g, b, a);

			if (pxlSrc == -1) {
				pxlSrc = gdImageColorClosestAlpha(dst, r, g, b, a);
			}
		}

		if ((i + iOffset >= 0) && (i + iOffset < dst->sx)) {
			gdImageSetPixel (dst, i+iOffset, uRow,  pxlSrc);
		}

		pxlOldLeft = pxlLeft;
	}

	i += iOffset;

	if (i < dst->sx) {
		gdImageSetPixel (dst, i, uRow, pxlLeft);
	}

	gdImageSetPixel (dst, iOffset, uRow, clrBack);

	i--;

	while (++i < dst->sx) {
		gdImageSetPixel (dst, i, uRow, clrBack);
	}
}

void gdImageSkewY (gdImagePtr dst, gdImagePtr src, int uCol, int iOffset, double dWeight, int clrBack, int ignoretransparent)
{
	typedef int (*FuncPtr)(gdImagePtr, int, int);
	int i, iYPos=0, r, g, b, a;
	FuncPtr f;
	int pxlOldLeft, pxlLeft=0, pxlSrc;

	if (src->trueColor) {
		f = gdImageGetTrueColorPixel;
	} else {
		f = gdImageGetPixel;
	}

	for (i = 0; i<=iOffset; i++) {
		gdImageSetPixel (dst, uCol, i, clrBack);
	}
	r = (int)((double)gdImageRed(src,clrBack) * dWeight);
	g = (int)((double)gdImageGreen(src,clrBack) * dWeight);
	b = (int)((double)gdImageBlue(src,clrBack) * dWeight);
	a = (int)((double)gdImageAlpha(src,clrBack) * dWeight);

	pxlOldLeft = gdImageColorAllocateAlpha(dst, r, g, b, a);

	for (i = 0; i < src->sy; i++) {
		pxlSrc = f (src, uCol, i);
		iYPos = i + iOffset;

		r = (int)((double)gdImageRed(src,pxlSrc) * dWeight);
		g = (int)((double)gdImageGreen(src,pxlSrc) * dWeight);
		b = (int)((double)gdImageBlue(src,pxlSrc) * dWeight);
		a = (int)((double)gdImageAlpha(src,pxlSrc) * dWeight);

		pxlLeft = gdImageColorAllocateAlpha(src, r, g, b, a);

		if (pxlLeft == -1) {
			pxlLeft = gdImageColorClosestAlpha(src, r, g, b, a);
		}

		r = gdImageRed(src,pxlSrc) - (gdImageRed(src,pxlLeft) - gdImageRed(src,pxlOldLeft));
		g = gdImageGreen(src,pxlSrc) - (gdImageGreen(src,pxlLeft) - gdImageGreen(src,pxlOldLeft));
		b = gdImageBlue(src,pxlSrc) - (gdImageBlue(src,pxlLeft) - gdImageBlue(src,pxlOldLeft));
		a = gdImageAlpha(src,pxlSrc) - (gdImageAlpha(src,pxlLeft) - gdImageAlpha(src,pxlOldLeft));

		if (r>255) {
        		r = 255;
		}

		if (g>255) {
			g = 255;
		}

		if (b>255) {
    			b = 255;
		}

		if (a>127) {
			a = 127;
		}

		if (ignoretransparent && pxlSrc == dst->transparent) {
			pxlSrc = dst->transparent;
		} else {
			pxlSrc = gdImageColorAllocateAlpha(dst, r, g, b, a);

			if (pxlSrc == -1) {
				pxlSrc = gdImageColorClosestAlpha(dst, r, g, b, a);
			}
		}

		if ((iYPos >= 0) && (iYPos < dst->sy)) {
			gdImageSetPixel (dst, uCol, iYPos, pxlSrc);
		}

		pxlOldLeft = pxlLeft;
	}

	i = iYPos;
	if (i < dst->sy) {
		gdImageSetPixel (dst, uCol, i, pxlLeft);
	}

	i--;
	while (++i < dst->sy) {
		gdImageSetPixel (dst, uCol, i, clrBack);
	}
}

/* Rotates an image by 90 degrees (counter clockwise) */
gdImagePtr gdImageRotate90 (gdImagePtr src, int ignoretransparent)
{
	int uY, uX;
	int c,r,g,b,a;
	gdImagePtr dst;
	typedef int (*FuncPtr)(gdImagePtr, int, int);
	FuncPtr f;

	if (src->trueColor) {
		f = gdImageGetTrueColorPixel;
	} else {
		f = gdImageGetPixel;
	}
	dst = gdImageCreateTrueColor(src->sy, src->sx);
	dst->transparent = src->transparent;

	if (dst != NULL) {
		gdImagePaletteCopy (dst, src);

		for (uY = 0; uY<src->sy; uY++) {
			for (uX = 0; uX<src->sx; uX++) {
				c = f (src, uX, uY);
				if (!src->trueColor) {
					r = gdImageRed(src,c);
					g = gdImageGreen(src,c);
					b = gdImageBlue(src,c);
					a = gdImageAlpha(src,c);
					c = gdTrueColorAlpha(r, g, b, a);
				}
				if (ignoretransparent && c == dst->transparent) {
					gdImageSetPixel(dst, uY, (dst->sy - uX - 1), dst->transparent);
				} else {
					gdImageSetPixel(dst, uY, (dst->sy - uX - 1), c);
				}
			}
		}
	}

	return dst;
}

/* Rotates an image by 180 degrees (counter clockwise) */
gdImagePtr gdImageRotate180 (gdImagePtr src, int ignoretransparent)
{
	int uY, uX;
	int c,r,g,b,a;
	gdImagePtr dst;
	typedef int (*FuncPtr)(gdImagePtr, int, int);
	FuncPtr f;

	if (src->trueColor) {
		f = gdImageGetTrueColorPixel;
	} else {
		f = gdImageGetPixel;
	}
	dst = gdImageCreateTrueColor(src->sx, src->sy);
	dst->transparent = src->transparent;

	if (dst != NULL) {
		gdImagePaletteCopy (dst, src);

		for (uY = 0; uY<src->sy; uY++) {
			for (uX = 0; uX<src->sx; uX++) {
				c = f (src, uX, uY);
				if (!src->trueColor) {
					r = gdImageRed(src,c);
					g = gdImageGreen(src,c);
					b = gdImageBlue(src,c);
					a = gdImageAlpha(src,c);
					c = gdTrueColorAlpha(r, g, b, a);
				}

				if (ignoretransparent && c == dst->transparent) {
					gdImageSetPixel(dst, (dst->sx - uX - 1), (dst->sy - uY - 1), dst->transparent);
				} else {
					gdImageSetPixel(dst, (dst->sx - uX - 1), (dst->sy - uY - 1), c);
				}
			}
		}
	}

	return dst;
}

/* Rotates an image by 270 degrees (counter clockwise) */
gdImagePtr gdImageRotate270 (gdImagePtr src, int ignoretransparent)
{
	int uY, uX;
	int c,r,g,b,a;
	gdImagePtr dst;
	typedef int (*FuncPtr)(gdImagePtr, int, int);
	FuncPtr f;

	if (src->trueColor) {
		f = gdImageGetTrueColorPixel;
	} else {
		f = gdImageGetPixel;
	}
	dst = gdImageCreateTrueColor (src->sy, src->sx);
	dst->transparent = src->transparent;

	if (dst != NULL) {
		gdImagePaletteCopy (dst, src);

		for (uY = 0; uY<src->sy; uY++) {
			for (uX = 0; uX<src->sx; uX++) {
				c = f (src, uX, uY);
				if (!src->trueColor) {
					r = gdImageRed(src,c);
					g = gdImageGreen(src,c);
					b = gdImageBlue(src,c);
					a = gdImageAlpha(src,c);
					c = gdTrueColorAlpha(r, g, b, a);
				}

				if (ignoretransparent && c == dst->transparent) {
					gdImageSetPixel(dst, (dst->sx - uY - 1), uX, dst->transparent);
				} else {
					gdImageSetPixel(dst, (dst->sx - uY - 1), uX, c);
				}
			}
		}
	}

	return dst;
}

gdImagePtr gdImageRotate45 (gdImagePtr src, double dAngle, int clrBack, int ignoretransparent)
{
	typedef int (*FuncPtr)(gdImagePtr, int, int);
	gdImagePtr dst1,dst2,dst3;
	FuncPtr f;
	double dRadAngle, dSinE, dTan, dShear;
	double dOffset;     /* Variable skew offset */
	int u, iShear, newx, newy;
	int clrBackR, clrBackG, clrBackB, clrBackA;

	/* See GEMS I for the algorithm details */
	dRadAngle = dAngle * ROTATE_DEG2RAD; /* Angle in radians */
	dSinE = sin (dRadAngle);
	dTan = tan (dRadAngle / 2.0);

	newx = (int)(src->sx + src->sy * fabs(dTan));
	newy = src->sy;

	/* 1st shear */
	if (src->trueColor) {
		f = gdImageGetTrueColorPixel;
	} else {
		f = gdImageGetPixel;
	}

	dst1 = gdImageCreateTrueColor(newx, newy);
	/******* Perform 1st shear (horizontal) ******/
	if (dst1 == NULL) {
		return NULL;
	}
	dst1->alphaBlendingFlag = gdEffectReplace;

	if (dAngle == 0.0) {
		/* Returns copy of src */
		gdImageCopy (dst1, src,0,0,0,0,src->sx,src->sy);
		return dst1;
	}

	gdImagePaletteCopy (dst1, src);

	if (ignoretransparent) {
		if (gdImageTrueColor(src)) {
			dst1->transparent = src->transparent;
		} else {

			dst1->transparent = gdTrueColorAlpha(gdImageRed(src, src->transparent), gdImageBlue(src, src->transparent), gdImageGreen(src, src->transparent), 127);
		}
	}

	dRadAngle = dAngle * ROTATE_DEG2RAD; /* Angle in radians */
	dSinE = sin (dRadAngle);
	dTan = tan (dRadAngle / 2.0);

	for (u = 0; u < dst1->sy; u++) {
		if (dTan >= 0.0) {
			dShear = ((double)(u + 0.5)) * dTan;
		} else {
			dShear = ((double)(u - dst1->sy) + 0.5) * dTan;
		}

		iShear = (int)floor(dShear);
		gdImageSkewX(dst1, src, u, iShear, (dShear - iShear), clrBack, ignoretransparent);
	}

	/*
	The 1st shear may use the original clrBack as color index
	Convert it once here
	*/
	if(!src->trueColor) {
		clrBackR = gdImageRed(src, clrBack);
		clrBackG = gdImageGreen(src, clrBack);
		clrBackB = gdImageBlue(src, clrBack);
		clrBackA = gdImageAlpha(src, clrBack);
		clrBack =  gdTrueColorAlpha(clrBackR, clrBackG, clrBackB, clrBackA);
	}
	/* 2nd shear */
	newx = dst1->sx;

	if (dSinE > 0.0) {
		dOffset = (src->sx-1) * dSinE;
	} else {
		dOffset = -dSinE *  (src->sx - newx);
	}

	newy = (int) ((double) src->sx * fabs( dSinE ) + (double) src->sy * cos (dRadAngle))+1;

	if (src->trueColor) {
		f = gdImageGetTrueColorPixel;
	} else {
		f = gdImageGetPixel;
	}
	dst2 = gdImageCreateTrueColor(newx, newy);
	if (dst2 == NULL) {
		gdImageDestroy(dst1);
		return NULL;
	}
	dst2->alphaBlendingFlag = gdEffectReplace;
	if (ignoretransparent) {
		dst2->transparent = dst1->transparent;
	}

	for (u = 0; u < dst2->sx; u++, dOffset -= dSinE) {
		iShear = (int)floor (dOffset);
		gdImageSkewY(dst2, dst1, u, iShear, (dOffset - (double)iShear), clrBack, ignoretransparent);
	}

	/* 3rd shear */
	gdImageDestroy(dst1);

	newx = (int) ((double)src->sy * fabs (dSinE) + (double)src->sx * cos (dRadAngle)) + 1;
	newy = dst2->sy;

	if (src->trueColor) {
		f = gdImageGetTrueColorPixel;
	} else {
		f = gdImageGetPixel;
	}
	dst3 = gdImageCreateTrueColor(newx, newy);
	if (dst3 == NULL) {
		gdImageDestroy(dst2);
		return NULL;
	}

	dst3->alphaBlendingFlag = gdEffectReplace;
	if (ignoretransparent) {
		dst3->transparent = dst2->transparent;
	}

	if (dSinE >= 0.0) {
		dOffset = (double)(src->sx - 1) * dSinE * -dTan;
	} else {
		dOffset = dTan * ((double)(src->sx - 1) * -dSinE + (double)(1 - newy));
	}

	for (u = 0; u < dst3->sy; u++, dOffset += dTan) {
		int iShear = (int)floor(dOffset);
		gdImageSkewX(dst3, dst2, u, iShear, (dOffset - iShear), clrBack, ignoretransparent);
	}

	gdImageDestroy(dst2);

	return dst3;
}

gdImagePtr gdImageRotate (gdImagePtr src, double dAngle, int clrBack, int ignoretransparent)
{
	gdImagePtr pMidImg;
	gdImagePtr rotatedImg;

	if (src == NULL) {
		return NULL;
	}

	if (!gdImageTrueColor(src) && clrBack>=gdImageColorsTotal(src)) {
		return NULL;
	}

	while (dAngle >= 360.0) {
		dAngle -= 360.0;
	}

	while (dAngle < 0) {
		dAngle += 360.0;
	}

	if (dAngle == 90.00) {
		return gdImageRotate90(src, ignoretransparent);
	}
	if (dAngle == 180.00) {
		return gdImageRotate180(src, ignoretransparent);
	}
	if(dAngle == 270.00) {
		return gdImageRotate270 (src, ignoretransparent);
	}

	if ((dAngle > 45.0) && (dAngle <= 135.0)) {
		pMidImg = gdImageRotate90 (src, ignoretransparent);
		dAngle -= 90.0;
	} else if ((dAngle > 135.0) && (dAngle <= 225.0)) {
		pMidImg = gdImageRotate180 (src, ignoretransparent);
		dAngle -= 180.0;
	} else if ((dAngle > 225.0) && (dAngle <= 315.0)) {
		pMidImg = gdImageRotate270 (src, ignoretransparent);
		dAngle -= 270.0;
	} else {
		return gdImageRotate45 (src, dAngle, clrBack, ignoretransparent);
	}

	if (pMidImg == NULL) {
		return NULL;
	}

	rotatedImg = gdImageRotate45 (pMidImg, dAngle, clrBack, ignoretransparent);
	gdImageDestroy(pMidImg);

	return rotatedImg;
}
/* End Rotate function */

void gdImagePolygon (gdImagePtr im, gdPointPtr p, int n, int c)
{
	int i;
	int lx, ly;
	typedef void (*image_line)(gdImagePtr im, int x1, int y1, int x2, int y2, int color);
	image_line draw_line;

	if (!n) {
		return;
	}

	/* Let it be known that we are drawing a polygon so that the opacity
	 * mask doesn't get cleared after each line.
	 */
	if (c == gdAntiAliased) {
		im->AA_polygon = 1;
	}

	if ( im->antialias) {
		draw_line = gdImageAALine;
	} else {
		draw_line = gdImageLine;
	}
	lx = p->x;
	ly = p->y;
	draw_line(im, lx, ly, p[n - 1].x, p[n - 1].y, c);
	for (i = 1; i < n; i++) {
		p++;
		draw_line(im, lx, ly, p->x, p->y, c);
		lx = p->x;
		ly = p->y;
	}

	if (c == gdAntiAliased) {
		im->AA_polygon = 0;
		gdImageAABlend(im);
	}
}

int gdCompareInt (const void *a, const void *b);

/* THANKS to Kirsten Schulz for the polygon fixes! */

/* The intersection finding technique of this code could be improved
 * by remembering the previous intertersection, and by using the slope.
 * That could help to adjust intersections  to produce a nice
 * interior_extrema.
 */

void gdImageFilledPolygon (gdImagePtr im, gdPointPtr p, int n, int c)
{
	int i;
	int y;
	int miny, maxy;
	int x1, y1;
	int x2, y2;
	int ind1, ind2;
	int ints;
	int fill_color;

	if (!n) {
		return;
	}

	if (c == gdAntiAliased) {
		fill_color = im->AA_color;
	} else {
		fill_color = c;
	}

	if (!im->polyAllocated) {
		im->polyInts = (int *) gdMalloc(sizeof(int) * n);
		im->polyAllocated = n;
	}
	if (im->polyAllocated < n) {
		while (im->polyAllocated < n) {
			im->polyAllocated *= 2;
		}
		im->polyInts = (int *) gdRealloc(im->polyInts, sizeof(int) * im->polyAllocated);
	}
	miny = p[0].y;
	maxy = p[0].y;
	for (i = 1; i < n; i++) {
		if (p[i].y < miny) {
			miny = p[i].y;
		}
		if (p[i].y > maxy) {
			maxy = p[i].y;
		}
	}

	/* 2.0.16: Optimization by Ilia Chipitsine -- don't waste time offscreen */
	if (miny < 0) {
		miny = 0;
	}
	if (maxy >= gdImageSY(im)) {
		maxy = gdImageSY(im) - 1;
	}

	/* Fix in 1.3: count a vertex only once */
	for (y = miny; y <= maxy; y++) {
		/*1.4           int interLast = 0; */
		/*              int dirLast = 0; */
		/*              int interFirst = 1; */
		ints = 0;
		for (i = 0; i < n; i++) {
			if (!i) {
				ind1 = n - 1;
				ind2 = 0;
			} else {
				ind1 = i - 1;
				ind2 = i;
			}
			y1 = p[ind1].y;
			y2 = p[ind2].y;
			if (y1 < y2) {
				x1 = p[ind1].x;
				x2 = p[ind2].x;
			} else if (y1 > y2) {
				y2 = p[ind1].y;
				y1 = p[ind2].y;
				x2 = p[ind1].x;
				x1 = p[ind2].x;
			} else {
				continue;
			}
			/* Do the following math as float intermediately, and round to ensure
			 * that Polygon and FilledPolygon for the same set of points have the
			 * same footprint.
			 */
			if (y >= y1 && y < y2) {
				im->polyInts[ints++] = (float) ((y - y1) * (x2 - x1)) / (float) (y2 - y1) + 0.5 + x1;
			} else if (y == maxy && y > y1 && y <= y2) {
				im->polyInts[ints++] = (float) ((y - y1) * (x2 - x1)) / (float) (y2 - y1) + 0.5 + x1;
			}
		}
		qsort(im->polyInts, ints, sizeof(int), gdCompareInt);

		for (i = 0; i < ints; i += 2) {
			gdImageLine(im, im->polyInts[i], y, im->polyInts[i + 1], y, fill_color);
		}
	}

	/* If we are drawing this AA, then redraw the border with AA lines. */
	if (c == gdAntiAliased) {
		gdImagePolygon(im, p, n, c);
	}
}

int gdCompareInt (const void *a, const void *b)
{
	return (*(const int *) a) - (*(const int *) b);
}

void gdImageSetStyle (gdImagePtr im, int *style, int noOfPixels)
{
	if (im->style) {
		gdFree(im->style);
	}
	im->style = (int *) gdMalloc(sizeof(int) * noOfPixels);
	memcpy(im->style, style, sizeof(int) * noOfPixels);
	im->styleLength = noOfPixels;
	im->stylePos = 0;
}

void gdImageSetThickness (gdImagePtr im, int thickness)
{
	im->thick = thickness;
}

void gdImageSetBrush (gdImagePtr im, gdImagePtr brush)
{
	int i;
	im->brush = brush;
	if (!im->trueColor && !im->brush->trueColor) {
		for (i = 0; i < gdImageColorsTotal(brush); i++) {
			int index;
			index = gdImageColorResolveAlpha(im, gdImageRed(brush, i), gdImageGreen(brush, i), gdImageBlue(brush, i), gdImageAlpha(brush, i));
			im->brushColorMap[i] = index;
		}
	}
}

void gdImageSetTile (gdImagePtr im, gdImagePtr tile)
{
	int i;
	im->tile = tile;
	if (!im->trueColor && !im->tile->trueColor) {
		for (i = 0; i < gdImageColorsTotal(tile); i++) {
			int index;
			index = gdImageColorResolveAlpha(im, gdImageRed(tile, i), gdImageGreen(tile, i), gdImageBlue(tile, i), gdImageAlpha(tile, i));
			im->tileColorMap[i] = index;
		}
	}
}

void gdImageSetAntiAliased (gdImagePtr im, int c)
{
	im->AA = 1;
	im->AA_color = c;
	im->AA_dont_blend = -1;
}

void gdImageSetAntiAliasedDontBlend (gdImagePtr im, int c, int dont_blend)
{
	im->AA = 1;
	im->AA_color = c;
	im->AA_dont_blend = dont_blend;
}


void gdImageInterlace (gdImagePtr im, int interlaceArg)
{
	im->interlace = interlaceArg;
}

int gdImageCompare (gdImagePtr im1, gdImagePtr im2)
{
	int x, y;
	int p1, p2;
	int cmpStatus = 0;
	int sx, sy;

	if (im1->interlace != im2->interlace) {
		cmpStatus |= GD_CMP_INTERLACE;
	}

	if (im1->transparent != im2->transparent) {
		cmpStatus |= GD_CMP_TRANSPARENT;
	}

	if (im1->trueColor != im2->trueColor) {
		cmpStatus |= GD_CMP_TRUECOLOR;
	}

	sx = im1->sx;
	if (im1->sx != im2->sx) {
		cmpStatus |= GD_CMP_SIZE_X + GD_CMP_IMAGE;
		if (im2->sx < im1->sx) {
			sx = im2->sx;
		}
	}

	sy = im1->sy;
	if (im1->sy != im2->sy) {
		cmpStatus |= GD_CMP_SIZE_Y + GD_CMP_IMAGE;
		if (im2->sy < im1->sy) {
			sy = im2->sy;
		}
	}

	if (im1->colorsTotal != im2->colorsTotal) {
		cmpStatus |= GD_CMP_NUM_COLORS;
	}

	for (y = 0; y < sy; y++) {
		for (x = 0; x < sx; x++) {
			p1 = im1->trueColor ? gdImageTrueColorPixel(im1, x, y) : gdImagePalettePixel(im1, x, y);
			p2 = im2->trueColor ? gdImageTrueColorPixel(im2, x, y) : gdImagePalettePixel(im2, x, y);

			if (gdImageRed(im1, p1) != gdImageRed(im2, p2)) {
				cmpStatus |= GD_CMP_COLOR + GD_CMP_IMAGE;
				break;
			}
			if (gdImageGreen(im1, p1) != gdImageGreen(im2, p2)) {
				cmpStatus |= GD_CMP_COLOR + GD_CMP_IMAGE;
				break;
			}
			if (gdImageBlue(im1, p1) != gdImageBlue(im2, p2)) {
				cmpStatus |= GD_CMP_COLOR + GD_CMP_IMAGE;
				break;
			}
#if 0
			/* Soon we'll add alpha channel to palettes */
			if (gdImageAlpha(im1, p1) != gdImageAlpha(im2, p2)) {
				cmpStatus |= GD_CMP_COLOR + GD_CMP_IMAGE;
				break;
			}
#endif
		}
		if (cmpStatus & GD_CMP_COLOR) {
			break;
		}
	}

	return cmpStatus;
}

int
gdAlphaBlend (int dst, int src)
{
	/* 2.0.12: TBB: alpha in the destination should be a
	 * component of the result. Thanks to Frank Warmerdam for
	 * pointing out the issue.
	 */
	return ((((gdTrueColorGetAlpha (src) *
	     gdTrueColorGetAlpha (dst)) / gdAlphaMax) << 24) +
	  ((((gdAlphaTransparent - gdTrueColorGetAlpha (src)) *
	     gdTrueColorGetRed (src) / gdAlphaMax) +
	    (gdTrueColorGetAlpha (src) *
	     gdTrueColorGetRed (dst)) / gdAlphaMax) << 16) +
	  ((((gdAlphaTransparent - gdTrueColorGetAlpha (src)) *
	     gdTrueColorGetGreen (src) / gdAlphaMax) +
	    (gdTrueColorGetAlpha (src) *
	     gdTrueColorGetGreen (dst)) / gdAlphaMax) << 8) +
	  (((gdAlphaTransparent - gdTrueColorGetAlpha (src)) *
	    gdTrueColorGetBlue (src) / gdAlphaMax) +
	   (gdTrueColorGetAlpha (src) *
	    gdTrueColorGetBlue (dst)) / gdAlphaMax));
}

void gdImageAlphaBlending (gdImagePtr im, int alphaBlendingArg)
{
	im->alphaBlendingFlag = alphaBlendingArg;
}

void gdImageAntialias (gdImagePtr im, int antialias)
{
	if (im->trueColor){
		im->antialias = antialias;
	}
}

void gdImageSaveAlpha (gdImagePtr im, int saveAlphaArg)
{
	im->saveAlphaFlag = saveAlphaArg;
}

static int gdLayerOverlay (int dst, int src)
{
	int a1, a2;
	a1 = gdAlphaMax - gdTrueColorGetAlpha(dst);
	a2 = gdAlphaMax - gdTrueColorGetAlpha(src);
	return ( ((gdAlphaMax - a1*a2/gdAlphaMax) << 24) +
		(gdAlphaOverlayColor( gdTrueColorGetRed(src), gdTrueColorGetRed(dst), gdRedMax ) << 16) +
		(gdAlphaOverlayColor( gdTrueColorGetGreen(src), gdTrueColorGetGreen(dst), gdGreenMax ) << 8) +
		(gdAlphaOverlayColor( gdTrueColorGetBlue(src), gdTrueColorGetBlue(dst), gdBlueMax ))
		);
}

static int gdAlphaOverlayColor (int src, int dst, int max )
{
	/* this function implements the algorithm
	 *
	 * for dst[rgb] < 0.5,
	 *   c[rgb] = 2.src[rgb].dst[rgb]
	 * and for dst[rgb] > 0.5,
	 *   c[rgb] = -2.src[rgb].dst[rgb] + 2.dst[rgb] + 2.src[rgb] - 1
	 *
	 */

	dst = dst << 1;
	if( dst > max ) {
		/* in the "light" zone */
		return dst + (src << 1) - (dst * src / max) - max;
	} else {
		/* in the "dark" zone */
		return dst * src / max;
	}
}

void gdImageSetClip (gdImagePtr im, int x1, int y1, int x2, int y2)
{
	if (x1 < 0) {
		x1 = 0;
	}
	if (x1 >= im->sx) {
		x1 = im->sx - 1;
	}
	if (x2 < 0) {
		x2 = 0;
	}
	if (x2 >= im->sx) {
		x2 = im->sx - 1;
	}
	if (y1 < 0) {
		y1 = 0;
	}
	if (y1 >= im->sy) {
		y1 = im->sy - 1;
	}
	if (y2 < 0) {
		y2 = 0;
	}
	if (y2 >= im->sy) {
		y2 = im->sy - 1;
	}
	im->cx1 = x1;
	im->cy1 = y1;
	im->cx2 = x2;
	im->cy2 = y2;
}

void gdImageGetClip (gdImagePtr im, int *x1P, int *y1P, int *x2P, int *y2P)
{
	*x1P = im->cx1;
	*y1P = im->cy1;
	*x2P = im->cx2;
	*y2P = im->cy2;
}


/* Filters function added on 2003/12
 * by Pierre-Alain Joye (pajoye@pearfr.org)
 **/
/* Begin filters function */
#ifndef HAVE_GET_TRUE_COLOR
#define GET_PIXEL_FUNCTION(src)(src->trueColor?gdImageGetTrueColorPixel:gdImageGetPixel)
#endif

/* invert src image */
int gdImageNegate(gdImagePtr src)
{
	int x, y;
	int r,g,b,a;
	int new_pxl, pxl;
	typedef int (*FuncPtr)(gdImagePtr, int, int);
	FuncPtr f;

	if (src==NULL) {
		return 0;
	}

	f = GET_PIXEL_FUNCTION(src);

	for (y=0; y<src->sy; ++y) {
		for (x=0; x<src->sx; ++x) {
			pxl = f (src, x, y);
			r = gdImageRed(src, pxl);
			g = gdImageGreen(src, pxl);
			b = gdImageBlue(src, pxl);
			a = gdImageAlpha(src, pxl);

			new_pxl = gdImageColorAllocateAlpha(src, 255-r, 255-g, 255-b, a);
			if (new_pxl == -1) {
				new_pxl = gdImageColorClosestAlpha(src, 255-r, 255-g, 255-b, a);
			}
			if ((y >= 0) && (y < src->sy)) {
				gdImageSetPixel (src, x, y, new_pxl);
			}
		}
	}
	return 1;
}

/* Convert the image src to a grayscale image */
int gdImageGrayScale(gdImagePtr src)
{
	int x, y;
	int r,g,b,a;
	int new_pxl, pxl;
	typedef int (*FuncPtr)(gdImagePtr, int, int);
	FuncPtr f;
	f = GET_PIXEL_FUNCTION(src);

	if (src==NULL) {
		return 0;
	}

	for (y=0; y<src->sy; ++y) {
		for (x=0; x<src->sx; ++x) {
			pxl = f (src, x, y);
			r = gdImageRed(src, pxl);
			g = gdImageGreen(src, pxl);
			b = gdImageBlue(src, pxl);
			a = gdImageAlpha(src, pxl);
			r = g = b = (int) (.299 * r + .587 * g + .114 * b);

			new_pxl = gdImageColorAllocateAlpha(src, r, g, b, a);
			if (new_pxl == -1) {
				new_pxl = gdImageColorClosestAlpha(src, r, g, b, a);
			}
			if ((y >= 0) && (y < src->sy)) {
				gdImageSetPixel (src, x, y, new_pxl);
			}
		}
	}
	return 1;
}

/* Set the brightness level <level> for the image src */
int gdImageBrightness(gdImagePtr src, int brightness)
{
	int x, y;
	int r,g,b,a;
	int new_pxl, pxl;
	typedef int (*FuncPtr)(gdImagePtr, int, int);
	FuncPtr f;
	f = GET_PIXEL_FUNCTION(src);

	if (src==NULL || (brightness < -255 || brightness>255)) {
		return 0;
	}

	if (brightness==0) {
		return 1;
	}

	for (y=0; y<src->sy; ++y) {
		for (x=0; x<src->sx; ++x) {
			pxl = f (src, x, y);

			r = gdImageRed(src, pxl);
			g = gdImageGreen(src, pxl);
			b = gdImageBlue(src, pxl);
			a = gdImageAlpha(src, pxl);

			r = r + brightness;
			g = g + brightness;
			b = b + brightness;

			r = (r > 255)? 255 : ((r < 0)? 0:r);
			g = (g > 255)? 255 : ((g < 0)? 0:g);
			b = (b > 255)? 255 : ((b < 0)? 0:b);

			new_pxl = gdImageColorAllocateAlpha(src, (int)r, (int)g, (int)b, a);
			if (new_pxl == -1) {
				new_pxl = gdImageColorClosestAlpha(src, (int)r, (int)g, (int)b, a);
			}
			if ((y >= 0) && (y < src->sy)) {
				gdImageSetPixel (src, x, y, new_pxl);
			}
		}
	}
	return 1;
}


int gdImageContrast(gdImagePtr src, double contrast)
{
	int x, y;
	int r,g,b,a;
	double rf,gf,bf;
	int new_pxl, pxl;
	typedef int (*FuncPtr)(gdImagePtr, int, int);

	FuncPtr f;
	f = GET_PIXEL_FUNCTION(src);

	if (src==NULL) {
		return 0;
	}

	contrast = (double)(100.0-contrast)/100.0;
	contrast = contrast*contrast;

	for (y=0; y<src->sy; ++y) {
		for (x=0; x<src->sx; ++x) {
			pxl = f(src, x, y);

			r = gdImageRed(src, pxl);
			g = gdImageGreen(src, pxl);
			b = gdImageBlue(src, pxl);
			a = gdImageAlpha(src, pxl);

			rf = (double)r/255.0;
			rf = rf-0.5;
			rf = rf*contrast;
			rf = rf+0.5;
			rf = rf*255.0;

			bf = (double)b/255.0;
			bf = bf-0.5;
			bf = bf*contrast;
			bf = bf+0.5;
			bf = bf*255.0;

			gf = (double)g/255.0;
			gf = gf-0.5;
			gf = gf*contrast;
			gf = gf+0.5;
			gf = gf*255.0;

			rf = (rf > 255.0)? 255.0 : ((rf < 0.0)? 0.0:rf);
			gf = (gf > 255.0)? 255.0 : ((gf < 0.0)? 0.0:gf);
			bf = (bf > 255.0)? 255.0 : ((bf < 0.0)? 0.0:bf);

			new_pxl = gdImageColorAllocateAlpha(src, (int)rf, (int)gf, (int)bf, a);
			if (new_pxl == -1) {
				new_pxl = gdImageColorClosestAlpha(src, (int)rf, (int)gf, (int)bf, a);
			}
			if ((y >= 0) && (y < src->sy)) {
				gdImageSetPixel (src, x, y, new_pxl);
			}
		}
	}
	return 1;
}


int gdImageColor(gdImagePtr src, int red, int green, int blue)
{
	int x, y;
	int r,g,b,a;
	int new_pxl, pxl;
	typedef int (*FuncPtr)(gdImagePtr, int, int);
	FuncPtr f;

	if (src==NULL || (red<-255||red>255) || (green<-255||green>255) || (blue<-255||blue>255)) {
		return 0;
	}

	f = GET_PIXEL_FUNCTION(src);

	for (y=0; y<src->sy; ++y) {
		for (x=0; x<src->sx; ++x) {
			pxl = f(src, x, y);
			r = gdImageRed(src, pxl);
			g = gdImageGreen(src, pxl);
			b = gdImageBlue(src, pxl);
			a = gdImageAlpha(src, pxl);

			r = r + red;
			g = g + green;
			b = b + blue;

			r = (r > 255)? 255 : ((r < 0)? 0:r);
			g = (g > 255)? 255 : ((g < 0)? 0:g);
			b = (b > 255)? 255 : ((b < 0)? 0:b);

			new_pxl = gdImageColorAllocateAlpha(src, (int)r, (int)g, (int)b, a);
			if (new_pxl == -1) {
				new_pxl = gdImageColorClosestAlpha(src, (int)r, (int)g, (int)b, a);
			}
			if ((y >= 0) && (y < src->sy)) {
				gdImageSetPixel (src, x, y, new_pxl);
			}
		}
	}
	return 1;
}

int gdImageConvolution(gdImagePtr src, float filter[3][3], float filter_div, float offset)
{
	int         x, y, i, j, new_a;
	float       new_r, new_g, new_b;
	int         new_pxl, pxl=0;
	gdImagePtr  srcback;
	typedef int (*FuncPtr)(gdImagePtr, int, int);
	FuncPtr f;

	if (src==NULL) {
		return 0;
	}

	/* We need the orinal image with each safe neoghb. pixel */
	srcback = gdImageCreateTrueColor (src->sx, src->sy);
	gdImageCopy(srcback, src,0,0,0,0,src->sx,src->sy);

	if (srcback==NULL) {
		return 0;
	}

	f = GET_PIXEL_FUNCTION(src);

	for ( y=0; y<src->sy; y++) {
		for(x=0; x<src->sx; x++) {
			new_r = new_g = new_b = 0;
			new_a = gdImageAlpha(srcback, pxl);

			for (j=0; j<3; j++) {
				int yv = MIN(MAX(y - 1 + j, 0), src->sy - 1);
				for (i=0; i<3; i++) {
				        pxl = f(srcback, MIN(MAX(x - 1 + i, 0), src->sx - 1), yv);
					new_r += (float)gdImageRed(srcback, pxl) * filter[j][i];
					new_g += (float)gdImageGreen(srcback, pxl) * filter[j][i];
					new_b += (float)gdImageBlue(srcback, pxl) * filter[j][i];
				}
			}

			new_r = (new_r/filter_div)+offset;
			new_g = (new_g/filter_div)+offset;
			new_b = (new_b/filter_div)+offset;

			new_r = (new_r > 255.0f)? 255.0f : ((new_r < 0.0f)? 0.0f:new_r);
			new_g = (new_g > 255.0f)? 255.0f : ((new_g < 0.0f)? 0.0f:new_g);
			new_b = (new_b > 255.0f)? 255.0f : ((new_b < 0.0f)? 0.0f:new_b);

			new_pxl = gdImageColorAllocateAlpha(src, (int)new_r, (int)new_g, (int)new_b, new_a);
			if (new_pxl == -1) {
				new_pxl = gdImageColorClosestAlpha(src, (int)new_r, (int)new_g, (int)new_b, new_a);
			}
			if ((y >= 0) && (y < src->sy)) {
				gdImageSetPixel (src, x, y, new_pxl);
			}
		}
	}
	gdImageDestroy(srcback);
	return 1;
}

int gdImageSelectiveBlur( gdImagePtr src)
{
	int         x, y, i, j;
	float       new_r, new_g, new_b;
	int         new_pxl, cpxl, pxl, new_a=0;
	float flt_r [3][3];
	float flt_g [3][3];
	float flt_b [3][3];
	float flt_r_sum, flt_g_sum, flt_b_sum;

	gdImagePtr srcback;
	typedef int (*FuncPtr)(gdImagePtr, int, int);
	FuncPtr f;

	if (src==NULL) {
		return 0;
	}

	/* We need the orinal image with each safe neoghb. pixel */
	srcback = gdImageCreateTrueColor (src->sx, src->sy);
	gdImageCopy(srcback, src,0,0,0,0,src->sx,src->sy);

	if (srcback==NULL) {
		return 0;
	}

	f = GET_PIXEL_FUNCTION(src);

	for(y = 0; y<src->sy; y++) {
		for (x=0; x<src->sx; x++) {
		      flt_r_sum = flt_g_sum = flt_b_sum = 0.0;
			cpxl = f(src, x, y);

			for (j=0; j<3; j++) {
				for (i=0; i<3; i++) {
					if ((j == 1) && (i == 1)) {
						flt_r[1][1] = flt_g[1][1] = flt_b[1][1] = 0.5;
					} else {
						pxl = f(src, x-(3>>1)+i, y-(3>>1)+j);
						new_a = gdImageAlpha(srcback, pxl);

						new_r = ((float)gdImageRed(srcback, cpxl)) - ((float)gdImageRed (srcback, pxl));

						if (new_r < 0.0f) {
							new_r = -new_r;
						}
						if (new_r != 0) {
							flt_r[j][i] = 1.0f/new_r;
						} else {
							flt_r[j][i] = 1.0f;
						}

						new_g = ((float)gdImageGreen(srcback, cpxl)) - ((float)gdImageGreen(srcback, pxl));

						if (new_g < 0.0f) {
							new_g = -new_g;
						}
						if (new_g != 0) {
							flt_g[j][i] = 1.0f/new_g;
						} else {
							flt_g[j][i] = 1.0f;
						}

						new_b = ((float)gdImageBlue(srcback, cpxl)) - ((float)gdImageBlue(srcback, pxl));

						if (new_b < 0.0f) {
							new_b = -new_b;
						}
						if (new_b != 0) {
							flt_b[j][i] = 1.0f/new_b;
						} else {
							flt_b[j][i] = 1.0f;
						}
					}

					flt_r_sum += flt_r[j][i];
					flt_g_sum += flt_g[j][i];
					flt_b_sum += flt_b [j][i];
				}
			}

			for (j=0; j<3; j++) {
				for (i=0; i<3; i++) {
					if (flt_r_sum != 0.0) {
						flt_r[j][i] /= flt_r_sum;
					}
					if (flt_g_sum != 0.0) {
						flt_g[j][i] /= flt_g_sum;
					}
					if (flt_b_sum != 0.0) {
						flt_b [j][i] /= flt_b_sum;
					}
				}
			}

			new_r = new_g = new_b = 0.0;

			for (j=0; j<3; j++) {
				for (i=0; i<3; i++) {
					pxl = f(src, x-(3>>1)+i, y-(3>>1)+j);
					new_r += (float)gdImageRed(srcback, pxl) * flt_r[j][i];
					new_g += (float)gdImageGreen(srcback, pxl) * flt_g[j][i];
					new_b += (float)gdImageBlue(srcback, pxl) * flt_b[j][i];
				}
			}

			new_r = (new_r > 255.0f)? 255.0f : ((new_r < 0.0f)? 0.0f:new_r);
			new_g = (new_g > 255.0f)? 255.0f : ((new_g < 0.0f)? 0.0f:new_g);
			new_b = (new_b > 255.0f)? 255.0f : ((new_b < 0.0f)? 0.0f:new_b);
			new_pxl = gdImageColorAllocateAlpha(src, (int)new_r, (int)new_g, (int)new_b, new_a);
			if (new_pxl == -1) {
				new_pxl = gdImageColorClosestAlpha(src, (int)new_r, (int)new_g, (int)new_b, new_a);
			}
			if ((y >= 0) && (y < src->sy)) {
				gdImageSetPixel (src, x, y, new_pxl);
			}
		}
	}
	gdImageDestroy(srcback);
	return 1;
}

int gdImageEdgeDetectQuick(gdImagePtr src)
{
	float filter[3][3] =	{{-1.0,0.0,-1.0},
				{0.0,4.0,0.0},
				{-1.0,0.0,-1.0}};

	return gdImageConvolution(src, filter, 1, 127);
}

int gdImageGaussianBlur(gdImagePtr im)
{
	float filter[3][3] =	{{1.0,2.0,1.0},
				{2.0,4.0,2.0},
				{1.0,2.0,1.0}};

	return gdImageConvolution(im, filter, 16, 0);
}

int gdImageEmboss(gdImagePtr im)
{
/*
	float filter[3][3] =	{{1.0,1.0,1.0},
				{0.0,0.0,0.0},
				{-1.0,-1.0,-1.0}};
*/
	float filter[3][3] =	{{ 1.5, 0.0, 0.0},
				 { 0.0, 0.0, 0.0},
				 { 0.0, 0.0,-1.5}};

	return gdImageConvolution(im, filter, 1, 127);
}

int gdImageMeanRemoval(gdImagePtr im)
{
	float filter[3][3] =	{{-1.0,-1.0,-1.0},
				{-1.0,9.0,-1.0},
				{-1.0,-1.0,-1.0}};

	return gdImageConvolution(im, filter, 1, 0);
}

int gdImageSmooth(gdImagePtr im, float weight)
{
	float filter[3][3] =	{{1.0,1.0,1.0},
				{1.0,0.0,1.0},
				{1.0,1.0,1.0}};

	filter[1][1] = weight;

	return gdImageConvolution(im, filter, weight+8, 0);
}
/* End filters function */
