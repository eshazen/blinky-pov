#include <avr/io.h>

#define BUTTON_PORT PORTD
#define BUTTON_BIT 6
#define BUTTON_PIN PIND

#define button_pressed() (!(BUTTON_PIN&(1<<BUTTON_BIT)))
