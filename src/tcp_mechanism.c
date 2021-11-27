#include "includes/tcp_mechanism.h"

/**
 * @brief This function is used to give the next packet to send. It implement the fast retransmission mechanism.
 * 
 * @param last_ack The last ACK received
 * @param last_seq The last sequence number sent
 * 
 * @return int The next sequence number to send
 */
int next_seq_to_send(STACK acks, STACK segs) {

    int last_ack = acks->element;
    int last_seg = segs->element;

    if (acks->duplicate > MAX_DUPLICATE_ACK) {
        printf("Too many duplicate ACKs, sending back the segment n° %d...\n", last_ack+1);
        return last_ack + 1;
    } else {
        if(last_ack > last_seg) {
            printf("Duplicate ACK resolved, sending the next segment n° %d...\n", last_ack+1);
            return last_ack + 1;
        } else {
            return last_seg + 1;
        }
    }
} 
