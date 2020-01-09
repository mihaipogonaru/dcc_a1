#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int ret;

    if (argc < 2) {
        printf("Too few parameters\n");
        return -1;
    }

    read_times_config();
    init_l2(sleep_per_bit_ns, th_clocks_per_bit);

    if (strcmp(argv[1], "send") == 0) {
        printf("Sending\n");
        ret = send(TO_SEND, sizeof(TO_SEND));
        printf("SENT %d\n", ret);
    } else {
        char buf[20];
        printf("Receiving\n");

        ret = recv(buf, sizeof(TO_SEND));
        printf("Received %d bytes\n", ret);
        if (ret >= 0) {
            buf[ret] = '\0';
            printf("Msg: %s\n", buf);
        }
    }
    uninit_l2();

    return 0;
}
