
#include <stdint.h>

// Find divisors for the UART0 and I2C baud rates
// #define BDIV (F_CPU / 100000 - 16) / 2 + 1    // Puts I2C rate just below 100kHz
#define BDIV (F_CPU / 400000 - 16) / 2 + 1    // Puts I2C rate 400kHz

#define ACC_I2C_ADR 0x98

void i2c_init(uint8_t bdiv);
uint8_t i2c_io(uint8_t device_addr, uint8_t *ap, uint16_t an, 
               uint8_t *wp, uint16_t wn, uint8_t *rp, uint16_t rn);

