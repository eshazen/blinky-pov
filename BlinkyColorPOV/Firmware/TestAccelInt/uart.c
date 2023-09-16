#include <stdio.h>
#include "uart.h"


void USART0Init(void)
{
  // Set baud rate
  UBRR0H = (uint8_t)(UBRR_VALUE>>8);
  UBRR0L = (uint8_t)UBRR_VALUE;
  // Set frame format to 8 data bits, no parity, 1 stop bit
  UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
  //enable transmission and reception
  UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
}

int USART0SendByte(char u8Data, FILE *stream)
{
  if(u8Data == '\n')
    {
      USART0SendByte('\r', stream);
    }
  //wait while previous byte is completed
  while(!(UCSR0A&(1<<UDRE0))){};
  // Transmit data
  UDR0 = u8Data;
  return 0;
}


uint8_t USART0ReceiveByte()
{
  // Wait for byte to be received
  while(!(UCSR0A&(1<<RXC0))){};
  // Return received data
  return UDR0;
}



void USART0GetString( char *buffer, int max)
{
  int n = 0;
  char *p = buffer;
  uint8_t c;

  while( 1) {
    c = USART0ReceiveByte();
    putchar( c);
    if( c == '\n') {
      *p++ = '\0';
      return;
    }
    if( c == '\b' && p > buffer) {
      putchar('\b');
      --p;
    } else {
      *p++ = c;
    }
    
  }
  
}
