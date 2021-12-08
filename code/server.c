#include "rpi.h"
#include "nrf.h"
#include "nrf-test.h"
#include "i2c.h"
#include "button.h"
#include "ads1115_joystick.h"
#include "package_data.h"

void notmain(void) {
    int on_off_pin = 4;
    int red_pin = 22;
    int blue_pin = 27;
    int green_pin = 17;
    unsigned n = NTEST;
    uart_init();
    delay_ms(30);   // allow time for device to boot up.
    i2c_init();
    delay_ms(30);   // allow time to settle after init.

    unsigned dev_addr = ads1115_config();

    nrf_config_t c = nrf_conf_reliable_mk(RETRAN_ATTEMPTS, RETRAN_DELAY);
    nrf_init_acked(c, server_addr, 4);

    button_initialize(on_off_pin);
    button_initialize(red_pin);
    button_initialize(blue_pin);
    button_initialize(green_pin);

    Color col = make_color(100, 100, 100);

    while (1) {
        uint8_t joy = ((get_joystick_sample(dev_addr) + 100) / 2);
        unsigned on_off_val = get_button_val(on_off_pin);
        uint32_t send = package(&col, get_button_val(red_pin), get_button_val(green_pin), get_button_val(blue_pin), on_off_val, joy);
        //printk("server: sent joy=%d, red=%d, green=%d, blue=%d\n", joy, col.red, col.green, col.blue);
        if(nrf_send_ack(client_addr, &send, 4) != 4)
            panic("send failed\n");
        if (on_off_val) break;
    }
    nrf_dump("server end");
    printk("server: done!\n");
}
