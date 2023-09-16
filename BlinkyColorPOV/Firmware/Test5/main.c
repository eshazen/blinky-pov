/*
 * initial test of blinky downloading
 * code ripped from previous ATTiny version so may not work as-is
 * UART output for debugging
 */

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "neopixel.h"
#include "adc.h"
#include "uart.h"
#include "download.h"

#include "button.h"

FILE usart0_str = FDEV_SETUP_STREAM(USART0SendByte, NULL, _FDEV_SETUP_WRITE);

extern uint8_t ihex_nd;
extern uint8_t ihex[BUFSIZ];

static uint16_t c, d;

static int8_t rc;
static int8_t n;

void blank() {
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  show();
}

void setfirst( uint8_t r, uint8_t g, uint8_t b) {
  sendPixel( g, r, b);
  show();
}  

int main (void)
{
  ledsetup();			/* setup output for Neopixels */
  InitADC();			/* initialize on-chip ADC */
  USART0Init();			/* initialize USART for 9600 8N1 */
  BUTTON_PORT |= (1<<BUTTON_BIT); /* pull-up on "PROG" button */

  stdout = &usart0_str;		/* connect UART to stdout */

  _delay_ms(10);		/* wait for Neopixels to boot */

  blank();
  setfirst( 10, 0, 0);
  puts("Hello\n");
  _delay_ms( 1000);
  sendPixel( 0, 10, 0);		/* dim red and green */
  sendPixel( 0, 0, 10);
  sendPixel( 10, 0, 0);
  show();
  _delay_ms( 1000);
  sendPixel( 0, 0, 10);		/* dim red and green */
  sendPixel( 10, 0, 0);
  sendPixel( 0, 10, 00);
  show();
  _delay_ms( 1000);

  while( 1) {
    sendPixel( 10, 0, 0);		/* dim red and green */
    sendPixel( 0, 10, 0);
    sendPixel( 0, 0, 10);
    show();

    while( !button_pressed())	/* wait for button press */
      ;

    setfirst( 0, 10, 10);
    _delay_ms( 500);
    setfirst( 0, 0, 10);

    putchar('*');
    calibrate();
    sendPixel( 0, 10, 10);
    sendPixel( 0, 10, 0);
    sendPixel( 10, 0, 0);
    show();

    do {
      rc = rx_ihex();
      printf("\ntype=%d len=%d", rc, ihex_nd);
      if( button_pressed())
	_delay_ms( 500);

       if( rc > 0) {
 	for( n=0; n<ihex_nd+5; n++)
 	  printf(" %d", ihex[n]);
 	putchar('\n');
 	setfirst( 0, 10, 0);
       } else {
 	puts("Error");
 	setfirst( 10, 0, 0);
       }

    } while( n > 0);
    
    _delay_ms( 500);
  }

}


