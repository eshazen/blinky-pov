
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

  uint8_t r, g, b;
  uint16_t a1, a2;

  while( 1) {
    a1 = ReadADC( 0);
    a2 = ReadADC( 1);
    printf("%u %u\n", a1, a2);
    r = a1 / 32;
    g = a2 / 32;
    sendPixel( r, 0, 0);
    sendPixel( 0, g, 0);
    show();
    _delay_ms(500);
  }

}


