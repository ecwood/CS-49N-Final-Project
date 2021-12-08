// fragile one-way test to see that something is coming out.
#include "rpi.h"
#include "nrf.h"
#include "nrf-test.h"
#include "color.h"
#include "unpack.h"
#include "lightstrip.h"

void notmain(void) {
    unsigned n = NTEST;
    unsigned light_pin = 4;
    unsigned npixels = 59;

    printk("recv: am a client: going to do %d trials\n", n);
    nrf_config_t c = nrf_conf_reliable_mk(RETRAN_ATTEMPTS, RETRAN_DELAY);
    nrf_init_acked(c, client_addr, 4);
    
    nrf_dump("client");

    Color col = make_color(100, 100, 100);

    neo_t light_strip = initialize_light_strip(light_pin, npixels);

    while (1) {
        uint32_t x;
        unsigned joy;
        if(nrf_get_data_exact(client_addr, &x, 4) != 4)
            panic("impossible\n");
        unsigned button_val = unpack(&col, &joy, x);
        light_up(light_strip, &col, joy, npixels);
        //printk("client: received joy=%d, red=%d, green=%d, blue=%d\n", joy, col.red, col.green, col.blue);
        if (button_val) break;
    }
    set_pixels_off(light_strip, npixels);
    nrf_dump("client end");
    printk("client: done!\n");
}