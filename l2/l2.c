#include <stdio.h>
#include <string.h>

#include "l2.h"
#include "checksum.h"
#include "../phy/phy.h"
#include "../utils.h"

void init_l2(unsigned long sleep_per_bit_ns,
        unsigned long clocks_per_bit)
{
    init_phy(sleep_per_bit_ns, clocks_per_bit);
}

void uninit_l2(void)
{
    uninit_phy();
}

static int copy_and_pad_frame(char *frame, char *buf, int len)
{
    memcpy(frame, buf, MIN(len, FRAME_LENGTH));
    while (len < FRAME_LENGTH)
        frame[len++] = FRAME_PAD;

    return len;
}

int send(char *buf, int len)
{
    int k = 0, ret;
    char frame[FRAME_LENGTH];

    while (len) {
        len = copy_and_pad_frame(frame, &buf[k], len);

        ret = transmit_frame(frame);
        if (ret) {
            printf("Failed to send frame %d\n", ret);
            continue;
        }
        printf("Sent frame\n");

        len -= FRAME_LENGTH;
        k += FRAME_LENGTH;
    }

    return 0;
}

int recv(char *buf, int len)
{
    int k = 0, ret;
    char frame[FRAME_LENGTH];

    while (len) {
        ret = receive_frame(frame);
        if (ret) {
            printf("Failed to receive frame %d\n", ret);
            continue;
        }
        printf("Received frame\n");

        ret = MIN(len, FRAME_LENGTH);
        memcpy(&buf[k], frame, ret);

        len -= ret;
        k += ret;
    }

    return k;
}
