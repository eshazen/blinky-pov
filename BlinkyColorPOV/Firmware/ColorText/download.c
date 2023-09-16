//
// download a blinky record
// 

// #define DEBUG

#include "download.h"
#include "button.h"
#include <stdio.h>
#include "uart.h"

// test message
const code_t default_msg[] PROGMEM = {
    0x2aa,0x4,0x6,0x68,0x65,0x6c,0x6c,0x6f,0x41,0x0 // "Hello!" (all red)
  // 0x2aa,0x4,0x9,0x64,0xb2,0xe9,0x133,0x163,0x1af,0x1ec,0x1ac,0x141,0x0
  // 0x2aa,0x4,0x6,0xe2,0xec,0xe9,0xee,0xeb,0xf9,0x0
  // 0x2aa,0x4,0x2,0x61,0x62,0x166 // "AB"
};

// variables for blinky receiver
static int16_t base_d, base_c;
static int32_t sum_d, sum_c;

// loop variable
static uint16_t k;
static int16_t a;

// number of elements in data record
uint8_t irec_nd;

// buffer for data records
code_t irec[BUFSIZ];

// flag:  button pressed, do reset
static uint8_t do_reset;

// keep track of current clock edge
static uint8_t clk_edge;

void setfirst( uint8_t r, uint8_t g, uint8_t b);

void defmsg() {
  memcpy_P( irec, default_msg, sizeof(default_msg) );
  irec_nd  = irec[IREC_LEN];
}

// measure mean black level
void calibrate() {
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

#ifdef DEBUG
      printf("base_d=%d base_c=%d\n", base_d, base_c);
#endif
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
  
  uint32_t dc, cc;

  dc = base_d - (sum_d / NAV);
  cc = base_c - (sum_c / NAV);

  if( (base_d - (sum_d / NAV)) >= THR)
    rv |= DATA_VAL;
  if( (base_c - (sum_c / NAV)) >= THR)
    rv |= CLK_VAL;

  return rv;
}


//
// wait for clock state change or button press
// return data state, set do_reset on button press
// use 'clk_edge' to determine edge to match
//
uint8_t wait_for_clock()
{
  uint8_t s;

  do {
    s = sample_clk_data();
    if( button_pressed()) {
      do_reset = 1;
      return 0;
    }
  } while( (s & CLK_VAL) != clk_edge);

  clk_edge ^= CLK_VAL;		/* toggle clock edge */

  return( (s & DATA_VAL) != 0);
}

// receive blinky code
code_t rx_code() {

  code_t bit;
  code_t code;
  uint8_t i;
  uint8_t d;

  bit = (1<<CODE_LEN-1);
  code = 0;

  for( i=0; i<CODE_LEN; i++) {
    d = wait_for_clock();
    if( do_reset) return 0;
    if( d)
      code |= bit;
    bit >>= 1;
  }

#ifdef DEBUG
  printf("%03x ", code);
#else
  putchar('.');
#endif  
  
  return code;
}

// receive an message record into buffer
// return type or -1 on failure
int rx_irec() {

  code_t* p = irec;
  code_t rsum, csum;
  uint8_t i;

  do_reset = 0;
  clk_edge = CLK_VAL;			/* first clock edge polarity ('1')*/

  setfirst( 0, 0, 10);

  // receive minimum record <sync> <sync> <length>
  *p++ = rx_code();
  setfirst( 0, 10, 0);
  *p++ = rx_code();
  setfirst( 0, 10, 10);
  *p++ = rx_code();
  setfirst( 10, 10, 0);

  irec_nd  = irec[IREC_LEN];
  if( irec_nd == 0) {		/* ignore length 0 records for now */
#ifdef DEBUG
    printf("\nzero\n");
#endif
    return -1;
  }

  if( irec_nd < 1 || irec_nd > MAX_MSG) { /* length invalid */
#ifdef DEBUG
    printf("\nlen\n");
#endif
    return -1;
  }

  for( i=0; i<irec_nd; i++) {	/* get message */
    *p++ = rx_code();  
    if( do_reset) return -2;
    setfirst( 0, 0, (i & 1) << 4);
  }

  rsum = rx_code();		/* get checksum */
  *p++ = rsum;			/* store checksum at end */
  if( do_reset) return -2;

  return check_sum();
}


uint8_t check_sum()
{
  code_t rsum, csum;
  uint8_t i, n;
  n = irec[IREC_LEN] + IREC_EXTRA;
  csum = 0;
  for( i=0; i<n-1; i++)
    csum += irec[i];
  csum &= CODE_MASK;
#ifdef DEBUG
  printf("csum=%04x rsum=%04x\n", csum, irec[n-1]);
#endif
  if( csum == irec[n-1])
    return 0;
  else
    return 1;
}
