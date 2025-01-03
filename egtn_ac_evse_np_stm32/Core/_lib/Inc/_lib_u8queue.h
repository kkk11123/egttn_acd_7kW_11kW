/*
 * _lib_u8queue.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef LIB_INC__LIB_U8QUEUE_H_
#define LIB_INC__LIB_U8QUEUE_H_



#include "main.h"

//#define RING_BUFFER_MAX 1024
#define RETURN_INTERVAL 1

typedef struct s__lib_queue{
	//uint8_t ring_buffer[RING_BUFFER_MAX + 1];
	uint8_t *ring_buffer;
	uint16_t ring_buffer_size;
	uint16_t save_pointer;
	uint16_t action_pointer;
	uint16_t blank_check;
}s__LIB_QUEUE;


void _LIB_U8QUEUE_push_ring(s__LIB_QUEUE* s_queue, uint8_t comdt);
uint16_t _LIB_U8QUEUE_get_word(s__LIB_QUEUE* s_queue);
uint8_t _LIB_U8QUEUE_get_byte(s__LIB_QUEUE* s_queue);
uint16_t _LIB_U8QUEUE_comLen(s__LIB_QUEUE* s_queue);
void _LIB_U8QUEUE_comFlush(s__LIB_QUEUE* s_queue);
void _LIB_U8QUEUE_inc_pointer(s__LIB_QUEUE* s_queue);



#endif /* LIB_INC__LIB_U8QUEUE_H_ */
