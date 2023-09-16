#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

/*
 * header file for simplfied neopixels library
 * courtesy of josh.com
 */

#define PIXEL_PORT  PORTD  // Port of the pin the pixels are connected to
#define PIXEL_DDR   DDRD   // Port of the pin the pixels are connected to
#define PIXEL_BIT   7      // Bit of the pin the pixels are connected to


#define T1H  900    // Width of a 1 bit in ns
#define T1L  600    // Width of a 1 bit in ns

#define T0H  400    // Width of a 0 bit in ns
#define T0L  900    // Width of a 0 bit in ns

#define RES 6000    // Width of the low gap between bits to cause a frame to latch

#define NS_PER_SEC (1000000000L)          // Note that this has to be SIGNED since we want to be
                                          // able to check for negative values of derivatives
#define CYCLES_PER_SEC (F_CPU)
#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )
#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )

#define bitRead(w,b) (((w)>>(b))&1)

#define bool uint8_t

void sendBit( bool bitVal );
void sendByte( unsigned char byte );
void ledsetup();
void sendPixel( unsigned char r, unsigned char g , unsigned char b );
void show();
