#ifndef DCC_A1_L2_H
#define DCC_A1_L2_H

#define FRAME_PAD 0xFF

#define FRAME_SEQ_START_BYTE 8
#define FRAME_SEQ_LENGH 1
#define FRAME_SEQ_0 0x00
#define FRAME_SEQ_1 0xFF

#define FRAME_FCS_START_BYTE 9
#define FRAME_FCS_LENGTH 1
#define FRAME_DATA_LEGNTH \
    (FRAME_LENGTH - FRAME_SEQ_LENGH - FRAME_FCS_LENGTH)

void init_l2(unsigned long sleep_per_bit_ns,
        unsigned long clocks_per_bit);
void uninit_l2(void);

int send(char *buf, int len, int frame_retries);
int recv(char *buf, int len, int frame_retries);

#endif /* DCC_A1_L2_H */
