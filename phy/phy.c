#include <stdio.h>

#include "cable.h"
#include "phy.h"

/* Intervals of sleep_per_bit_ns in which the lane is high
 * that must pass to consider the receiver reading
 *
 * A.k.a we wait some time and check if the lane is high
 */
#define RECEIVER_READING_THRESHOLD 8

/* Max intervals to wait for receiver to be reading */
#define RECEIVER_READING_MAX_WAIT 16

static unsigned long sleep_per_bit_ns;
static unsigned long clocks_per_bit;

static char preamble[] = {0xAA, 0xFA} /* 10101010, 11111010 */;

void init_phy(unsigned long sleep_per_bit_ns_,
        unsigned long clocks_per_bit_)
{
    sleep_per_bit_ns = sleep_per_bit_ns_;
    clocks_per_bit = clocks_per_bit_;
}
void uninit_phy(void)
{}

static int send_bit(char bit)
{
    int ret;

    if (bit)
        ret = keep_lane_high(sleep_per_bit_ns);
    else
        ret = keep_lane_low(sleep_per_bit_ns);

    return ret;
}

static int recv_bit(void)
{
    unsigned long elapsed;

    elapsed = keep_lane_high(sleep_per_bit_ns);
    if (elapsed < clocks_per_bit)
        return 1;

    return 0;
}

static int lane_is_high(void)
{
    return recv_bit() == 1;
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
 * ifg_length are 0x00
 */
static int check_ifg(void)
{
    int i, ret, count = 0;

    for (i = 0; i < IFG_LENGTH; ++i) {
        ret = recv_byte();

        if (ret == 0)
            count++;
    }

    if (count < IFG_LENGTH)
        return -1;

    return 0;
}

/* Checks if the receiver is reading */
static int check_receiver_reading(void)
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

int transmit_frame(char *frame)
{
    int ret;

    printf("WAITING RECEVIER TO READ\n");
    ret = check_receiver_reading();
    if (ret)
        return ret;

    printf("SENDING PREAMBLE\n");
    ret = send_preamble();
    if (ret)
        return ret;

    printf("SENDING DATA\n");
    ret = do_transmit(frame, FRAME_LENGTH);
    if (ret)
        return ret;

    printf("WAITING FOR IFG\n");
    return check_ifg();
}

int receive_frame(char *frame)
{
    int i, ret;

    ret = recv_preamble();
    if (ret)
        return ret;

    i = 0;
    while (i < FRAME_LENGTH) {
        ret = recv_byte();
        if (ret < 0)
            return ret;

        frame[i++] = ret;
    }

    printf("LANE LOW FOR IFG\n");
    /* Leave IFG after frame */
    for (i = 0; i < 8 * IFG_LENGTH; ++i) {
        ret = keep_lane_low(sleep_per_bit_ns);
        if (ret)
            return ret;
    }

    return 0;
}
