#include "rpi.h"
#include "color.h"
#include "package_data.h"

const int COLOR_INCREMENT = 40;
const int COLOR_MAX = 255;
const int COLOR_MIN = 0;

uint8_t format_color(unsigned original_color, unsigned button_val) {
	assert(original_color <= COLOR_MAX && original_color >= 0);
	unsigned color = original_color + (button_val * COLOR_INCREMENT);
	if (color >= COLOR_MAX) color = color - COLOR_MAX;
	if (color <= COLOR_MIN) color = color + COLOR_MAX;
	uint8_t bit_color = color;
	return bit_color;
}

uint32_t package(Color* color, uint16_t buttons, uint8_t joystick) {
	uint8_t red = format_color(color->red, buttons >> 3);
	uint8_t green = format_color(color->green, buttons >> 2 & 1);
	uint8_t blue = format_color(color->blue, buttons >> 1 & 1);
	uint8_t joystick_on_off = ((buttons & 1) << 7) | joystick;
	color->red = red;
	color->blue = blue;
	color->green = green;
	uint32_t pack = joystick_on_off << 23 | red << 15 | green << 7 | blue;
	return pack;
}

uint16_t package_buttons(unsigned r_button, unsigned g_button, unsigned b_button, unsigned on_off_button) {
	return (r_button << 3 | g_button << 2 | b_button << 1 | on_off_button) << 7;
}