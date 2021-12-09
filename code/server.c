#include "rpi.h"
#include "nrf.h"
#include "nrf-test.h"
#include "i2c.h"
#include "button.h"
#include "ads1115_joystick.h"
#include "package_data.h"
#include "color.h"

void notmain(void) {
    int on_off_pin = 4;
    int red_pin = 22;
    int blue_pin = 27;
    int green_pin = 17;
    unsigned nbytes = 2;
    unsigned npixels = 59;
    uart_init();
    delay_ms(30);   // allow time for device to boot up.
    i2c_init();
    delay_ms(30);   // allow time to settle after init.

    unsigned dev_addr = ads1115_config();

    nrf_config_t c = nrf_conf_reliable_mk(RETRAN_ATTEMPTS, RETRAN_DELAY);
    nrf_init_acked(c, server_addr, nbytes);

    button_initialize(on_off_pin);
    button_initialize(red_pin);
    button_initialize(blue_pin);
    button_initialize(green_pin);

    // Initialize the packet without any meaningful data
    uint16_t pack = package_buttons(0, 0, 0, 0) | 0;

    // Set the last packet equal to this empty packet
    uint16_t last_pack = pack;
    unsigned next_on_off = 0;
    uint16_t last_buttons = 0;

    while (1) {
        // Package the button values in a 2 byte value
        uint16_t buttons = package_buttons(get_button_val(red_pin), get_button_val(green_pin), get_button_val(blue_pin), get_button_val(on_off_pin));

        // Add the joystick value to the packet started with buttons
        pack = buttons | (get_joystick_val(dev_addr, npixels));

        // Only send if the packet is different, so that we don't unnecessarily increment the hue
        if (pack != last_pack) {
            // If the buttons are still pressed, we don't want to keep incrementing. Clear them.
            if (last_buttons == buttons) {
                pack = pack & ~(0b111 << 8);
            }
            nrf_send_ack(client_addr, &pack, nbytes);
        }

        // If we sent an off signal last time, turn off now
        if (next_on_off) break;

        // Set up the previous markers
        next_on_off = pack >> 7 & 1;
        last_pack = pack;
        last_buttons = buttons;
    }
    nrf_dump("server end");
    printk("server: done!\n");
}
