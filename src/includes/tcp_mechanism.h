#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stack.h"
#include "utils.h"
 
#define MAX_DUPLICATE_ACK 2
#define ALPHA 0.02
#define BETA 0.01
#define DEFAULT_CWND 1

int next_seq_to_send(STACK acks, STACK segs, int timedout, int eof);
int estimate_timeout(double rtt);
void slow_start(int positive_ack);
void congestion_avoidance() ;
void fast_retransmit() ;
void fast_recovery();
int new_window_size(STACK segs, STACK acks, int positive_ack, int timedout);