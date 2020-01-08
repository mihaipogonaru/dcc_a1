#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "l2/l2.h"

#define TO_SEND "Hello, World!"

unsigned long sleep_per_bit_ns;
unsigned long th_clocks_per_bit;

static void read_times_config(void)
{
    FILE *times;

    times = fopen("times.config", "r");
    if (!times) {
        printf("No times.config file found\n");
        exit(-1);
    }
    fscanf(times, "%lu", &sleep_per_bit_ns);
    fscanf(times, "%lu", &th_clocks_per_bit);
    fclose(times);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Too few parameters\n");
        return -1;
    }

    read_times_config();

    if (strcmp(argv[1], "send") == 0) {
        init_phy(sleep_per_bit_ns, th_clocks_per_bit);
        printf("Sending\n");
        transmit(TO_SEND, sizeof(TO_SEND) - 1);
        printf("SENT\n");
    } else {
        int len;
        char buf[20];

        init_phy(sleep_per_bit_ns, th_clocks_per_bit);
        printf("Receiving\n");

        len = receive(buf, 20, 1000);
        printf("Received %d bytes\n", len);
        if (len >= 0) {
            buf[len] = '\0';
            printf("Msg: %s\n", buf);
        }
    }

    uninit_phy();

    return 0;
}
