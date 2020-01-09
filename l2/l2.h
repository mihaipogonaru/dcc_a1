#ifndef DCC_A1_L2_H
#define DCC_A1_L2_H

#define FRAME_PAD 0x00

void init_l2(unsigned long sleep_per_bit_ns,
        unsigned long clocks_per_bit);
void uninit_l2(void);

int send(char *buf, int len);
int recv(char *buf, int len);

#endif /* DCC_A1_L2_H */
