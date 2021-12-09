#ifndef __PACKAGE_DATA_H__
#define __PACKAGE_DATA_H__

#include "rpi.h"
#include "color.h"

uint8_t format_color(unsigned original_color, unsigned button_val);

uint32_t package(Color* color, uint16_t buttons, uint8_t joystick);

uint16_t package_buttons(unsigned r_button, unsigned g_button, unsigned b_button, unsigned on_off_button);

#endif