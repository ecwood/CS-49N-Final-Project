#include "rpi.h"
#include "WS2812B.h"
#include "neopixel.h"
#include "color.h"
#include "lightstrip.h"

neo_t initialize_light_strip(int pin, int npixels) {
	enable_cache();
	gpio_set_output(pin);
	neo_t h = neopix_init(pin, npixels);
	return h;
}

void set_pixels_off(neo_t h, int npixels) {
	for (int pixel = 0; pixel < npixels; pixel++) {
		neopix_write(h, pixel, 0, 0, 0);
	}
	neopix_flush(h);
}

void light_up(neo_t h, Color* col, int percent, int npixels) {
	int pixels = (npixels * percent) / 100;
	for (int pixel = 0; pixel < pixels; pixel++) {
		neopix_write(h, pixel, col->red, col->green, col->blue);
	}
	neopix_flush(h);
}