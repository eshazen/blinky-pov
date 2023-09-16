/*
 * test USART
 */

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "neopixel.h"
#include "adc.h"
#include "uart.h"


FILE usart0_str = FDEV_SETUP_STREAM(USART0SendByte, NULL, _FDEV_SETUP_WRITE);


int main (void)
{
  ledsetup();
  InitADC();
  USART0Init();
  stdout = &usart0_str;
  static char buff[10];

  uint8_t r, g, b;
  uint16_t a1, a2;

  while( 1) {
    puts("R>");
    USART0GetString( buff, 10);
    r = atoi( buff);
    puts("G>");
    USART0GetString( buff, 10);
    g = atoi( buff);
    puts("B>");
    USART0GetString( buff, 10);
    b = atoi( buff);
    
    sendPixel( g, r, b);
    sendPixel( g, r, b);
    show();
    _delay_ms(500);
  }

}


