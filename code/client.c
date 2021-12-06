// fragile one-way test to see that something is coming out.
#include "rpi.h"
#include "nrf.h"
#include "nrf-test.h"

void notmain(void) {
    unsigned n = NTEST;

    printk("recv: am a client: going to do %d trials\n", n);
    nrf_config_t c = nrf_conf_reliable_mk(RETRAN_ATTEMPTS, RETRAN_DELAY);
    nrf_init_acked(c, client_addr, 8);
    
    nrf_dump("client");

    while (1) {
        uint8_t x;

        if(nrf_get_data_exact(client_addr, &x, 8) != 8)
            panic("impossible\n");
        int val = x & ~(1 << 7);
        int button_val = x >> 7;
        printk("client: received %d\n", val);
        delay_ms(DELAY_MS);
        if (button_val) break;
    }
    nrf_dump("client end");
    printk("client: done!\n");
}