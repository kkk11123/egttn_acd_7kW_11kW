/*
 * _app_rfid_comm.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef APP_INC__APP_RFID_COMM_H_
#define APP_INC__APP_RFID_COMM_H_


#include "main.h"

#include <_lib_u8queue.h>
#include <_mw_uart.h>

#define RFID_RING_BUFFER_SIZE	50
#define RFID_MAX_TX_BUFFER   40

#define RFID_STX  			0x02
#define RFID_SEQ_REQ		0x01
#define RFID_SEQ_RESP		0x02
//#define RFID_SIDX_H			0xB0
#define RFID_SIDX_H			0x0B
#define RFID_SIDX_L			0x01
#define RFID_RIDX_H			0x0E
#define RFID_RIDX_L			0x01
#define RFID_ETX  			0x03
#define RFID_TERMINATE		0xE3

typedef enum rfid_State
{
    RFID_Init = 1,			//
    RFID_Standby,
    RFID_Start,
	RFID_Tagging,
	RFID_Error,
	RFID_Terminate,
    RFID_Finish
}eRFID_State;

typedef enum e_rfid{
	RFID_STATUS,
	RFID_H_VERSION,
	RFID_L_VERSION,
	RFID_CARD_ID_01,
	RFID_CARD_ID_02,
	RFID_CARD_ID_03,
	RFID_CARD_ID_04,
	RFID_ENUM_TOTAL_COUNT
}e_RFID;

#pragma pack(push, 1)
typedef union
{
	uint8_t Raw;
	struct
	{
		uint8_t init_ok	: 1;
		uint8_t tag_start	: 1;
		uint8_t tag_ok	: 1;
		uint8_t tag_error	: 1;
		uint8_t tag_timeout	: 1;
		uint8_t data_standby	: 1;
		uint8_t read_data	: 1;
		uint8_t tag_terminate	: 1;
	};
}s_RFID_REG;
#pragma pack(pop)

typedef struct s__app_rfid
{
	uint16_t interval_counter;
	uint8_t found_frame;
	uint16_t rfid_rx_buf[RFID_ENUM_TOTAL_COUNT];
	uint8_t rfid_tx_buf[RFID_MAX_TX_BUFFER];
	uint16_t rfid_tx_cnt;

	eRFID_State state;
	s_RFID_REG	reg;
	uint16_t    cardnum[4];

}s__APP_RFID;

void _APP_RFID_push_ringbuffer(uint8_t comdt);
void _APP_RFID_located_in_timer();
void _APP_RFID_req(uint8_t cmd);
uint8_t _APP_RFID_save_card_num(uint16_t* card_num);
uint8_t _APP_RFID_get_stus();
uint32_t _APP_RFID_get_version();

void _APP_RFID_set_start_tagging(uint8_t mode);
uint8_t _APP_RFID_get_tagging_result();
#if 0
void _APP_RFID_get_card_number(char * cardnum);
#else
void _APP_RFID_get_card_number(uint8_t * cardnum);
#endif

uint8_t _APP_RFID_is_init_ok();
void _APP_RFID_startup();
void _APP_RFID_comm_process();


#endif /* APP_INC__APP_RFID_COMM_H_ */
