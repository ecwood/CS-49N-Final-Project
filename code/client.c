#include "rpi.h"
#include "nrf.h"
#include "nrf-test.h"
#include "color.h"
#include "unpack.h"
#include "lightstrip.h"

void notmain(void) {
    unsigned light_pin = 4;
    unsigned npixels = 59;
    unsigned nbytes = 2;

    nrf_config_t c = nrf_conf_reliable_mk(RETRAN_ATTEMPTS, RETRAN_DELAY);
    nrf_init_acked(c, client_addr, nbytes);
    
    nrf_dump("client");

    Color col = make_color(100, 100, 100);

    neo_t light_strip = initialize_light_strip(light_pin, npixels);

    unsigned joy = 0;
    uint16_t pack = 0;

    while (1) {
        int received = nrf_get_data_exact_noblk(client_addr, &pack, nbytes);
        unsigned button_val = unpack(&col, &joy, pack);

        // If we received the right packet, set the lights
        if (received == nbytes) {
            light_up(light_strip, &col, joy, npixels);
        }

        // If the off button has been pressed, break out of the while loop
        if (button_val) break;

        // Clear out the button section of the packet now that we've incremeneted, (acts as a toggle)
        pack = pack & ~(0b111 << 8);
    }
    // When we are done receiving, turn the light strip off
    set_pixels_off(light_strip, npixels);

    nrf_dump("client end");
    printk("client: done!\n");
}