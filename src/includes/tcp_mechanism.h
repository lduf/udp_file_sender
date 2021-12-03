#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stack.h"
#include "utils.h"

#define MAX_DUPLICATE_ACK 3
#define ALPHA 0.4
#define BETA 0.25
#define DEFAULT_CWND 1

#define FAST_RECOVERY_MOD 1
int next_seq_to_send(STACK acks, STACK segs, int timedout, int eof);
int estimate_timeout(double rtt);