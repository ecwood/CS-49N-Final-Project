#ifndef __PACKAGE_DATA_H__
#define __PACKAGE_DATA_H__

#include "rpi.h"
#include "color.h"

uint8_t format_color(unsigned original_color, unsigned button_val);

uint32_t package(Color* color, unsigned r_button, unsigned g_button, unsigned b_button, unsigned on_off_button, uint8_t joystick);

#endif