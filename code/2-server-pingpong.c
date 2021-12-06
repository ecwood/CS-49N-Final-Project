// fragile one-way test to see that something is coming out.
#include "rpi.h"
#include "nrf.h"
#include "nrf-test.h"
#include "i2c.h"
#include "ads1115_joystick.h"

void notmain(void) {
    unsigned n = NTEST;
    uart_init();
    delay_ms(30);   // allow time for device to boot up.
    i2c_init();
    delay_ms(30);   // allow time to settle after init.

    unsigned dev_addr = ads1115_config();

    printk("server: going to do %d trials\n", n);

    nrf_config_t c = nrf_conf_reliable_mk(RETRAN_ATTEMPTS, RETRAN_DELAY);
    nrf_init_acked(c, server_addr, 8);

    for(unsigned i = 0; i < n; i++) {
        int send = ((get_joystick_sample(dev_addr) + 100) / 2);
        if(nrf_send_ack(client_addr, &send, 8) != 8)
            panic("send failed\n");
        printk("server: sent %d\n", send);

        unsigned x;
        if(nrf_get_data_exact(server_addr, &x, 8) != 8)
            panic("impossible\n");
        printk("server: received %d (expected=%d)\n", x,i);
        assert(x == i);
        delay_ms(DELAY_MS);
    }
    nrf_dump("server end");
    printk("server: done!\n");
}
