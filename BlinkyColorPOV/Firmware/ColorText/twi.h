/*
 * DAC test program
 */
// #define F_CPU 1000000UL  // 1 MHz

#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>

#define MAX_ITER 200

void twi_init(void);
int twi_write_bytes( uint8_t twi_addr, int len, uint8_t *dat);

