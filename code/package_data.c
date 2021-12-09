#include "rpi.h"
#include "color.h"
#include "package_data.h"

uint16_t package_buttons(unsigned r_button, unsigned g_button, unsigned b_button, unsigned on_off_button) {
	return (r_button << 3 | g_button << 2 | b_button << 1 | on_off_button) << 7;
}