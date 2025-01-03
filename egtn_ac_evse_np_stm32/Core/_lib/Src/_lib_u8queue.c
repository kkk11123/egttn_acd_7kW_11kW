/*
 * _lib_u8queue.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */



#include <_lib_u8queue.h>

void _LIB_U8QUEUE_comFlush(s__LIB_QUEUE* s_queue)
{
	//s_queue->action_pointer = RING_BUFFER_MAX;
	s_queue->action_pointer = s_queue->ring_buffer_size;
	s_queue->save_pointer = 0;
}

uint16_t _LIB_U8QUEUE_comLen(s__LIB_QUEUE* s_queue)
{
	uint16_t i;
	if(s_queue->save_pointer > (s_queue->action_pointer + 1)){
		i = s_queue->save_pointer - s_queue->action_pointer - 1;
	}
	else if(s_queue->save_pointer < (s_queue->action_pointer+1)){
		i = s_queue->ring_buffer_size - s_queue->action_pointer + s_queue->save_pointer;
	}
	else{
		i = 0;
	}
	return i;
}

uint8_t read_ringbuffer(s__LIB_QUEUE* s_queue)
{
	s_queue->action_pointer++;
	if(s_queue->action_pointer > s_queue->ring_buffer_size)	{
		s_queue->action_pointer = 0;
	}
	return (s_queue->ring_buffer[s_queue->action_pointer]);
}

uint8_t _LIB_U8QUEUE_get_byte(s__LIB_QUEUE* s_queue)
{
	return read_ringbuffer(s_queue);
}

uint16_t _LIB_U8QUEUE_get_word(s__LIB_QUEUE* s_queue)
{
	uint16_t wh = 0x0000;
	uint16_t wl = 0x0000;
	uint16_t temp = 0;

	wh = read_ringbuffer(s_queue);
	wl = read_ringbuffer(s_queue);

	temp = ((wh << 8) & 0xFF00) | (wl & 0x00FF);

	return temp;
}

void _LIB_U8QUEUE_inc_pointer(s__LIB_QUEUE* s_queue)
{
	read_ringbuffer(s_queue);
}

void _LIB_U8QUEUE_push_ring(s__LIB_QUEUE* s_queue, uint8_t comdt)
{
#if 0
	if(s_queue->blank_check > 50)	{
		_LIB_U8QUEUE_comFlush(s_queue);
	}
	s_queue->blank_check = 0;
#endif

	if(s_queue->save_pointer != s_queue->action_pointer){
		s_queue->ring_buffer[s_queue->save_pointer] = comdt;
		s_queue->save_pointer++;
		if(s_queue->save_pointer > s_queue->ring_buffer_size){
			s_queue->save_pointer = 0;
		}
	}
}




