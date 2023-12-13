#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include "custom_queue.h"


int custom_queue_add(custom_queue_t *queue, char *str)
{
    int ret = 0;
    size_t length = strlen(str);
    unsigned int rooms_number = (length / CUSTOM_QUEUE_ROOM_SIZE) + ((length % CUSTOM_QUEUE_ROOM_SIZE) ? 1 : 0);

    if(queue->empty_rooms_num >= rooms_number) {
        for(unsigned int i = 0; i < rooms_number; i++) {
            strncpy(queue->queue[queue->next_index], (str + CUSTOM_QUEUE_ROOM_SIZE * i), CUSTOM_QUEUE_ROOM_SIZE);
            queue->empty_rooms_num--;
            if(CUSTOM_QUEUE_LENGTH <= ++queue->next_index) {
                queue->next_index = 0;
            }
        }
    }
    else {
        ret = -1;
    }

    return ret;
}

int custom_queue_get(char *str, custom_queue_t *queue)
{
    int ret = 0;
    if(CUSTOM_QUEUE_LENGTH > queue->empty_rooms_num) {
        strncpy(str, queue->queue[queue->last_index], CUSTOM_QUEUE_ROOM_SIZE);
        queue->empty_rooms_num++;
        if(CUSTOM_QUEUE_LENGTH <= ++queue->last_index) {
            queue->last_index = 0;
        }
    }
    else {
        ret = -1;
    }

    return ret;
}

bool custom_queue_is_empty(custom_queue_t *queue)
{
    return (CUSTOM_QUEUE_LENGTH == queue->empty_rooms_num);
}


