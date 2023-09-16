
#include <stdio.h>

#include "display.h"
#include "neopixel.h"
#include "font.h"
#include "download.h"

// #define DEBUG

#define INTEN 0xff

uint8_t pal_r[] = { 0x00, INTEN, 0x00, 0x00, INTEN, 0x00, INTEN, INTEN };
uint8_t pal_g[] = { 0x00, 0x00, INTEN, 0x00, INTEN, INTEN, 0x00, INTEN };
uint8_t pal_b[] = { 0x00, 0x00, 0x00, INTEN, 0x00, INTEN, INTEN, INTEN };

uint8_t font_lo = 32;
uint8_t font_hi = 96;
uint8_t font_hgt = 8;

#define MAX_PIXELS (MAX_MSG*6)
uint8_t g_pixels[MAX_PIXELS];
uint8_t g_colors[MAX_PIXELS];
uint8_t g_numpix;

extern uint8_t irec_nd;
extern code_t irec[BUFSIZ];

//
// convert message to pixel and color data
// return length in pixels
// store results in globals g_pixels, g_colors and g_numpix
//
void msg_to_pixels()
{
  uint8_t i, k, n;
  uint8_t ch, colr;
  code_t *p;
  uint8_t* fp;
  uint8_t* p_pix;
  uint8_t* p_col;

  p = irec+IREC_DATA;
  p_pix = g_pixels;
  p_col = g_colors;

  for( i=0; i<irec_nd; i++) {
    ch = *p & 0x3f;		/* get character code */
    colr = (*p >> 6) & 7;	/* get color value */
    fp = lookup_font( ch);	/* lookup character in font */
    p++;			/* advance past character */
    n = *fp++;			/* get width of char in pixels */
    printf("%d ch=0x%x colr=%d fp=%d len=%d\n", i, ch, colr, (fp-1)-font, n);
    if( n) {			/* not a space */
      for( k=0; k<n; k++) {
	*p_pix++ = *fp;
	*p_col++ = colr;
	++fp;
      }
      *p_pix++ = 0;		/* space between letters */
      *p_col++ = 0;
    }
  }

  g_numpix = p_pix - g_pixels;
}


// display column idx from current pixel-ated message
void display_column( uint8_t idx)
{
  uint8_t i;
  uint8_t b;
  uint8_t colr = g_colors[idx];
  uint8_t bits = g_pixels[idx];
  uint8_t rr = pal_r[colr];
  uint8_t gg = pal_g[colr];
  uint8_t bb = pal_b[colr];

  b = 0x80;
  for( i=0; i<8; i++) {
    if( bits & b) {
      sendPixel( gg, rr, bb);
    } else {
      sendPixel( 0, 0, 0);
    }
    b >>= 1;
  }
  show();
}


// lookup character in font
uint8_t* lookup_font( uint8_t ch)
{
  uint8_t i;
  uint8_t* p;
  p = font;

  if( ch && ch < (font_hi-font_lo) ) {
    for( i=0; i<ch; i++)
      p += *p + 1;
  }
  return p;
}
