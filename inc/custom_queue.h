#ifndef CUSTOM_QUEUE_H__
#define CUSTOM_QUEUE_H__

#include <stdbool.h>
#include "sdk_errors.h"

#define CUSTOM_QUEUE_LENGTH 20
#define CUSTOM_QUEUE_ROOM_SIZE 64

typedef struct {
    char queue[CUSTOM_QUEUE_LENGTH][CUSTOM_QUEUE_ROOM_SIZE];
    int next_index;
    int last_index;
    unsigned int empty_rooms_num;
} custom_queue_t;

#define CUSTOM_QUEUE_INIT_VALUES \
{\
    .next_index = 0,\
    .last_index = 0,\
    .empty_rooms_num = CUSTOM_QUEUE_LENGTH\
}

ret_code_t custom_queue_add(custom_queue_t *queue, const char *str);
ret_code_t custom_queue_get(char *str, custom_queue_t *queue);
bool custom_queue_is_empty(const custom_queue_t *queue);


#endif // CUSTOM_QUEUE_H__