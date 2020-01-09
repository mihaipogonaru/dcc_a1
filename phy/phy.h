#ifndef DCC_A1_PHY_H
#define DCC_A1_PHY_H

#define PRAMBLE_LENGTH 2
#define FRAME_LENGTH 10
#define IFG_LENGTH 4

/* Upper layer can tell us if the received frame is ok */
typedef int (*check_recv_frame)(char *frame);

void init_phy(check_recv_frame check_recv_frame_fn,
        unsigned long sleep_per_bit_ns,
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
