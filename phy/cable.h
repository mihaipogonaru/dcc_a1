#ifndef DCC_A1_CABLE_H
#define DCC_A1_CABLE_H

void init_cable(unsigned long sleep_per_bit_ns,
        unsigned long th_clocks_per_bit);
void uninit_cable(void);

int send_bit(char bit);
int recv_bit(void);
int lane_is_high(void);

#endif /* DCC_A1_CABLE_H */
