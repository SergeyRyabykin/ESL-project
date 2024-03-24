#include <stdbool.h>
#include "custom_vptr_queue.h"

ret_code_t custom_vptr_queue_add(custom_vptr_queue_t *queue, void *target)
{
    int ret = NRF_SUCCESS;

    if(queue->empty_rooms_num) {
        queue->queue[queue->next_index] = target;
        queue->empty_rooms_num--;
        queue->next_index++;


        if(CUSTOM_VPTR_QUEUE_LENGTH <= queue->next_index) {
            queue->next_index = 0;
        }
    }
    else {
        ret = NRF_ERROR_NO_MEM;
    }

    if(queue->is_busy && queue->handle_target && false == *queue->is_busy) {
        queue->handle_target();
    }

    return ret;
}

ret_code_t custom_vptr_queue_get(void **target, custom_vptr_queue_t *queue)
{
    int ret = NRF_SUCCESS;

    if(CUSTOM_VPTR_QUEUE_LENGTH > queue->empty_rooms_num) {
        *target = queue->queue[queue->last_index];
        queue->last_index++;

        if(CUSTOM_VPTR_QUEUE_LENGTH <= queue->last_index) {
            queue->last_index = 0;
        }
        
        queue->empty_rooms_num++;
    }
    else {
        ret = NRF_ERROR_NOT_FOUND;
    }

    return ret;
}

ret_code_t custom_vptr_queue_read(void *target, custom_vptr_queue_t *queue)
{
    int ret = NRF_SUCCESS;
    
    if(CUSTOM_VPTR_QUEUE_LENGTH > queue->empty_rooms_num) {
        target = queue->queue[queue->last_index];
    }
    else {
        ret = NRF_ERROR_NOT_FOUND;
    }

    return ret;
}


bool custom_vptr_queue_is_empty(const custom_vptr_queue_t *queue)
{
    return (CUSTOM_VPTR_QUEUE_LENGTH <= queue->empty_rooms_num);
}


