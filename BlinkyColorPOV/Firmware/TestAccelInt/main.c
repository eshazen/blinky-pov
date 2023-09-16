/*
 * test Accelerometer with interrupts
 */

#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "neopixel.h"
#include "uart.h"
#include "button.h"

#include "i2c.h"
#include "TWI_Master.h"

#define abs(x) ((x)<0?(-(x)):(x))



FILE usart0_str = FDEV_SETUP_STREAM(USART0SendByte, NULL, _FDEV_SETUP_WRITE);
void acc_print( char c, uint8_t val);

uint8_t msg[8];

void blank() {
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  sendPixel( 0, 0, 0);
  show();
}

void setfirst( uint8_t r, uint8_t g, uint8_t b) {
  sendPixel( g, r, b);
  show();
}  

int main (void)
{
  uint8_t i2c_state;
  uint8_t i;

  ledsetup();
  //  InitADC();
  USART0Init();
  BUTTON_PORT |= (1<<BUTTON_BIT);

  TWI_Master_Initialise();
  sei();

  stdout = &usart0_str;

  _delay_ms(10);

  blank();
  setfirst( 10, 0, 0);
  puts("Hello\n");

  // set register 7 (mode) to 1 to enable
  msg[0] = ACC_I2C_ADR;
  msg[1] = 7;
  msg[2] = 1;
  TWI_Start_Transceiver_With_Data( msg, 3);
  i2c_state = TWI_Get_State_Info();
  if( i2c_state != TWI_NO_STATE)
    printf("op 1: state=%02x\n", i2c_state);

  // loop reading out and printing X/Y/Z values
  while( 1) {
    // set address to 0
    msg[0] = ACC_I2C_ADR;
    msg[1] = 0;
    TWI_Start_Transceiver_With_Data( msg, 2);
    i2c_state = TWI_Get_State_Info();
    if( i2c_state != TWI_NO_STATE)
      printf("op 2: state=%02x\n", i2c_state);

    // read 3 bytes
    msg[0] = ACC_I2C_ADR + 1;
    msg[1] = 1;
    msg[2] = 1;
    msg[3] = 1;
    TWI_Start_Transceiver_With_Data( msg, 4);
    TWI_Get_Data_From_Transceiver( msg, 4);
    i2c_state = TWI_Get_State_Info();
    if( i2c_state != TWI_NO_STATE)
      printf("op 3: state=%02x\n", i2c_state);

    for( i=0; i<4; i++)
      printf("%02x ", msg[i]);
    putchar('\n');
    
    _delay_ms(1000);
  }

}


void acc_print( char c, uint8_t val) {
  putchar(' ');
  putchar( c);
  putchar( '=');
  if( val & 0x40)
    puts("ERR");
  else {
    int8_t v = (val & 0x3f) << 2;
    v /= 4;
    printf(" %3d", v);
  }
}
