#include "includes/stack.h"


/**
 * @brief This function is used to initialize the stack
 * 
 * @return stack The stack
 */
STACK stack_init() {

    STACK stack = (STACK) NULL;
	return(stack);
}

/**
 * @brief This function is used to push an element on the stack
 * 
 * @param stack The stack
 * @param element The element to push
 * 
 * @return stack The stack
 */
STACK stack_push(STACK stack, int element) {
    STACK new_stack = (STACK) malloc(sizeof(ELEMSTACK));
    new_stack->element = element;
    new_stack->mode = (stack != NULL) ? stack->mode : 0;
    new_stack->next = stack;
    
    int duplicate = 0;
    // if(stack != NULL) {
    //     if(stack->element == element) {
    //         duplicate = stack->duplicate;
    //         duplicate = duplicate + 1;
    //     }
    // }
    int resent_packet = element;
    int resent_duplicate = 0;
    while(stack != NULL) {
        if(stack->mode == 'A' && stack->element > resent_packet) {
            resent_packet = stack->element;
            resent_duplicate = stack->duplicate;
        }
        if(stack->element == element) {
            duplicate++;
        }
        stack = stack->next;
    }
    if(resent_packet != 0) {
        new_stack->element = resent_packet;
        new_stack->duplicate = resent_duplicate;
    }
    else{
        new_stack->duplicate = duplicate;
    }
    
    if(stack_size(new_stack) >= MAX_STACK_SIZE) {
        new_stack = stack_pop_last(new_stack);
    }
    return new_stack;
}

/**
 * @brief This function is used to pop an element from the stack
 * 
 * @param stack The stack
 * 
 * @return stack The stack
 */
STACK stack_pop(STACK stack) {
    STACK new_stack = stack->next;
    free(stack);
    return new_stack;
}

/**
 * @brief This function is used to remove the last element of the stack
 * 
 * @param stack The stack
 * 
 * @return stack The stack
 */
STACK stack_pop_last(STACK stack) {
    STACK new_stack = stack;
    while (new_stack->next->next != NULL) {
        new_stack = new_stack->next;
    }
    free(new_stack->next);
    new_stack->next = NULL;
    return stack;
}

/**
 * @brief This function is used to get the i est element of the stack
 * 
 * @param stack The stack
 * @param i The index of the element to get
 * 
 * @return int The element
 */
STACK stack_get(STACK stack, int i) {
    int j = 0;
    STACK new_stack = stack;
    while (new_stack != NULL && j < i) {
        new_stack = new_stack->next;
        j++;
    }
    if (new_stack != NULL) {
        return new_stack;
    } else {
        return NULL;
    }
}


/**
 * @brief This function is used to get the size of the stack
 * 
 * @param stack The stack
 * 
 * @return int The size of the stack
 */
int stack_size(STACK stack) {
    int i = 0;
    STACK new_stack = stack;
    while (new_stack != NULL) {
        new_stack = new_stack->next;
        i++;
    }
    return i;
}

/**
 * @brief This function is used to returns all the elements of the stack as an array of int
 * 
 * @param stack The stack
 * 
 * @return int* The array of int
 */
int* stack_to_array(STACK stack) {
    int* array = (int*) malloc(sizeof(int) * stack_size(stack));
    int i = 0;
    STACK new_stack = stack;
    while (new_stack != NULL) {
        array[i] = new_stack->element;
        new_stack = new_stack->next;
        i++;
    }
    return array;
}

/**
 * @brief This function is used to print the stack
 * 
 * @param stack The stack
 */
void stack_print(STACK stack) {
    STACK new_stack = stack;
    printf("%c : ", stack->mode);
    while (new_stack != NULL) {
        printf("%d (%d) %d| ", new_stack->element, new_stack->duplicate, new_stack->RTT);
        new_stack = new_stack->next;
    }
    printf("\n");
}