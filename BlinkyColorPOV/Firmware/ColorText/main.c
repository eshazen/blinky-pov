/*
 * full-function color Blinky text display
 *
 * Power-up check for correctly-checksummed message in EEPROM
 * if not found, load default red "Hello" message
 * Display green on top LED if EEPROM checksum is good, else red
 *
 * convert message to global arrays of pixels and colors
 * display when back-and-forth motion is sensed on i2c accelerometer.
 *
 * If button pressed, download message using simplified blinky
 * DDR protocol based on original Wayne&Layne idea.
 */

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/eeprom.h>

#include <util/delay.h>

#include "neopixel.h"
#include "adc.h"
#include "uart.h"
#include "download.h"
#include "i2c.h"

#include "display.h"

#include "button.h"

// #define DEBUG

// setup uart for 9600 baud output
FILE usart0_str = FDEV_SETUP_STREAM(USART0SendByte, NULL, _FDEV_SETUP_WRITE);

// reference download record and count for text message
extern uint8_t irec_nd;
extern code_t irec[BUFSIZ];

static uint16_t c, d;

// global arrays to hold pixelated message
#define MAX_PIXELS (MAX_MSG*6)
extern uint8_t g_pixels[MAX_PIXELS];
extern uint8_t g_colors[MAX_PIXELS];
extern uint8_t g_numpix;
static int8_t i_pix;

// delay before starting display in percent
#define FRONT_PORCH 80
// delay after end of display in percent
#define BACK_PORCH 120

static int8_t rc;
static int8_t n;

// buffers for funky i2c function
static uint8_t abuf[1];
static uint8_t dbuf[1];
static uint8_t rbuf[3];

// deal with 32-bit non-interrupt driven timer at 120uS tick rate
static uint16_t tyme_lo;
static uint16_t tyme_hi;
#define TIME32 (((uint32_t)tyme_hi<<16)|((uint32_t)tyme_lo))

static uint32_t tyme_accel;
static uint32_t trig_tyme;
static uint32_t last_trig;

#define ACCEL_DELAY 78

// history of accel readings with timestamps
#define NPTS 10
static int8_t pts[NPTS];
static uint32_t times[NPTS];

int32_t x;
uint8_t left;
uint8_t tick;

// two of these hold the most recent sweep of the gizmo
// and the one before that
typedef struct {
  uint32_t pixel_first;		// time in sweep to start message
  uint16_t pixel_period;	// period between pixel columns
  int8_t dir;			// direction +/-1
  uint8_t valid;		// 1 if valid
} pixel_info_t;

// other variables used in calculating pixel display
uint32_t pixel_next;
uint32_t pixel_last;
uint16_t pixel_tick;
uint8_t pixel_idx;
uint8_t pixel_count;
int8_t pixel_dir;
uint8_t alive;

static pixel_info_t pix[2];
static pixel_info_t *pp;
static uint8_t active_pix;

// turn off all LEDs
#define blank() set_all(0,0,0)

// set selected LED only, others blank, update all
void set_one( uint8_t n, uint8_t r, uint8_t g, uint8_t b) {
  uint8_t i;
  for( i=0; i<8; i++) {
    if( i == n)
      sendPixel( r, g, b);
    else
      sendPixel( 0, 0, 0);
  }
  show();
}

// set all LEDs to same color
void set_all( uint8_t r, uint8_t g, uint8_t b) {
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  show();
}

// update only first LED (for speed)
void setfirst( uint8_t r, uint8_t g, uint8_t b) {
  sendPixel( g, r, b);
  show();
}  

int main (void)
{
  uint8_t i;

  ledsetup();
  InitADC();
  USART0Init();
  BUTTON_PORT |= (1<<BUTTON_BIT);
  i2c_init( BDIV);

  stdout = &usart0_str;
  _delay_ms(10);
  blank();

  // setup timer 1 to wrap every 120us, no interrupts
  TCCR1A = 0;
  TCCR1B = 0x05;		/* mode 0, prescale 1024 */

  // write 1 to accelerometer mode register to turn it on
  abuf[0] = 7;
  dbuf[0] = 1;
  rc = i2c_io( ACC_I2C_ADR, abuf, 1, dbuf, 1, NULL, 0);
#ifdef DEBUG
  if( rc) printf("I2C err=%d\n", rc);
#endif
  // set address for subsequent I2C reads
#ifdef PCB_REV3
  abuf[0] = 1;
#else  
  abuf[0] = 0;
#endif
  
  // power-up LED test, chase white LED
  blank();
  for( i=0; i<8; i++) {
    set_one( i, 20, 20, 20);
    _delay_ms(100);
  }
  blank();
  setfirst( 10, 0, 0);

  // try to read message from eeprom
  eeprom_read_block( irec, 0, sizeof(irec));
  if( check_sum()) {
    setfirst( 10, 0, 0);	// first LED red if fail
    // preload with "Hello" message
    defmsg();
  } else {
    setfirst( 0, 10, 0);	// first LED green if OK
    irec_nd = irec[IREC_LEN];
  }

  _delay_ms(100);

#ifdef DEBUG
  printf("Hello\n");
  printf("rec len=%d\n", irec_nd);
#endif
  // convert message to pixels
  msg_to_pixels();
  TCNT1 = 0;			// reset timer
  tyme_accel = TCNT1;

#ifdef DEBUG
  printf("numpix = %d\n", g_numpix);
#endif

  while( 1) {			// ----------main loop -----------

    if( button_pressed()) {	// button press -- download message
      _delay_ms( 500);
      setfirst( 10, 10, 0);
      calibrate();		// set black level
      setfirst( 0, 10, 10);
      rc = rx_irec();		// receive blinky record, return if
                                // button pressed
      if( rc) {
	setfirst( 10, 0, 0);	// checksum error in download
      } else {
	eeprom_write_block( irec, 0, sizeof(irec));
	setfirst( 0, 10, 0);	// checksum good
	msg_to_pixels();
      }
      _delay_ms(100);
      TCNT1 = 0;		// reset timer to start display
      tyme_lo = tyme_hi = 0;
    }


    // wait for timer tick, every 120uS
    while( tyme_lo == TCNT1)
      ;
    if( TCNT1 < tyme_lo)	// update upper 16 bits
      ++tyme_hi;
    tyme_lo = TCNT1;

    // see if time for accelerometer update
    // every 10ms
    if( TIME32 - tyme_accel > ACCEL_DELAY) {
      tyme_accel = TIME32;

      // read accelerometer
      rc = i2c_io( ACC_I2C_ADR, abuf, 1, NULL, 0, rbuf, 3);
#ifdef DEBUG
      if( rc) printf("I2C err=%d\n", rc);
#endif
      if( !(rbuf[0] & 0x40)) {	// check for read/write collision
	// convert acceleration to signed number
	int8_t v = (rbuf[0] & 0x3f) << 2;
	v /= 4;

	// keep history of NPTS points with timestamps
	memmove( pts, pts+1, NPTS-1);
	pts[NPTS-1] = v;

	memmove( &times[0], &times[1], sizeof(times[0])*NPTS-1);
	times[NPTS-1] = TIME32;

	// check for zero-crossing positive-going
	if( pts[0] < 0 && pts[1] < 0 && pts[2] < 0 &&
	    pts[NPTS-1] > 0 && pts[NPTS-2] > 0 && pts[NPTS-3] > 0 && !left && !tick) {
	  // trigger on left-going sweep
	  left = 1;
	  last_trig = trig_tyme;
	  trig_tyme = times[NPTS/2];
	  tick = 1;
	} else if( pts[0] > 0 && pts[1] > 0 && pts[2] > 0 &&
		   pts[NPTS-1] < 0 && pts[NPTS-2] < 0 && pts[NPTS-3] < 0 && left && !tick) {
	  // trigger on right-going sweep
	  left = 0;
	  last_trig = trig_tyme;
	  trig_tyme = times[NPTS/2]; // approx. time of zero crossing
	  tick = 1;
	}
      }	// valid I2C data

      // this macro gives the time in 120uS ticks since last trigger
#define TrigDelay (trig_tyme-last_trig)

      if( tick && TrigDelay < 8000) { // about 1s maximum for half-cycle
	// setup for next period
	active_pix ^= 1;	// toggle to other struct
	pp = &pix[active_pix];
	pp->pixel_first = trig_tyme + ((TrigDelay * FRONT_PORCH) / 100L);      // start time
	pixel_last = trig_tyme + ((TrigDelay * BACK_PORCH) / 100L);            // end time
	pp->pixel_period = (pixel_last-pp->pixel_first)/((uint32_t)g_numpix);  // pixel period
	pp->dir = left ? 1 : -1;
#ifdef DEBUG
	printf("%ld %d %ld %d\n", TrigDelay, left, pp->pixel_first-TIME32, pp->pixel_period);
#endif
	pp->valid = 1;

	// dump pixels
#ifdef DEBUG
	printf("num=%d\n", g_numpix);
	//	uint8_t k;
	//	for( k=0; k<g_numpix; k++)
	//	  printf("%d %02x\n", g_colors[k], g_pixels[k]);
#endif
      }
      tick = 0;
    } //----- accel update ----------

    if( alive) {		/* display active? */
      if( TIME32 >= pixel_next) {
	pixel_next += pixel_tick;
	display_column( pixel_idx);
	pixel_idx += pixel_dir;
	if( --pixel_count == 0) {
	  blank();
	  alive = 0;
	}
      }

    } else {			/* display not active,
				   check if time to start */
      pp = &pix[active_pix];
      if( pp->valid && TIME32 >= pp->pixel_first) {
	pp->valid = 0;		/* we've used this one */
	pixel_count = g_numpix;
	pixel_next = pp->pixel_first + pp->pixel_period;
	pixel_tick = pp->pixel_period;
	pixel_dir = pp->dir;
	pixel_idx = pp->dir > 0 ? 0 : g_numpix-1;
	alive = 1;
      }
    }

  } // -------- main loop ----------

}



