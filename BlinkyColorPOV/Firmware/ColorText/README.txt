
First attempt at full version.
Character-based, ASCII codes 32-95 supported

Timing issues:

Optimized I2C for accel- 400kHz clock, read only one byte (X)
A read takes about 130us.  Could be interrupt driven?

Neopixel timing is 24 bits '1' is 1.5uS, '0' is 1.3uS.
So 1.5uS * 24 * 8 is about 300uS for an update of all 8 LEDs

Hand sweep of blinky is ~ 1Hz or 500ms per direction.
If we allow 20 characters at average with of 5 pixels, that's
100 pixels or 5ms per pixel.  Should work down to 1ms per pixel,
For total error less than 1/2 character (2 pixels) that's
2% of 5ms or 100us.  So pixel timing should be scheduled in 
100uS ticks.

Interrupts must be disabled during neopixel output, though
the timer can still run.

Timer setup for non-interrupt ops:
TCCR1A = $00
TCCR1B = $0x where x is $04 with prescale 1...5 for 1, 8, 64...1024

Proposed implementation:

Setup timer1 for 128uS counting (prescale=1024)

Measure time between zero-crossings (200-700ms or so)
Calculate pixel update period in 100us uinits.

100us loop:
  if time >= next_pixel:
    run_pixel_update
  if time >= next_accel:
    run_accel_update

  




----------------------------------------

Download protocol:

9 bit tokens
  0x55 0x1aa        (sync)
  <nchars>          (character count)
  <code> <code>..   (low 6 bits are character code, upper 3 are color)
  <checksum>        (such that sum of all = 0)

Character codes start with 0=ascii 32 (space), 1=ascii 33 "!" etc
through 63=ascii 95 (unused code)

Sent as DDR bit-stream as follows using blinking squares on monitor.
Maximum achievable rate with a typical browser/monitor seems to
be about 10 bits/s.

white    +----+    +----+    +----+    +----+    +----+    +----+    
CLOCK    |    |    |    |    |    |    |    |    |    |    |    | 
black----+    +----+    +----+    +----+    +----+    +----+    +--

white  +----+    +----+    +----+    +----+    +----+    +----+    
DATA   | 8  | 7  | 6  | 5  | 4  | 3  | 2  | 1  | 0  | 8  | 7  | 6  
black--+    +----+    +----+    +----+    +----+    +----+    +----

One thing to note is that since the transfer is DDR and the word
length is odd, that the first word's MSB is on a rising clock edge,
while next is on a falling clock edge etc.

