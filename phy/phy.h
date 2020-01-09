#ifndef DCC_A1_PHY_H
#define DCC_A1_PHY_H

#define PRAMBLE_LENGTH 2
#define FRAME_LENGTH 8
#define IFG_LENGTH 2

void init_phy(unsigned long sleep_per_bit_ns,
        unsigned long clocks_per_bit);
void uninit_phy(void);

/* Transmits a frame
 *
 * frame must be a buffer of at least FRAME_LENGTH bytes
 */
int transmit_frame(char *frame);

/* Receives a frame
 *
 * frame must be a buffer of at least FRAME_LENGTH bytes
 */
int receive_frame(char *frame);

#endif /* DCC_A1_PHY_H */
