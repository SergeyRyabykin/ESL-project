#ifndef CUSTOM_VPTR_QUEUE_H
#define CUSTOM_VPTR_QUEUE_H

#include <stdbool.h>
#include "sdk_errors.h"

#define CUSTOM_VPTR_QUEUE_LENGTH 5

typedef void (*custom_vptr_queue_handler_t)(void);

typedef struct {
    void *queue[CUSTOM_VPTR_QUEUE_LENGTH];
    int next_index;
    int last_index;
    unsigned int empty_rooms_num;
    volatile bool *is_busy;
    custom_vptr_queue_handler_t handle_target;
} custom_vptr_queue_t;

/**
 * @brief Use this macros to initialize queue with correct values before the first usage. 
 * To skip using busy_flag and trigger_func just initialize one of these values (or both) as NULL.
 */
#define CUSTOM_VPTR_QUEUE_INIT_VALUES(busy_flag, handler) \
{ \
    .next_index = 0, \
    .last_index = 0, \
    .empty_rooms_num = CUSTOM_VPTR_QUEUE_LENGTH, \
    .is_busy = busy_flag, \
    .handle_target = handler \
}

ret_code_t custom_vptr_queue_add(custom_vptr_queue_t *queue, void *target);
ret_code_t custom_vptr_queue_get(void **target, custom_vptr_queue_t *queue);
ret_code_t custom_vptr_queue_read(void *target, custom_vptr_queue_t *queue);
bool custom_vptr_queue_is_empty(const custom_vptr_queue_t *queue);


#endif // CUSTOM_VPTR_QUEUE_H