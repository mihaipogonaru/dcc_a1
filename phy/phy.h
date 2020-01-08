#ifndef DCC_A1_PHY_H
#define DCC_A1_PHY_H

void init_phy(unsigned long sleep_per_bit_ns,
        unsigned long th_clocks_per_bit);
void uninit_phy(void);

/* Transmits from buf, len */
int transmit(char *buf, int len);

/* Receives in buf, len */
int receive(char *buf, int max_len, int retries);

#endif /* DCC_A1_PHY_H */
