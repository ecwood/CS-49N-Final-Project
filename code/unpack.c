#include "unpack.h"

const int COLOR_INCREMENT_U = 40;
const int COLOR_MAX_U = 255;
const int COLOR_MIN_U = 0;

uint8_t bit_mask() {
	return ~0;
}

uint8_t joy_mask() {
	uint8_t pre_invert = 1 << 7;
	return ~(pre_invert);
}

unsigned unpack(Color* col, unsigned *joy, uint32_t data) {
	unsigned joy_val = (data >> 23) & ~(1 << 7);
	unsigned button_val = data >> 30;
	unsigned red = data >> 15 & bit_mask();
	unsigned green = (data >> 7) & bit_mask();
	unsigned blue = data & bit_mask();
	col->red = red;
	col->blue = blue;
	col->green = green;
	*(joy) = joy_val;
	return button_val;
}

unsigned format_color_u(unsigned original_color, unsigned button_val) {
	assert(original_color <= COLOR_MAX_U && original_color >= 0);
	unsigned color = original_color + (button_val * COLOR_INCREMENT_U);
	if (color >= COLOR_MAX_U) color = color - COLOR_MAX_U;
	if (color <= COLOR_MIN_U) color = color + COLOR_MAX_U;
	return color;
}

unsigned unpack_16(Color* col, unsigned *joy, uint16_t data) {
	unsigned joy_val = data & joy_mask();
	unsigned button_val = data >> 7 & 1;
	unsigned red = data >> 10 & 1;
	unsigned green = data >> 9 & 1;
	unsigned blue = data >> 8 & 1;
	col->red = format_color_u(col->red, red);
	col->blue = format_color_u(col->blue, blue);
	col->green = format_color_u(col->green, green);
	*(joy) = joy_val;
	return button_val;
}