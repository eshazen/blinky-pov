
/*
Example of usage of AnimatedGifSaver class.
Saves a small looped 0-1-2-3 animation in file "0123.gif".
*/

#include "AnimatedGifSaver.h"


// red and white RGB pixels
#define R 255,0,0
#define W 255,255,255


// Lets define a few frames for this little demo...

// ...red and white RGB pixels
#define R 255,0,0
#define W 255,255,255

// ...frames sizes
const int  SX=5;
const int  SY=7;

// ...and, the frames themselves
// (note: they are defined bottom-to-top (a-la OpenGL) so they appear upside-down).

Byte frame0[SX*SY*3] ={
  W,W,W,W,W,
  W,W,R,W,W,
  W,R,W,R,W,
  W,R,W,R,W,
  W,R,W,R,W,
  W,W,R,W,W,
  W,W,W,W,W,
};

Byte frame1[SX*SY*3] ={
  W,W,W,W,W,
  W,W,R,W,W,
  W,W,R,W,W,
  W,W,R,W,W,
  W,R,R,W,W,
  W,W,R,W,W,
  W,W,W,W,W,
};

Byte frame2[SX*SY*3]={
  W,W,W,W,W,
  W,R,R,R,W,
  W,R,W,W,W,
  W,W,R,W,W,
  W,W,W,R,W,
  W,R,R,W,W,
  W,W,W,W,W,
};

Byte frame3[SX*SY*3]={
  W,W,W,W,W,
  W,R,R,W,W,
  W,W,W,R,W,
  W,W,R,W,W,
  W,W,W,R,W,
  W,R,R,W,W,
  W,W,W,W,W,
};

int main()
{
  AnimatedGifSaver saver(SX,SY);
  
  saver.AddFrame(frame0,3.0); // first frame: three secs
  saver.AddFrame(frame1,1.5); // first frame: one and half secs
  saver.AddFrame(frame2,1.5); // first frame: one and half secs
  saver.AddFrame(frame3,1.5); // first frame: one and half secs

  saver.Save("0123.gif");
}
