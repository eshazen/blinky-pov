
//Blinky.c

#include <avr/io.h>
#include <avr/delay.h>

int main (void)
{
  DDRB = 1;			/* PB0 is output */
  PORTD |= (1<<6);		/* PD6 gets pull-up */

  while(1) {
    if( PIND & (1<<6)) {
      PORTB = 1;
      _delay_ms(500);
      PORTB = 0;
      _delay_ms(500);
    }
  }
}
