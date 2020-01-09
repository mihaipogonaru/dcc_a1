#include <stdio.h>
#include <errno.h>

#include "cable.h"
#include "phy.h"

/* bits of 1 to consider the receiver reading */
#define RECEIVER_READING_THRESHOLD 8
/* max bits to wait for receiver to be reading */
#define RECEIVER_READING_MAX_WAIT 16

static char preamble[] = {0xAE, 0xAA, 0xAA, 0x75} /* 10101110, 10101010, 10101010, 01111010 */;

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

static int send_preamble(void)
{
    return do_transmit(preamble, PRAMBLE_LENGTH);
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

static int recv_preamble(void)
{
    int i, j, ret;
    char bit;

    for (i = 0; i < PRAMBLE_LENGTH; ++i) {
        for (j = 0; j < 8; ++j) {
            ret = recv_bit();
            if (ret < 0)
                return -1;

            bit = (preamble[i] >> j) & 1;
            if (bit != ret)
                break;
            printf("Correct bit %hhu %hhu %d!\n", bit, ret, j);
        }

        /* If we didn't receive all the bits correctly */
        if (j != 8)
            break;
    }

    if (i == sizeof(preamble)) {
        printf("Received start!\n");
        return 0;
    }

    return -1;
}

/* To mark a frame as received correctly
 * the receiver lets the lane low for the ifg duration
 *
 * We check that for the next ifg_length bytes at least
 * ifg_length / 2 are 0x00
 */
static int check_ifg(void)
{
    int i, ret, count = 0;

    for (i = 0; i < IFG_LENGTH; ++i) {
        ret = recv_byte();

        if (ret == 0)
            count++;
    }

    return count >= IFG_LENGTH / 2;
}

static int wait_receiver_reading(void)
{
    int i, counter = 0;

    for (i = 0; i < RECEIVER_READING_MAX_WAIT; ++i) {
        if (lane_is_high()) {
            counter++;
            if (counter == RECEIVER_READING_THRESHOLD)
                return 0;
        } else {
            counter = 0;
        }
    }

    return -1;
}

int transmit_frame(char *buf)
{
    int ret;

    printf("WAITING RECEVIER\n");
    ret = wait_receiver_reading();
    if (ret)
        return ret;

    printf("SENDING PREAMBLE\n");
    ret = send_preamble();
    if (ret)
        return ret;

    printf("SENDING DATA\n");
    ret = do_transmit(buf, FRAME_LENGTH);
    if (ret)
        return ret;

    return check_ifg();
}

int receive_frame(char *buf)
{
    int ret, len = 0;

    ret = recv_preamble();
    if (ret)
        return ret;

    while (len < FRAME_LENGTH) {
        ret = recv_byte();
        if (ret < 0)
            return ret;

        buf[len++] = ret;
    }

    return len;
}
