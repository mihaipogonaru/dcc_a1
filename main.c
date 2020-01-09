#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "l2/l2.h"

unsigned long sleep_per_bit_ns;
unsigned long th_clocks_per_bit;

static void err_exit(const char *msg)
{
    printf("%s\n", msg);
    exit(-1);
}

static void read_times_config(void)
{
    FILE *times;

    times = fopen("times.config", "r");
    if (!times)
        err_exit("No times.config file found");

    fscanf(times, "%lu", &sleep_per_bit_ns);
    fscanf(times, "%lu", &th_clocks_per_bit);

    fclose(times);
}

static int get_file_size(FILE *fp)
{
    int sz;

    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);

    fseek(fp, 0L, SEEK_SET);

    return sz;
}

int main(int argc, char *argv[])
{
    int len, ret;
    char *buf;
    FILE *fp;

    if (argc < 3)
        err_exit("Too few parameters");

    read_times_config();
    init_l2(sleep_per_bit_ns, th_clocks_per_bit);

    if (strcmp(argv[1], "send") == 0) {
        printf("Sending\n");

        fp = fopen(argv[2], "r");
        if (!fp)
            err_exit("File to read from not found");
        len = get_file_size(fp);

        printf("Sending size: %d\n", len);
        ret = send((char *)&len, sizeof(len), -1);
        if (ret < 0)
            err_exit("Failed to send size");

        printf("Sent size\n");
        buf = malloc(len);
        if (!buf)
            err_exit("Failed to allocate buffer");

        ret = fread(buf, 1, len, fp);
        if (ret != len)
            err_exit("Error reading from file");
        fclose(fp);

        printf("Sending %d bytes\n", len);
        ret = send(buf, len, -1);
        if (ret < 0)
            err_exit("Failed to send message");

        free(buf);
        printf("Done sending\n");
    } else {
        printf("Receiving\n");

        ret = recv((char *)&len, sizeof(len), -1);
        if (ret < 0)
            err_exit("Failed to read message size");

        printf("Received size: %d\n", len);
        buf = malloc(len);
        if (!buf)
            err_exit("Failed to allocate buffer");

        ret = recv(buf, len, -1);
        if (ret < 0)
            err_exit("Failed to read message");

        printf("Received %d bytes\n", ret);
        fp = fopen(argv[2], "w");
        if (!fp)
            err_exit("Failed to open file to write to");
        fwrite(buf, 1, len, fp);
        fclose(fp);

        free(buf);
        printf("Done receiving\n");
    }
    uninit_l2();

    return 0;
}
