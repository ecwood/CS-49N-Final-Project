#include "unpack.h"

uint8_t bit_mask() {
	return ~0;
}

unsigned unpack(Color* col, unsigned *joy, uint32_t data) {
	unsigned joy_val = (data >> 23) & ~(1 << 7);
	unsigned button_val = data >> 31;
	unsigned red = data >> 15 & bit_mask();
	unsigned green = (data >> 7) & bit_mask();
	unsigned blue = data & bit_mask();
	col->red = red;
	col->blue = blue;
	col->green = green;
	*(joy) = joy_val;
	return button_val;
}