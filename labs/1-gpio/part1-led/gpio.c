/*
 * write code to allow blinking using arbitrary pins.    
 * Implement:
 *	- gpio_set_output(pin) --- set GPIO <pin> as an output (vs input) pin.
 *	- gpio_set_on(pin) --- set the GPIO <pin> on.
 * 	- gpio_set_off(pin) --- set the GPIO <pin> off.
 * Use the minimal number of loads and stores to GPIO memory.  
 *
 * start.s defines a of helper functions (feel free to look at the assembly!  it's
 *  not tricky):
 *      uint32_t get32(volatile uint32_t *addr) 
 *              --- return the 32-bit value held at <addr>.
 *
 *      void put32(volatile uint32_t *addr, uint32_t v) 
 *              -- write the 32-bit quantity <v> to <addr>
 * 
 * Check-off:
 *  1. get a single LED to blink.
 *  2. attach an LED to pin 19 and another to pin 20 and blink in opposite order (i.e.,
 *     one should be on, while the other is off).   Note, if they behave weirdly, look
 *     carefully at the wording for GPIO set.
 */
#include "gpio.h"

/*
 * These routines are given by us and are in start.s
 */
// writes the 32-bit value <v> to address <addr>:   *(unsigned *)addr = v;
void put32(volatile void *addr, unsigned v);
// returns the 32-bit value at <addr>:  return *(unsigned *)addr
unsigned get32(const volatile void *addr);
// does nothing.
void nop(void);

// see broadcomm documents for magic addresses.
#define GPIO_BASE 0x20200000
volatile unsigned *gpio_fsel0 = (void*)(GPIO_BASE + 0x00);
volatile unsigned *gpio_set0  = (void*)(GPIO_BASE + 0x1C);
volatile unsigned *gpio_clr0  = (void*)(GPIO_BASE + 0x28);
volatile unsigned *gpio_lev0  = (void*)(GPIO_BASE + 0x34);

// idea shamelessly stolen from mlfb
typedef enum gpio_direction_enum {
    gpio_dir_in = 0,
    gpio_dir_out,
    gpio_fn_0,
    gpio_fn_1,
    gpio_fn_2,
    gpio_fn_3,
    gpio_fn_4,
    gpio_fn_5,
} gpio_direction_ty;

static void _gpio_set_io(unsigned pin, gpio_direction_ty direction) {
    // no pins above 53 are defined
    if (pin > 53) {
        return;
    }
    // read
    volatile unsigned *pin_reg = gpio_fsel0 + (pin / 10);
    unsigned current_value = get32(pin_reg);
    // modify
    unsigned pin_offset = (pin % 10) * 3;
    unsigned pin_mask = ~(0x07 << pin_offset);
    current_value &= pin_mask;
    current_value |= ((unsigned)direction) << pin_offset;
    // write
    put32(pin_reg, current_value);
}

#define GET_PIN_AND_REG(RET)    \
    do {                        \
        if (pin > 53) {         \
            return RET;         \
        }                       \
        if (pin > 31) {         \
            pin -= 32;          \
            pin_reg += 1;       \
        }                       \
    } while (0)

static void _gpio_set(unsigned pin, volatile unsigned *pin_reg) {
    GET_PIN_AND_REG();
    put32(pin_reg, 1 << pin);
}

// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output

// set <pin> to be an output pin.  note: fsel0, fsel1, fsel2 are contiguous in memory,
// so you can use array calculations!
void gpio_set_output(unsigned pin) {
    _gpio_set_io(pin, gpio_dir_out);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    _gpio_set(pin, gpio_set0);
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    _gpio_set(pin, gpio_clr0);
}

// Part 2: implement gpio_set_input and gpio_read

// set <pin> to input.
void gpio_set_input(unsigned pin) {
    _gpio_set_io(pin, gpio_dir_in);
}

// return the value of <pin>
int gpio_read(unsigned pin) {
    volatile unsigned *pin_reg = gpio_lev0;
    GET_PIN_AND_REG(-1);
    unsigned v = get32(pin_reg);
    return (v >> pin) & 1;
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v) {
       gpio_set_on(pin);
    } else {
       gpio_set_off(pin);
    }
}
