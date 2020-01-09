#include <stdio.h>
#include <string.h>

#include "l2.h"
#include "checksum.h"
#include "../phy/phy.h"
#include "../utils.h"

char seq_send;
char seq_recv;

static char get_next_seq(char seq) {
    if (seq == FRAME_SEQ_0)
        return FRAME_SEQ_1;

    return FRAME_SEQ_0;
}

static int check_recv_frame_seq(char *frame)
{
    return frame[FRAME_SEQ_START_BYTE] != seq_recv;
}

static int check_recv_frame_fn(char *frame)
{
    return check_recv_frame_seq(frame) ||
            crc_8((const unsigned char *)frame, FRAME_LENGTH) != 0;
}

void init_l2(unsigned long sleep_per_bit_ns,
        unsigned long clocks_per_bit)
{
    init_phy(check_recv_frame_fn, sleep_per_bit_ns, clocks_per_bit);

    seq_send = FRAME_SEQ_0;
    seq_recv = FRAME_SEQ_0;
}

void uninit_l2(void)
{
    uninit_phy();
}

/* Copy, pad, add seq, fcs to frame
 *
 * @returns the updated total length of the buffer
 */
static void prepare_frame(char *frame, char *buf, int len)
{
    unsigned char fcs;

    /* copy and pad */
    memcpy(frame, buf, MIN(len, FRAME_DATA_LEGNTH));
    while (len < FRAME_DATA_LEGNTH)
        frame[len++] = FRAME_PAD;

    /* copy seq */
    memcpy(&frame[FRAME_SEQ_START_BYTE], &seq_send, FRAME_SEQ_LENGH);

    /* calculate and copy crc */
    fcs = crc_8((const unsigned char *) frame, FRAME_DATA_LEGNTH + FRAME_SEQ_LENGH);
    memcpy(&frame[FRAME_FCS_START_BYTE], &fcs, FRAME_FCS_LENGTH);
}

int send(char *buf, int len, int frame_retries)
{
    int k = 0, ret;
    int retries;
    char frame[FRAME_LENGTH];

    retries = frame_retries;
    while (len > 0 && retries) {
        prepare_frame(frame, &buf[k], len);

        ret = transmit_frame(frame);
        if (ret) {
            retries--;
            //printf("Failed to send frame %d\n", ret);
            continue;
        }

        retries = frame_retries;
        seq_send = get_next_seq(seq_send);
        printf("Sent frame\n");

        len -= FRAME_DATA_LEGNTH;
        k += FRAME_DATA_LEGNTH;
    }

    return retries == 0 ? -1 : 0;
}

int recv(char *buf, int len, int frame_retries)
{
    int k = 0, ret;
    int retries;
    char frame[FRAME_LENGTH];

    retries = frame_retries;
    while (len && retries) {
        ret = receive_frame(frame);
        if (ret) {
            retries--;
            //printf("Failed to receive frame %d\n", ret);
            continue;
        }

        retries = frame_retries;
        seq_recv = get_next_seq(seq_recv);
        printf("Received frame\n");

        ret = MIN(len, FRAME_DATA_LEGNTH);
        memcpy(&buf[k], frame, ret);

        len -= ret;
        k += ret;
    }

    return retries == 0 ? -1 : k;
}
