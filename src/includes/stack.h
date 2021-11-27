#include <stdio.h>
#include <stdlib.h>
#include "stack_type.h"
#define MAX_STACK_SIZE 10

STACK stack_init();
STACK stack_push(STACK stack, int element);
STACK stack_pop(STACK stack);
STACK stack_pop_last(STACK stack);
STACK stack_get(STACK stack, int i);
int stack_size(STACK stack);
int* stack_to_array(STACK stack);
