/*
 * main.c - BlinkyPov/AVR downloader
 *
 * Mods:
 * 2012-11-25, esh -- initial working version.  Supports download of one font-based
 * message to EEPROM.  Displays message on power-up, waits for button press.
 * 2012-11-26, esh -- port for Rev2 board
 *
 * TODO:
 * Implement pixel-based messages, multiple messages
 */

#define REV2

#define USE_EEPROM

// #define USE_UART

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <string.h>

#include "font_table.h"
#define FONT_WIDTH 5	/* bytes per character in font */

#define LED_PORTA 0
#define LED_PORTB 1

// LED pin assignments
const uint8_t led_port_code[] PROGMEM = {
#ifdef REV2
  // these are valid for Rev4 layout for 2014
  LED_PORTA, 0,
  LED_PORTA, 1,
  LED_PORTA, 2,
  LED_PORTA, 3,
  LED_PORTA, 7,
  LED_PORTB, 0,
  LED_PORTB, 1,
  LED_PORTB, 2
#else
  LED_PORTA, 0,
  LED_PORTA, 1,
  LED_PORTA, 2,
  LED_PORTA, 3,
  LED_PORTA, 4,
  LED_PORTA, 5,
  LED_PORTB, 3,
  LED_PORTB, 2
#endif
};

#define led_bit(n) (pgm_read_byte(&(led_port_code[((n)<<1)+1])))
#define led_port(n) (pgm_read_byte(&(led_port_code[((n)<<1)])))

static void inline setup_led_ports()
{
  uint8_t i;
  for( i=0; i<8; i++) {
    if( led_port(i) == LED_PORTA)
      DDRA |= _BV(led_bit(i));
    else
      DDRB |= _BV(led_bit(i));
  }
}

// create masks for ports A/B for LEDs specified
// PORT A = low byte
// PORT B = high byte
static uint16_t led_mask( uint8_t leds)
{
  uint16_t v = 0;
  uint8_t i;
  uint8_t b = 1;

  for( i=0; i<8; i++) {
    if( leds & b) {
      if( led_port(i) == LED_PORTA)
	v |= _BV(led_bit(i));
      else
	v |= (_BV(led_bit(i)) << 8);
    }
    b <<= 1;
  }
  return v;
}

// toggle LEDs per mask
static void led_flip( uint8_t flip_mask)
{
  uint16_t v = led_mask( flip_mask);
  PORTA ^= (v & 0xff);
  PORTB ^= ((v>>8) & 0xff);
}

// turn on LEDs per mask
static void led_on( uint8_t on_mask)
{
  uint16_t v = led_mask( on_mask);
  PORTA |= (v & 0xff);
  PORTB |= ((v>>8) & 0xff);
}

// turn off LEDs per mask
static void led_off( uint8_t off_mask)
{
  uint16_t v = led_mask( off_mask);
  PORTA &= ~(v & 0xff);
  PORTB &= ~((v>>8) & 0xff);
}

// set LEDs per mask
static void led_set( uint8_t set_mask)
{
  led_off( 0xff);
  led_on( set_mask);
}




#ifdef USE_UART
#ifdef REV2
#define UART_PORT PORTA
#define UART_DDR DDRA
#define UART_TX_BIT _BV(5)
#else
#define UART_PORT PORTB
#define UART_DDR DDRB
#define UART_TX_BIT _BV(1)
#endif
#endif

#ifdef REV2
// ok for Rev4 layout
#define SW_PORT PORTA
#define SW_PIN PINA
#define SW_BIT 5
#else
#define SW_PORT PORTB
#define SW_PIN PINB
#define SW_BIT 0
#endif
#define SW_MASK _BV(SW_BIT)

#ifdef REV2
// ok for Rev4 layout
#define ADC_DATA 4
#define ADC_CLK 6
#else
#define ADC_DATA 6
#define ADC_CLK 7
#endif

#define CLK_VAL 1
#define DATA_VAL 2

// buffer for intel hex records (size 16)
#define BUFSIZ 24
static uint8_t ihex[BUFSIZ];

// buffer for EEPROM image
#define EEBUFSIZ 128
static uint8_t eebuf[EEBUFSIZ];

static uint8_t eep_nd;

// offsets of things in the hex record
#define IHEX_LEN 0
#define IHEX_ADDR 1
#define IHEX_TYPE 3
#define IHEX_DATA 4

// number of bytes in latest hex record
static uint8_t ihex_nd;

// address of latest hex record
static uint16_t ihex_addr;

// number of ADC samples to average
#define NAV 30

// logic threshold in ADC units (30 ~ 100mV)
#define THR 30

// variables for blinky receiver
static int16_t base_d, base_c;
static int32_t sum_d, sum_c;

// loop variable
static uint16_t k;
static int16_t a;


// flag:  button pressed, do reset
static uint8_t do_reset;

// global mode
static uint8_t mode;

#define MODE_BLINKY 0
#define MODE_LOAD 1



#ifdef USE_UART
// send a character to UART at 9600 baud using software delay
#define BAUD_DELAY_FUDGE 10
#define BAUD_DELAY_US ((1000000/9600)-BAUD_DELAY_FUDGE)

static void inline send_uart_bit( uint8_t b)
{
  if( b)
    UART_PORT |= UART_TX_BIT;
  else
    UART_PORT &= ~UART_TX_BIT;
  _delay_us( BAUD_DELAY_US);
}

// send byte to UART
static void uart_putc( uint8_t c)
{
  send_uart_bit( 0);		/* start bit */
  send_uart_bit( c & 1);
  send_uart_bit( c & 2);
  send_uart_bit( c & 4);
  send_uart_bit( c & 8);
  send_uart_bit( c & 0x10);
  send_uart_bit( c & 0x20);
  send_uart_bit( c & 0x40);
  send_uart_bit( c & 0x80);
  send_uart_bit( 1);		/* stop bit */
}

// convert nibble to hex
#define nibble_to_hex(n) ((n)>9?((n)+('A'-10)):((n)+'0'))

// send hex byte to UART
static void uart_hex2( uint8_t c)
{
  uart_putc( nibble_to_hex( (c>>4)&0xf));
  uart_putc( nibble_to_hex( c & 0xf));
}

// send hex word to UART
static void uart_hex4( uint16_t w) {
  uart_hex2( w>>8);
  uart_hex2( w);
}

// send newline sequence
static void inline crlf()
{
  uart_putc( 0x0d);
  uart_putc( 0x0a);
}
#endif

/*
 * Do all the startup-time peripheral initializations.
 */
static void ioinit(void)
{
  setup_led_ports();
#ifdef USE_UART
  UART_DDR |= UART_TX_BIT;
#endif
  SW_PORT |= SW_MASK;		/* pull-up on switch */
  ADCSRA |= _BV(ADEN);		/* enable ADC, default settings */
  //  ADCSRB |= _BV(ADLAR);		/* left adjust result */
  ADMUX = 7;
}



uint16_t read_adc( uint8_t ch)
{
  uint8_t lo, hi;

  ADMUX = ch;
  ADCSRA |= _BV(ADSC);
  loop_until_bit_is_clear( ADCSRA, ADSC);
  lo = ADCL;
  hi = ADCH;
  
  return( (hi<<8) | lo);
}


// sample the light sensor once (with averaging)
// return bit 0 = clk state  bit 1 = data state
uint8_t sample_clk_data() {
  uint8_t rv;

  sum_d = sum_c = 0;
  for( k=0; k<NAV; k++) {
    a = read_adc( ADC_DATA);
    sum_d += a;
    a = read_adc( ADC_CLK);
    sum_c += a;
  }

  rv = 0;
  if( (base_d - (sum_d / NAV)) >= THR)
    rv |= DATA_VAL;
  if( (base_c - (sum_c / NAV)) >= THR)
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
    if( bit_is_clear( SW_PIN, SW_BIT)) {
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
  
  //  led_flip( 0x80);		/* toggle LED8 */
  led_set( byte);
  
  return byte;
}

// receive an intel hex record into buffer
// return type or -1 on failure
int rx_ihex() {

  uint8_t* p = ihex;
  uint8_t i, n, c, s;

  n  = rx_byte();	/* get length */
  if( n == 0)		/* ignore length 0 records for now */
    return 0;

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



// delay one column with code 0..15
// about 0.5 ms per unit
void delay_column( uint8_t d)
{
  if( d < 15)
    _delay_loop_2( d * 128);
  else
    _delay_ms( 500);
}

// display messages if any in EEPROM
// returns:
// 0 - no messages to display
// 1 - button pressed
//
int show_messages() {
  uint8_t i, t, n, j, speed, c;

  eep_nd = eeprom_read_byte( 0); /* get EEPROM data size */

  // display EEPROM buffer size on low LEDs, high one on
  led_set( (eep_nd&0x7f) | 0x80);
  _delay_ms( 500);
  // turn off all LEDs
  led_off( 0xff);
  _delay_ms( 500);

  if( eep_nd < 4 || eep_nd > EEBUFSIZ)
    return 0;

  // goto blinky mode until button pressed
  // copy data from the EEPROM
  eeprom_read_block( eebuf, (void *)1, eep_nd);

#ifdef USE_UART
  // dump eeprom buffer
  uart_putc('=');
  uart_hex2( eep_nd);
  crlf();
  for( i=0; i<eep_nd; i++) {
    uart_hex2( eebuf[i]);
    crlf();
  }
  uart_putc( '*');
  crlf();
#endif

  n = eebuf[2];		/* length of first message */
  speed = (eebuf[1] >> 2) & 15; /* speed code 0-15, 7=default */

  // loop until button is pressed
  while( bit_is_set( SW_PIN, SW_BIT) ) {

    // space between messages
    for( j=0; j<10; j++)
      delay_column( speed);

    // loop over message
    for( i=0; i<n; i++) {
#ifdef USE_UART
      uart_putc( 'd');
      uart_hex2( i);
      uart_putc( '=');
#endif
      // look up character
      t = eebuf[3+i] * FONT_WIDTH;
      if( t < sizeof(font_table)) {
	for( j=0; j<FONT_WIDTH; j++) {
	  c = pgm_read_byte(&(font_table[t+j]));
	  led_set( c);
#ifdef USE_UART
	  uart_hex2( c);
	  uart_putc( ' ');
#endif
	  delay_column( speed);
	}
      } else {		// invalid code probably a space
	// delay for three extra columns for space
#ifdef USE_UART
	uart_putc('S');
#endif
	delay_column( speed);
	delay_column( speed);
	delay_column( speed);
      }
      led_off( 0xff);
      delay_column( speed); /* two blank columns between characters */
      delay_column( speed);
#ifdef USE_UART
      crlf();
#endif
    }
  }

  // fall out if button pressed
  return 1;
}


int main(void)
{
  uint8_t i, j, n, c;
  uint8_t speed;
  uint8_t t;
  uint8_t nblk;

  ioinit();
  // test LEDs
  led_off( 0xff);
  _delay_ms( 100);
  led_on( 0xff);
  _delay_ms( 100);
  led_off( 0xff);  
  for( i=0; i<8; i++) {
    _delay_ms(100);
    led_on( 1<<i);
  }
  _delay_ms(100);
  led_off( 0xff);


  // Power-up:   check for EEPROM message and display it indefinitely until
  // the button is pressed
  i = show_messages();

  do_reset = 1;

  while( 1) {

    // see if button is pressed
    // if so, recalibrate threshold, reset bit counter
    if( bit_is_clear( SW_PIN, SW_BIT) || do_reset) {

#ifdef USE_UART
      uart_putc( 'R');
      crlf();
#endif
      led_set( 0x80);

      // measure background level
      sum_d = sum_c = 0;
      for( k=0; k<NAV; k++) {
	a = read_adc( ADC_DATA);
	sum_d += a;
	a = read_adc( ADC_CLK);
	sum_c += a;
	_delay_ms(1);
      }
      base_d = sum_d / NAV;
      base_c = sum_c / NAV;

      // now wait for button to be released, wait a while

      // loop while button is still pressed
      while( bit_is_clear( SW_PIN, SW_BIT))
	;

      _delay_ms( 1000);
      // turn off all LEDs
      led_off( 0xff);

      do_reset = 0;
      ihex_nd = 0;
      memset( eebuf, 0xab, EEBUFSIZ);
    } // if( button pressed)...

    // look for intel hex records
    eep_nd = 0;

    while( !do_reset) {

      t = rx_ihex();
      ++nblk;
      
      if( do_reset)
	break;

      if( t < 0) {		/* error blink */
	for( i=0; i<5; i++) {
	  led_set( 0xf);
	  _delay_ms(100);
	  led_set( 0);
	  _delay_ms(100);
	}
      }

      if( t == 0) {		/* end-of-file record */
#ifdef USE_EEPROM
	if( eep_nd) {
	  // copy EEPROM RAM buffer to EEPROM
	  eeprom_write_byte( (void *)0, eep_nd); /* first byte is size */
	  eeprom_write_block( eebuf, (void *)1, eep_nd); /* then the data */
	}
#endif	
	break;
      }

      if( t == 6) {		/* EEPROM message */
	i = ihex_addr + ihex[IHEX_LEN]; /* top address */
	// copy data to EEPROM buffer in RAM

	if( ihex_addr < 128 && i < EEBUFSIZ) {
	  memcpy( &eebuf[ihex_addr], &ihex[IHEX_DATA], ihex[IHEX_LEN]);
	  if( i > eep_nd)
	    eep_nd = i;
	}
      }

    }

    // display messages if any, wait for button press
    show_messages();

  }

}
