// fragile one-way test to see that something is coming out.
#include "rpi.h"
#include "nrf.h"
#include "nrf-test.h"
#include "i2c.h"
#include "ads1115_joystick.h"

void notmain(void) {
    int button_pin = 4;
    unsigned n = NTEST;
    uart_init();
    delay_ms(30);   // allow time for device to boot up.
    i2c_init();
    delay_ms(30);   // allow time to settle after init.

    unsigned dev_addr = ads1115_config();

    nrf_config_t c = nrf_conf_reliable_mk(RETRAN_ATTEMPTS, RETRAN_DELAY);
    nrf_init_acked(c, server_addr, 8);

    gpio_set_input(button_pin);

    while (1) {
        uint8_t button_val = (~(gpio_read(button_pin)) << 7) & (1 << 7);
        printk("button_val: %x\n", button_val);
        uint8_t val = ((get_joystick_sample(dev_addr) + 100) / 2);
        uint8_t send = val | button_val; // eq maxes at 100, so have an extra bit for button
        if(nrf_send_ack(client_addr, &send, 8) != 8)
            panic("send failed\n");
        printk("server: sent %d\n", val);
        if (button_val) break;
    }
    nrf_dump("server end");
    printk("server: done!\n");
}
