#include "rpi.h"
#include "WS2812B.h"
#include "neopixel.h"
#include "color.h"

neo_t initialize_light_strip(int pin, int npixels);

void set_pixels_off(neo_t h, int npixels);

void light_up(neo_t h, Color* col, int percent, int npixels);