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

    unsigned light_time = 0;
    unsigned get_time = 0;
    uint16_t x;
    int loop = 0;
    unsigned joy = 0;
    uint32_t pack = 0;
    while (1) {
        loop++;
        // unsigned start_get = timer_get_usec();
        int received = nrf_get_data_exact_noblk(client_addr, &pack, 4);
        unsigned button_val = unpack(&col, &joy, pack);
        if (received == 4) {
            light_up(light_strip, &col, joy, npixels);
        }
        //printk("client: received joy=%d, red=%d, green=%d, blue=%d\n", joy, col.red, col.green, col.blue);
        if (button_val) break;
    }
    set_pixels_off(light_strip, npixels);
    nrf_dump("client end");
    printk("client: done!\n");
}