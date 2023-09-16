
//
// generate animated GIF for blinky light download
//

#include <stdio.h>
#include <string.h>

#include "AnimatedGifSaver.h"

#define TICK 0.04

// ...frames sizes
const int  SX=200;
const int  SY=100;

Byte frame00[SX*SY*3];
Byte frame01[SX*SY*3];
Byte frame10[SX*SY*3];
Byte frame11[SX*SY*3];

int main( int argc, char *argv[])
{
  char *bits;

  if( argc > 1)
    bits = argv[1];
    
  printf("Encoding: %s\n", bits);

  // setup the frames
  for( int x=0; x<SX; x++) {
    for( int y=0; y<SY; y++) {
 
      int row = y*SX*3;
      int col = x*3;

      if( x > SX/2) { 		// right half, data
	frame00[row+col+0] = 0;
	frame00[row+col+1] = 0;
	frame00[row+col+2] = 0;
 
	frame01[row+col+0] = 0xff;
	frame01[row+col+1] = 0xff;
	frame01[row+col+2] = 0xff;

	frame10[row+col+0] = 0;
	frame10[row+col+1] = 0;
	frame10[row+col+2] = 0;
 
	frame11[row+col+0] = 0xff;
	frame11[row+col+1] = 0xff;
	frame11[row+col+2] = 0xff;
      } else {			// left half
	frame00[row+col+0] = 0;
	frame00[row+col+1] = 0;
	frame00[row+col+2] = 0;
 
	frame01[row+col+0] = 0;
	frame01[row+col+1] = 0;
	frame01[row+col+2] = 0;

	frame10[row+col+0] = 0xff;
	frame10[row+col+1] = 0xff;
	frame10[row+col+2] = 0xff;
 
	frame11[row+col+0] = 0xff;
	frame11[row+col+1] = 0xff;
	frame11[row+col+2] = 0xff;

      }

    }
  }

  // set 1 pixel to fix pallette bug
  memset( frame00, 0xff, 3);

  AnimatedGifSaver saver(SX,SY);
  
  saver.AddFrame( frame00, 1.0); // first frame, 3s

  for( int i=0; i<strlen(bits); i+=2) {
    printf("Pair %d/%d\n", i, strlen(bits));

    // assert data with clock low
    if( bits[i] == '1')
      saver.AddFrame( frame01, TICK);
    else
      saver.AddFrame( frame00, TICK);
    // now same data with clock high
    if( bits[i] == '1')
      saver.AddFrame( frame11, TICK);
    else
      saver.AddFrame( frame10, TICK);
    
    // assert new data with clock high
    if( bits[i+1] == '1')
      saver.AddFrame( frame11, TICK);
    else
      saver.AddFrame( frame10, TICK);

    // same data with clock low
    if( bits[i+1] == '1')
      saver.AddFrame( frame01, TICK);
    else
      saver.AddFrame( frame00, TICK);
  }

  saver.Save("test.gif");
}
