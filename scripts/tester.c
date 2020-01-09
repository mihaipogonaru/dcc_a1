#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "../phy/cable.h"

int main(int argc, char *argv[])
{
    unsigned long elapsed = 0;
    unsigned long sleep_per_bit_ns;

    sleep_per_bit_ns = strtoul(argv[2], NULL, 10);

    if (strcmp(argv[1], "runner") == 0) {
        for (;;)
            keep_lane_high(sleep_per_bit_ns);
    } else {
        int i;

        printf("Receiving\n");
        for (i = 0; i < 3; ++i)
            elapsed += keep_lane_high(sleep_per_bit_ns);
        printf("Done\n");

        printf("%lu\n", elapsed / 3);
    }

    return 0;
}
