#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "utils.h"

#define MAX_DUPLICATE_ACK 3
int next_seq_to_send(STACK acks, STACK segs);
