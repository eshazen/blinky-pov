/*
 * test Accelerometer
 */

#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <util/delay.h>

#include "neopixel.h"
#include "adc.h"
#include "uart.h"
#include "download.h"
#include "i2c.h"

#include "button.h"

#define abs(x) ((x)<0?(-(x)):(x))

FILE usart0_str = FDEV_SETUP_STREAM(USART0SendByte, NULL, _FDEV_SETUP_WRITE);

extern uint8_t ihex_nd;
extern uint8_t ihex[BUFSIZ];
static uint16_t c, d;

static int8_t rc;
static int8_t n;

static uint8_t abuf[1];
static uint8_t dbuf[1];
static uint8_t rbuf[3];

static uint8_t left;


void acc_print( char c, uint8_t val);

void send_all() {
  uint8_t i;
  for( i=0; i<8; i++)
    sendPixel( led_g[i], led_r[i], led_b[i]);
  show();
}

void clear_all() {
  int8_t k;
  for( k=0; k<8; k++)
    led_r[k] = led_g[k] = led_b[k] = 0;
}

// threshold in slope units for sweep trigger
#define STHR 3
// threshold in +/- accel units for sweep trigger
#define ATHR 5

#define NPTS 10
// precomputed sum of the squares of n=0..9
#define SUMX2 285
static int8_t pts[NPTS];
int32_t sumxy;
int32_t slope;
int32_t x;

int main (void)
{
  ledsetup();
  InitADC();
  USART0Init();
  BUTTON_PORT |= (1<<BUTTON_BIT);
  i2c_init( BDIV);

  // setup timer 1 to wrap every 100us, no interrupts
  TCCR1A = 0;
  TCCR1B = 0x05;		/* mode 0, prescale 1024 */

  stdout = &usart0_str;
  _delay_ms(10);

  for( n=0; n<8; n++) {
    led_r[n] = led_g[n] = led_b[n] = 0x40;
    send_all();
    _delay_ms(100);
  }
  
  for( n=0; n<8; n++) {
    led_r[n] = led_g[n] = led_b[n] = 0;
    send_all();
    _delay_ms(100);
  }
  
  puts("Hello\n");

  // write 1 to accelerometer mode register
  abuf[0] = 7;
  dbuf[0] = 1;

  rc = i2c_io( ACC_I2C_ADR, abuf, 1, dbuf, 1, NULL, 0);
  if( rc) printf("I2C err=%d\n", rc);

  abuf[0] = 0;

  uint32_t n = 0;

  // loop reading out and printing X/Y/Z values
  while( 1) {

    while( TCNT1 < 78)
      ;

    TCNT1 = 0;
    
    rc = i2c_io( ACC_I2C_ADR, abuf, 1, NULL, 0, rbuf, 1);
    if( rc) printf("I2C err=%d\n", rc);
    if( !(rbuf[0] & 0x40)) {
      // convert acceleration to signed number
      int8_t v = (rbuf[0] & 0x3f) << 2;
      v /= 4;
      printf("%ld %d\n", n, v);
      ++n;
    }

  }

}


void acc_print( char c, uint8_t val) {
  putchar(' ');
  putchar( c);
  putchar( '=');
  if( val & 0x40)
    puts("ERR");
  else {
    int8_t v = (val & 0x3f) << 2;
    v /= 4;
    printf(" %3d", v);
  }
}
