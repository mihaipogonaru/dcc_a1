#ifndef DCC_A1_PHY_H
#define DCC_A1_PHY_H

#define PRAMBLE_LENGTH 4
#define FRAME_LENGTH 16
#define IFG_LENGTH 4

void init_phy(unsigned long sleep_per_bit_ns,
        unsigned long th_clocks_per_bit);
void uninit_phy(void);

/* Transmits a frame of from buf */
int transmit_frame(char *buf);

/* Receives a frame of from buf */
int receive_frame(char *buf);

#endif /* DCC_A1_PHY_H */
