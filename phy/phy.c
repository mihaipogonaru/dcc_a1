#include <stdio.h>
#include <errno.h>

#include "cable.h"
#include "phy.h"

static char start_transmit = 0xAF /* 10101111 */;
static char stop_transmit  = 0xA0 /* 10100000 */;

void init_phy(unsigned long sleep_per_bit_ns,
        unsigned long th_clocks_per_bit)
{
    init_cable(sleep_per_bit_ns, th_clocks_per_bit);
}
void uninit_phy(void)
{
    uninit_cable();
}

static int send_byte(char byte)
{
    int i, ret;

    for (i = 0; i < 8; ++i) {
        ret = send_bit((byte >> i) & 1);
        if (ret < 0)
            return ret;
    }

    return 0;
}

static int do_transmit(char *buf, int len)
{
    int i, ret;

    for (i = 0; i < len; ++i) {
        ret = send_byte(buf[i]);
        if (ret)
            return ret;
    }

    return 0;
}

static int send_start_transmit(void)
{
    return do_transmit(&start_transmit, 1);
}

static int send_stop_transmit(void)
{
    return do_transmit(&stop_transmit, 1);
}

int transmit(char *buf, int len)
{
    int ret;

    ret = send_start_transmit();
    if (ret)
        return ret;

    ret = do_transmit(buf, len);
    if (ret)
        return ret;

    ret = send_stop_transmit();

    return ret;
}

static int recv_byte(void)
{
    int i, ret;
    unsigned char buf = 0;

    for (i = 0; i < 8; ++i) {
        ret = recv_bit();
        if (ret < 0)
            return ret;

        buf |= ((char) ret) << i;
    }

    printf("Received 0x%02hhx\n", buf);
    return buf;
}

static int recv_start_transmit(int retries)
{
    int i, j, ret;
    char bit;

    for (i = 0; i < retries; ++i) {
        for (j = 0; j < 8; ++j) {
            ret = recv_bit();
            if (ret < 0)
                return -1;

            bit = (start_transmit >> j) & 1;
            if (bit != ret)
                break;
            else if (j == 7) {
                printf("Received start!\n");
                return 0;
            }
            printf("Correct bit %hhu %hhu %d!\n", bit, ret, j);
        }
    }

    return -1;
}

int receive(char *buf, int max_len, int retries)
{
    int ret, len = 0;
    char crr;

    ret = recv_start_transmit(retries);
    if (ret)
        return ret;

    while (len < max_len) {
        ret = recv_byte();
        if (ret < 0)
            return ret;

        crr = (char) ret;
        if (crr == stop_transmit)
            break;

        buf[len++] = crr;
    }

    return len;
}
