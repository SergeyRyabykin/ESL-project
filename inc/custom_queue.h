#ifndef CUSTOM_QUEUE_H__
#define CUSTOM_QUEUE_H__

#include <stdbool.h>
#include "sdk_errors.h"

#define CUSTOM_QUEUE_LENGTH 5
#define CUSTOM_QUEUE_ROOM_SIZE 64

typedef void (*custom_queue_trigger_func_t)(void);

typedef struct {
    char queue[CUSTOM_QUEUE_LENGTH][CUSTOM_QUEUE_ROOM_SIZE];
    int next_index;
    int last_index;
    unsigned int empty_rooms_num;
    bool * volatile is_busy;
    custom_queue_trigger_func_t start_transmission;
} custom_queue_t;

/**
 * @brief Use this macros to initialize queue with correct values before the first usage. 
 * To skip using busy_flag and trigger_func just initialize one of these values (or both) as NULL.
 */
#define CUSTOM_QUEUE_INIT_VALUES(busy_flag_prt, trigger_func_ptr) \
{ \
    .next_index = 0, \
    .last_index = 0, \
    .empty_rooms_num = CUSTOM_QUEUE_LENGTH, \
    .is_busy = busy_flag_prt, \
    .start_transmission = trigger_func_ptr \
}

ret_code_t custom_queue_add(custom_queue_t *queue, const char *str);
ret_code_t custom_queue_get(char *str, custom_queue_t *queue);
bool custom_queue_is_empty(const custom_queue_t *queue);


#endif // CUSTOM_QUEUE_H__