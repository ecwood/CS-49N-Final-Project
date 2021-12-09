#ifndef __ADS1115_H__
#define __ADS1115_H__

void ads1115_reset(void);

void ads1115_write16(uint8_t dev_addr, uint8_t reg, uint16_t v);

uint16_t ads1115_read16(uint8_t dev_addr, uint8_t reg);

uint8_t ads1115_config(void);

int get_joystick_val(uint8_t dev_addr);

int get_joystick_sample(uint8_t dev_addr);

#endif
