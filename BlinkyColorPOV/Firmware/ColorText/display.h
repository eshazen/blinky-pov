#ifndef _DISPLAY_H_INCLUDED
#define _DISPLAY_H_INCLUDED

#include "download.h"
#include "button.h"

void msg_to_pixels();
void display_column( uint8_t idx);
uint8_t* lookup_font( uint8_t ch);

#endif
