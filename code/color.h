#ifndef __COLOR_H__
#define __COLOR_H__

typedef struct color {
    unsigned red, green, blue;
} Color;

static inline Color make_color(unsigned r, unsigned g, unsigned b) {
	assert(r <= 255);
	assert(r >= 0);
	assert(g <= 255);
	assert(g >= 0);
	assert(b <= 255);
	assert(b >= 0);
	Color col = (Color) {
		.red = r,
		.green = g,
		.blue = b
	};
	return col;
}

#endif