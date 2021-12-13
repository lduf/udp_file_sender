#include "includes/tcp_mechanism.h"
#include "includes/utils.h"

double sRtt = 500;
double devRtt = 100;

int swnd = 1; // fenêtre d'émission
int awnd = 1; // fenêtre de réception
int cwnd = DEFAULT_CWND; // fenêtre de congestion
int ssthresh = 1024; // seuil de congestion slow start
int FAST_RETRANSMIT_MOD = 0; //mode fast retransmit
int fast_rt_window  = DEFAULT_CWND; // fenêtre de congestion fast retransmit


/**
 * @brief This function is used to give the next packet to send. It implement the fast retransmission mechanism.
 * 
 * @param last_ack The last ACK received
 * @param last_seq The last sequence number sent
 * @param timeout this is a flag to indicate if the timeout occured
 * @param eof notify if the end of file is reached
 * 
 * @return int The next sequence number to send
 */
int next_seq_to_send(STACK acks, STACK segs, int timedout, int eof) {

    int last_ack = acks->element;
    int last_seg = segs->element;

    if(eof == 1) {
        return last_ack+1;
    }
    if(timedout == 1) {
        if(last_ack < last_seg) {
            return last_ack+1;
        }
        else {
            return last_seg+1;
        }
        //return last_seg +1 ; //last_seg +1 ; //what if ACK + 1 ? -> perf -- 
    }

    if (acks->duplicate > MAX_DUPLICATE_ACK && last_seg != last_ack +1) {
       // printf("Too many duplicate ACKs, sending back the segment n° %d...\n", last_ack+1);
        return last_ack + 1;
    } else {
        if(last_ack > last_seg && last_seg >= 1) {
            //printf("Duplicate ACK resolved, sending the next segment n° %d...\n", last_ack+1);
            return last_ack + 1;
        } else {
            return last_seg + 1;
        }
    }
} 

/**
 * @brief This function is used to estimate the timeout based on the RTT.
 * 
 * @param rtt The last mesured rtt
 * 
 * @return int The new timeout
 */
int estimate_timeout(double rtt) {
    sRtt = (1 - ALPHA) * sRtt + ALPHA*rtt;
    devRtt = (1 - BETA) * devRtt + BETA * fabs(rtt - sRtt);
    //printf("Estimated RTT: %f us\n", sRtt);
    return (int) (sRtt + 4 * devRtt);
}

/**
 * @brief This function is used to implement the slow start mechanism.
 * 
 * @param int positive_ack
 * @return void The number of segments to send
 */
void slow_start(int positive_ack) {
    if(positive_ack > 0) 
        cwnd = cwnd *2;
}

/**
 * @brief This function is used to implement the congestion avoidance mechanism.
 * 
 */
void congestion_avoidance() {
    cwnd = cwnd + 1;

    //CHECK IF FRT
    if(FAST_RETRANSMIT_MOD == 1){
        ssthresh = cwnd;
        cwnd = (cwnd/2 > DEFAULT_CWND) ? cwnd/2 : DEFAULT_CWND;
        FAST_RETRANSMIT_MOD = 0;
    }
}

/**
 * @brief This function is used to handle the fast retransmit mechanism.
 * 
 *
 *
*/
void fast_retransmit() {
    FAST_RETRANSMIT_MOD = 1;
    fast_rt_window = cwnd;
    cwnd = DEFAULT_CWND;
}

/**
 * @brief This function is used to implement the fast recovery mechanism.
 *
 * 
 */
void fast_recovery() {
    if(FAST_RECOVERY_MOD == 1){
        cwnd = (fast_rt_window / 2 >= DEFAULT_CWND) ? fast_rt_window / 2 : DEFAULT_CWND;
    }
    else{
        cwnd = DEFAULT_CWND;
    }
    //cwnd = ssthresh;
}

/**
 * @brief This function is used to calculate the new window size based on the number of segments received.
 * 
 * @param stack segs 
 * @param stack acks
 * @param int positive_ack : the number of positive ACKs received, 0 if all ACK are received
 *
 * @return int Window size
 */
 int new_window_size(STACK segs, STACK acks, int positive_ack) {
     if (swnd < ssthresh){
         slow_start(positive_ack);
     } 
     if (acks->duplicate > MAX_DUPLICATE_ACK ) { //&& cwnd >= ssthresh
         fast_retransmit();
     } 
     if (cwnd >= ssthresh) {
         congestion_avoidance();
     }  
     if (FAST_RECOVERY_MOD == 1) {
         fast_recovery();
     }
    
     swnd = cwnd;//min(cwnd, awnd);
     return swnd;
 }