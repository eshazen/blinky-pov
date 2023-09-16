/*
 * read and save ADC data
 */


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "sio_cmd.h"

char *sio_gets( int fd);

main()
{
  char *s;
  int fd;
  int n, d;
  
  if( (fd = sio_open("/dev/ttyUSB0")) < 0) {
    printf("Error opening serial port\n");
    exit( 1);
  }


  /* just read and display data for now */


  while( 1) {
    s = sio_gets( fd);
    sscanf( s, "%d %d", &n, &d);
    printf( "%d %d\n", n, d);
  }
  
}


/* read a string with any control/null char as terminator
   into local static buffer and return */

char *sio_gets( int fd) {
  char rch;
  int nc, res;
  static char buf[SIO_BUF_MAX];
  
  nc = 0;
  rch = ' ';
  do {
    res = read( fd, &rch, 1);
    if( res == 1) {
      buf[nc++] = rch;
      if( nc == SIO_BUF_MAX) {
	printf("BUffer overflow!\n");
	exit( 1);
      }
    }
  } while( rch >= 0x20);
  --nc;
  buf[nc] = '\0';

  return buf;
}

