
// candle for Adafruit NeoPixel
// 8 pixel version
// by Tim Bartlett, December 2013

#include <util/delay.h>
#include "neopixel.h"

#define NPIX 9

//#define constrain(x,a,b) (((x)>=(a)&&(x)<=(b))?(x):((x)<(a)?(a):(b)))

int constrain( int x, int a, int b) {
  if( x < a)
    return a;
  if( x > b)
    return b;
  return x;
}

void delay( int ms) {
  int i;
  for( i=0; i<ms; i++)
    _delay_ms(1);
}

void burn(int f);
void flicker(int f);
void flutter(int f);
void on(int f);

// color variables: mix RGB (0-255) for desired yellow
//int redPx = 255;
//int grnHigh = 100;
//int bluePx = 10;

int redPx = 200;
int grnHigh = 100;
int bluePx = 5;

// animation time variables, with recommendations
//int burnDepth = 6; //how much green dips below grnHigh for normal burn - 
//int flutterDepth = 20; //maximum dip for flutter
//int cycleTime = 120; //duration of one dip in milliseconds

int burnDepth = 80; //how much green dips below grnHigh for normal burn - 
int flutterDepth = 800; //maximum dip for flutter
int cycleTime = 120; //duration of one dip in milliseconds


// pay no attention to that man behind the curtain
int fDelay;
int fRep;
int flickerDepth;
int burnDelay;
int burnLow;
int flickDelay;
int flickLow;
int flutDelay;
int flutLow;

// set all LEDs to same color
void set_all( uint8_t r, uint8_t g, uint8_t b) {
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  sendPixel( r, g, b);
  show();
}


void setup() {

  flickerDepth = (burnDepth + flutterDepth) / 2.4;
  burnLow = grnHigh - burnDepth;
  burnDelay = (cycleTime / 2) / burnDepth;
  flickLow = grnHigh - flickerDepth;
  flickDelay = (cycleTime / 2) / flickerDepth;
  flutLow = grnHigh - flutterDepth;
  flutDelay = ((cycleTime / 2) / flutterDepth);
  
  ledsetup();
  set_all( 25, 25, 25);
  _delay_ms(500);
  set_all( 0, 0, 0);
  _delay_ms(500);
}

// In loop, call CANDLE STATES, with duration in seconds
// 1. on() = solid yellow
// 2. burn() = candle is burning normally, flickering slightly
// 3. flicker() = candle flickers noticably
// 4. flutter() = the candle needs air!

void loop() {
   burn(10);
   flicker(5);
   burn(8);
   flutter(6);
   burn(3);
   on(10);
   burn(10);
   flicker(10);
}

int main() {
  setup();
  while( 1) {
    loop();
  }
}

// basic fire funciton - not called in main loop
void fire(int grnLow) {
  for (int grnPx = grnHigh; grnPx > grnLow; grnPx--) {
    int halfGrn = grnHigh - ((grnHigh - grnPx) / 2);
    int darkGrn = grnPx - 70;
    darkGrn = constrain(darkGrn, 5, 255);
    sendPixel( redPx-180, darkGrn, 0);
    sendPixel( redPx-180, darkGrn, 0);
    sendPixel( redPx-120, grnPx-50, bluePx-5);
    sendPixel( redPx-60, grnPx-35, bluePx-2);
    sendPixel( redPx, grnPx, bluePx);
    sendPixel( redPx, grnPx, bluePx);
    sendPixel( redPx, halfGrn, bluePx);
    sendPixel( redPx, grnHigh, bluePx);
    show();
    delay(fDelay);
  }  
  for (int grnPx = grnLow; grnPx < grnHigh; grnPx++) {
    int halfGrn = grnHigh - ((grnHigh - grnPx) / 2);
    int darkGrn = grnPx-70;
    darkGrn = constrain(darkGrn, 5, 255);
    sendPixel( redPx-180, darkGrn, 0);
    sendPixel( redPx-180, darkGrn, 0);
    sendPixel( redPx-120, grnPx-50, bluePx-5);
    sendPixel( redPx-60, grnPx-35, bluePx-2);
    sendPixel( redPx, grnPx, bluePx);
    sendPixel( redPx, grnPx, bluePx);
    sendPixel( redPx, halfGrn, bluePx);
    sendPixel( redPx, grnHigh, bluePx);
    show();
    delay(fDelay);
  }
}

// fire animation
void on(int f) {
  fRep = f * 1000;
  int grnPx = grnHigh - 6;
  sendPixel( redPx-180, grnPx-70, 0);
  sendPixel( redPx-180, grnPx-70, 0);
  sendPixel( redPx-120, grnPx-50, bluePx-5);
  sendPixel( redPx-60, grnPx-35, bluePx-2);
  sendPixel( redPx, grnPx, bluePx);
  sendPixel( redPx, grnPx, bluePx);
  sendPixel( redPx, grnPx, bluePx);
  sendPixel( redPx, grnHigh, bluePx);
  show();
  delay(fRep);
}

void burn(int f) {
  fRep = f * 8;
  fDelay = burnDelay;
  for (int var = 0; var < fRep; var++) {
    fire(burnLow);
  }  
}

void flicker(int f) {
  fRep = f * 8;
  fDelay = burnDelay;
  fire(burnLow);
  fDelay = flickDelay;
  for (int var = 0; var < fRep; var++) {
    fire(flickLow);
  }
  fDelay = burnDelay;
  fire(burnLow);
  fire(burnLow);
  fire(burnLow);
}

void flutter(int f) {
  fRep = f * 8;  
  fDelay = burnDelay;
  fire(burnLow);
  fDelay = flickDelay;
  fire(flickLow);
  fDelay = flutDelay;
  for (int var = 0; var < fRep; var++) {
    fire(flutLow);
  }
  fDelay = flickDelay;
  fire(flickLow);
  fire(flickLow);
  fDelay = burnDelay;
  fire(burnLow);
  fire(burnLow);
}
