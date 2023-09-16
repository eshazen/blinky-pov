#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

// offsets of things in the hex record
#define IHEX_LEN 0
#define IHEX_ADDR 1
#define IHEX_TYPE 3
#define IHEX_DATA 4

#define BUFSIZ 24

// number of ADC samples to average
// #define NAV 30
#define NAV 5

// logic threshold in ADC units (30 ~ 100mV)
#define THR 100

#define MODE_BLINKY 0
#define MODE_LOAD 1

#define ADC_CLK 0
#define ADC_DATA 1

#define CLK_VAL 1
#define DATA_VAL 2

void calibrate();
uint8_t sample_clk_data();
uint8_t wait_for_clock( uint8_t c);
uint8_t rx_byte();
int rx_ihex();
