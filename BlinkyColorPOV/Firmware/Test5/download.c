/*
 * blinky serial download interface
 * sense DDR clock on ADC0, serial data on ADC1
 * pseudo-intel-hex format
 */

#include "download.h"
#include "button.h"
#include <stdio.h>
#include "uart.h"
#include "adc.h"

// variables for blinky receiver
static int16_t base_d, base_c;
static int32_t sum_d, sum_c;
int32_t max_d, max_c;
static int32_t d, c;

// loop variable
static uint16_t k;
static int16_t a;

// number of bytes in latest hex record
uint8_t ihex_nd;

// address of latest hex record
static uint16_t ihex_addr;

// flag:  button pressed, do reset
static uint8_t do_reset;

// global mode
static uint8_t mode;

// buffer for intel hex records (size 16)
uint8_t ihex[BUFSIZ];

void setfirst( uint8_t r, uint8_t g, uint8_t b);

void calibrate() {
      // measure background level
      sum_d = sum_c = 0;
      for( k=0; k<NAV; k++) {
	a = ReadADC( ADC_DATA);
	sum_d += a;
	a = ReadADC( ADC_CLK);
	sum_c += a;
	_delay_ms(1);
      }
      base_d = sum_d / NAV;
      base_c = sum_c / NAV;

      printf("base_d=%d base_c=%d\n", base_d, base_c);
}



// sample the light sensor once (with averaging)
// return bit 0 = clk state  bit 1 = data state
uint8_t sample_clk_data() {
  uint8_t rv;

  sum_d = sum_c = 0;
  for( k=0; k<NAV; k++) {
    a = ReadADC( ADC_DATA);
    sum_d += a;
    a = ReadADC( ADC_CLK);
    sum_c += a;
  }

  rv = 0;
  d = (base_d - (sum_d / NAV));
  c = (base_c - (sum_c / NAV));
  if( d > max_d) max_d = d;
  if( c > max_c) max_c = c;
 
  if( d >= THR)
    rv |= DATA_VAL;
  if( c >= THR)
    rv |= CLK_VAL;

  return rv;
}


//
// wait for clock state change or button press
// return data state, set do_reset on button press
//
uint8_t wait_for_clock( uint8_t c)
{
  uint8_t s;

  do {
    s = sample_clk_data();
    if( button_pressed()) {
      do_reset = 1;
      return 0;
    }
  } while( (s & CLK_VAL) != c);

  return( (s & DATA_VAL) != 0);
}

// receive blinky byte
uint8_t rx_byte() {

  uint8_t bit, byte, d;
  uint8_t i;

  bit = 0x80;
  byte = 0;

  for( i=0; i<4; i++) {
    d = wait_for_clock( 1);
    if( do_reset) return 0;
    if( d)
      byte |= bit;
    bit >>= 1;
    d = wait_for_clock( 0);
    if( do_reset) return 0;
    if( d)
      byte |= bit;
    bit >>= 1;
  }

  setfirst( (byte & 0xc0) >> 3, (byte & 0x38), (byte & 7) << 3);

  //  printf("%02x ", byte);
  //  putchar('.');
  
  return byte;
}

// receive an intel hex record into buffer
// return type, or
//  -1 for checksum error
//  -2 if button pressed
//
int rx_ihex() {

  uint8_t* p = ihex;
  uint8_t i, n, c, s;

  do_reset = 0;

  n  = rx_byte();	/* get length */
  //  if( n == 0)		/* ignore length 0 records for now */
  //    return 0;

  ihex_nd = n;
  *p++ = n;

  for( i=0; i<3; i++) {
    *p++ = rx_byte();	/* get addr_hi, addr_lo, type */
    if( do_reset) return -2;
  }
  ihex_nd = ihex[IHEX_LEN];

  ihex_addr = (ihex[IHEX_ADDR] << 8) | ihex[IHEX_ADDR+1];
  if( ihex_nd > 0 && ihex_nd <= 0x10) {
    for( i=0; i<ihex_nd; i++) {
      *p++ = rx_byte();		/* get data */
      if( do_reset) return -2;
    }
  }
  c = rx_byte();		/* get checksum */
  *p++ = c;			/* store checksum at end */
  if( do_reset) return -2;
  // calculate sum of received bytes
  s = 0;
  for( i=0; i<(ihex_nd+4); i++)
    s += ihex[i];

  s = (s ^ 0xff)+1;
  *p++ = s;			/* store raw sum */

  if( c == s ) {
    return ihex[IHEX_TYPE];
  } else {
    return -1;
  }
}

