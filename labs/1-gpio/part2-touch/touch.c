#include "gpio.h"

void notmain(void);

void notmain(void) {
    const int led1 = 26;
    const int led2 = 20;
    gpio_set_output(led1);
    gpio_set_output(led2);

    const int touch = 21;
    gpio_set_input(touch);
    unsigned ctr = 0;
    unsigned flip = 0;
    while(1) {
        ctr++;
        if (flip ^ gpio_read(touch)) {
            gpio_write(led1, (ctr >> 16) & 1);
            gpio_write(led2, (ctr >> 17) & 1);
        } else {
            gpio_write(led1, (ctr >> 17) & 1);
            gpio_write(led2, (ctr >> 18) & 1);
        }
    }
}
