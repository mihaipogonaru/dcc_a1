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

static char preamble[] = {0xAA, 0xFA} /* 10101010, 10101010, 11111010 */;

static struct {
    unsigned long sleep_per_bit_ns;
    unsigned long clocks_per_bit;
    check_recv_frame check_recv_frame_fn;
} phy;

void init_phy(check_recv_frame check_recv_frame_fn,
        unsigned long sleep_per_bit_ns,
        unsigned long clocks_per_bit)
{
    phy.check_recv_frame_fn = check_recv_frame_fn;
    phy.sleep_per_bit_ns = sleep_per_bit_ns;
    phy.clocks_per_bit = clocks_per_bit;
}

void uninit_phy(void)
{
    phy.check_recv_frame_fn = NULL;
    phy.sleep_per_bit_ns = 0;
    phy.clocks_per_bit = 0;
}

static int send_bit(char bit)
{
    int ret;

    if (bit)
        ret = keep_lane_high(phy.sleep_per_bit_ns);
    else
        ret = keep_lane_low(phy.sleep_per_bit_ns);

    return ret;
}

static int recv_bit(void)
{
    unsigned long elapsed;

    elapsed = keep_lane_high(phy.sleep_per_bit_ns);
    if (elapsed < phy.clocks_per_bit)
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

    if (i == sizeof(preamble))
        return 0;

    return -1;
}

/* To mark a frame as received correctly
 * the receiver lets the lane low for the ifg duration
 *
 * We check that in the next ifg_length bytes
 * at lease ifg_length/2 are 0x00
 *
 * We do this because after receiving the frame
 * the receiver must calculate the crc
 */
static int check_ifg(void)
{
    int i, ret, count = 0;

    for (i = 0; i < IFG_LENGTH; ++i) {
        ret = recv_byte();

        if (ret == 0)
            count++;
    }

    if (count < IFG_LENGTH / 2)
        return -1;

    return 0;
}

/* Checks if the receiver is reading
 *
 * When the receiver is reading the lane must be high
 * (since it's keeping the lane high to read)
 */
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

    printf("Waiting receiver to read\n");
    ret = check_receiver_reading();
    if (ret)
        return ret;

    printf("Sending preamble\n");
    ret = send_preamble();
    if (ret)
        return ret;

    printf("Sending data\n");
    ret = do_transmit(frame, FRAME_LENGTH);
    if (ret)
        return ret;

    printf("Waiting for IFG\n");
    return check_ifg();
}

int receive_frame(char *frame)
{
    int i, ret;

    printf("Receiving preamble\n");
    ret = recv_preamble();
    if (ret)
        return ret;

    printf("Receiving data\n");
    for (i = 0; i < FRAME_LENGTH; i++) {
        ret = recv_byte();
        if (ret < 0)
            return ret;

        frame[i] = ret;
    }

    /* Check with upper layer if the frame received is */
    printf("Checking frame with upper layer\n");
    if (phy.check_recv_frame_fn) {
        ret = phy.check_recv_frame_fn(frame);
        if (ret)
            return ret;
    }

    /* Leave IFG after frame
     * We use IFG to acknowledge a frame
     */
    printf("Frame OK. Lane low for IFG\n");
    for (i = 0; i < 8 * IFG_LENGTH; ++i) {
        ret = keep_lane_low(phy.sleep_per_bit_ns);
        if (ret)
            return ret;
    }

    return 0;
}
