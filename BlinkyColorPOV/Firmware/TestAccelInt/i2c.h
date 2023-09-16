
// Find divisors for the UART0 and I2C baud rates
// #define BDIV (F_CPU / 100000 - 16) / 2 + 1    // Puts I2C rate just below 100kHz
#define TWI_TWBR (F_CPU / 400000 - 16) / 2 + 1    // Puts I2C rate 400kHz

#define ACC_I2C_ADR 0x98


