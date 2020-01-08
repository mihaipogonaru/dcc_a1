#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "../phy/cable.h"
#include "../phy/phy.h"

extern unsigned long get_th_clock();

int main(int argc, char *argv[])
{
    if (strcmp(argv[1], "runner") == 0) {
        for (;;);
    } else {
        unsigned long elapsed;
        unsigned long sleep_per_bit_ns;

        sleep_per_bit_ns = strtoul(argv[2], NULL, 10);

        init_phy(sleep_per_bit_ns, 0);
        printf("Receiving\n");
        recv_bit();
        printf("Done\n");

        elapsed = get_th_clock();
        uninit_phy();

        printf("%lu\n", elapsed);
    }

    return 0;
}
