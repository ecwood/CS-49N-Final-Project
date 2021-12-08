#include "rpi.h"
#include "button.h"

void button_initialize(unsigned pin) {
	gpio_set_input(pin);
}

unsigned get_button_val(unsigned pin) {
	return ~(gpio_read(pin)) & 1;
}