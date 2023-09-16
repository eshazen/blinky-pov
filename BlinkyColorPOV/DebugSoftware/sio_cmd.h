#ifndef _SIO_CMD_H

#define SIO_BUF_MAX 10000        

#define BAUDRATE B9600
// #define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int sio_open( char *dev);
char *sio_cmd( int fd, char *s);
void dump_string( char *s);
int sio_open_trace( char *dev, char *tracef);

#define _SIO_CMD_H
#endif

