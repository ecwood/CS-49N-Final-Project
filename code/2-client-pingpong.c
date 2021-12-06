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

    for(unsigned i = 0; i < n; i++) {
        int x;

        if(nrf_get_data_exact(client_addr, &x, 8) != 8)
            panic("impossible\n");

        printk("client: received %d (expected=%d)\n", x,i);
        delay_ms(DELAY_MS);

        printk("client: sending %d\n", i);

        if(nrf_send_ack(server_addr, &i, 8) != 8)
            panic("send failed\n");
        printk("client: sent %d\n", i);
    }
    nrf_dump("client end");
    printk("client: done!\n");
}
