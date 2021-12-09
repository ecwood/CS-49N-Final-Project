#include "unpack.h"

const int COLOR_INCREMENT = 40;
const int COLOR_MAX = 255;
const int COLOR_MIN = 0;

uint8_t joy_mask() {
	uint8_t pre_invert = 1 << 7;
	return ~(pre_invert);
}

unsigned format_color(unsigned original_color, unsigned button_val) {
	assert(original_color <= COLOR_MAX && original_color >= 0);
	unsigned color = original_color + (button_val * COLOR_INCREMENT);
	if (color >= COLOR_MAX) color = color - COLOR_MAX;
	if (color <= COLOR_MIN) color = color + COLOR_MAX;
	return color;
}

unsigned unpack(Color* col, unsigned *joy, uint16_t data) {
	unsigned joy_val = data & joy_mask();
	unsigned button_val = data >> 7 & 1;
	unsigned red = data >> 10 & 1;
	unsigned green = data >> 9 & 1;
	unsigned blue = data >> 8 & 1;
	col->red = format_color(col->red, red);
	col->blue = format_color(col->blue, blue);
	col->green = format_color(col->green, green);
	*(joy) = joy_val;
	return button_val;
}