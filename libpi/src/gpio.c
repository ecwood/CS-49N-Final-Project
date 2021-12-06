/*
 * Implement the following routines to set GPIO pins to input or output,
 * and to read (input) and write (output) them.
 *
 * DO NOT USE loads and stores directly: only use GET32 and PUT32
 * to read and write memory.  Use the minimal number of such calls.
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"
#include <stdio.h>

// see broadcomm document for magic addresses.
#define GPIO_BASE 0x20200000
// do not read or write these directly --- only use put32 and get32
// (defined in start.S)
static volatile unsigned *gpio_fsel0 = (void*)(GPIO_BASE + 0x00);
static volatile unsigned *gpio_set0  = (void*)(GPIO_BASE + 0x1C);
static volatile unsigned *gpio_clr0  = (void*)(GPIO_BASE + 0x28);
static volatile unsigned *gpio_lev0 = (void*)(GPIO_BASE + 0x34);

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use array calculations!
void gpio_set_output(unsigned pin) {
    if (pin < 32 && pin >= 0) {
        int fsel_register = pin / 10;
        // Going forward, consider combining gpio_fsel + fsel_register into one variable
        // Also, consider combining (3 * (pin % 10)) into one variable representing the location of the
        // first bit that gets changed (per the standards outlined by the Rasberry Pi people)
        // ~(0b111 << (3 * pin % 10)) moves 111 into the bits that refer to the specific pin, then the 
        // ~ flips the full 32 bits, so that the ones occupied by the 111's become zeros.
        // When &'ed with the existing value of the FSEL register, the 1's in all of the spots that are
        // not occupied by the pin make it so the existing values of the register remain in those spots.
        // The 0's in the bits occupied by the pin clear that pin's value, making it so it can be turned
        // on. Then, on the next line, a bit shift with 0001 (into the spot it needs to be for based on
        // documentation), we can use | to combine the two, such that all of the 1's in each remain
        // in the output. 
        unsigned int fsel_clear = get32(gpio_fsel0 + fsel_register) & ~(0b111<<(3* (pin % 10)));
        put32(gpio_fsel0 + fsel_register, (fsel_clear | (0b001 << (3 * (pin % 10)))));
    }
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    if (pin < 32 && pin >= 0) {
        put32(gpio_set0, (1 << pin));
    }
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    if (pin < 32 && pin >= 0) {
        put32(gpio_clr0, (1 << pin));
    }
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if (v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned pin) {
    if (pin < 32 && pin >= 0) {
        int fsel_register = pin / 10;
        unsigned int fsel_clear = get32(gpio_fsel0 + fsel_register) & ~(0b111<<(3 * (pin % 10)));
        put32(gpio_fsel0 + fsel_register, (fsel_clear));
    }
}

// return the value of <pin>
int gpio_read(unsigned pin) {
    if (pin < 32 && pin >= 0) {
        unsigned v = 0;

        unsigned int val = get32(gpio_lev0);
        val = (val & (1 << pin)) >> pin; // val = (val >> pin) & 1 also works

        return val;
    } else {
        return 0;
    }
}

void gpio_set_function(unsigned pin, gpio_func_t function) {
    if (pin < 32 && pin >= 0 && function <= 7 && function >= 0) {
        int fsel_register = pin / 10;
        unsigned int fsel_clear = get32(gpio_fsel0 + fsel_register) & ~(0b111<<(3 * (pin % 10)));
        put32(gpio_fsel0 + fsel_register, (fsel_clear | (function << (3 * (pin % 10)))));
    }
}