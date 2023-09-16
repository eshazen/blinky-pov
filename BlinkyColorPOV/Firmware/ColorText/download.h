#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/pgmspace.h>

// length of one code in bits
#define CODE_LEN 10
#define CODE_MASK ((1<<CODE_LEN)-1)

typedef uint16_t code_t;


// offsets of things in the data record
#define IREC_SYNC0 0
#define IREC_SPEED 1
#define IREC_LEN 2
#define IREC_DATA 3

// number of extra bytes besides message
#define IREC_EXTRA 4

#define BUFSIZ 24
#define MAX_MSG (BUFSIZ-5)

// number of ADC samples to average
// #define NAV 30
#define NAV 5

// logic threshold in ADC units (30 ~ 100mV)
#define THR 50

#define MODE_BLINKY 0
#define MODE_LOAD 1

#define ADC_CLK 0
#define ADC_DATA 1

#define CLK_VAL 1
#define DATA_VAL 2

void calibrate();
uint8_t sample_clk_data();
uint8_t wait_for_clock();
code_t rx_code();
int rx_irec();
void defmsg();
uint8_t check_sum();
