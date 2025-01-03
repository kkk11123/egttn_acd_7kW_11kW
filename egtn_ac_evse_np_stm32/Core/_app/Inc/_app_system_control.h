/*
 * _app_system_control.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef APP_INC__APP_SYSTEM_CONTROL_H_
#define APP_INC__APP_SYSTEM_CONTROL_H_


#include "main.h"
//#include "iwdg.h"

#include <_mw_it.h>

#include <string.h>
#include <_lib_logging.h>
#include <_lib_userdelay.h>
#include <_lib_u8queue.h>
#include <_mw_uart.h>
#include <_mw_eeprom.h>

#define GLOBAL_TIMER_PERIOD		100		//Unit : ms

#define OVER_TEMP_THRESHOLD		75
#define NORMAL_TEMP_THRESHOLD	70

#define MAX_CARDNUM_MEMORY		16	//index : 0~15

//-----------------------------------------MEMROY(EEPROM)---------------------------------------------
#define MEM_ADDR_COMMON_BASE		0x0000
#define MEM_ADDR_NONE0				(MEM_ADDR_COMMON_BASE + 0x0000)
#define MEM_ADDR_NONE1				(MEM_ADDR_COMMON_BASE + 0x0001)
#define MEM_ADDR_NONE2				(MEM_ADDR_COMMON_BASE + 0x0002)
#define MEM_ADDR_NONE3				(MEM_ADDR_COMMON_BASE + 0x0003)
#define MEM_ADDR_NONE4				(MEM_ADDR_COMMON_BASE + 0x0004)
#define MEM_ADDR_NONE5				(MEM_ADDR_COMMON_BASE + 0x0005)
#define MEM_ADDR_NONE6				(MEM_ADDR_COMMON_BASE + 0x0006)
#define MEM_ADDR_NONE7				(MEM_ADDR_COMMON_BASE + 0x0007)

#define MEM_ADDR_CARDNUM_BASE			0x0010
#define MEM_ADDR_CARDNUM_OFFSET			0x0010
#define MEM_ADDR_CARDNUM_INDEX_MSB(X)	((MEM_ADDR_CARDNUM_BASE + ((MEM_ADDR_CARDNUM_OFFSET) * (X))) + 0x0000)
#define MEM_ADDR_CARDNUM_INDEX_LSB(X)	((MEM_ADDR_CARDNUM_BASE + ((MEM_ADDR_CARDNUM_OFFSET) * (X))) + 0x0007)
#define MEM_ADDR_CARDNUM_VALUE_MSB(X)	((MEM_ADDR_CARDNUM_BASE + ((MEM_ADDR_CARDNUM_OFFSET) * (X))) + 0x0008)
#define MEM_ADDR_CARDNUM_VALUE_LSB(X)	((MEM_ADDR_CARDNUM_BASE + ((MEM_ADDR_CARDNUM_OFFSET) * (X))) + 0x000F)

//----------------------------------

//----------------------------------

#if 0
#pragma pack(push, 1)
typedef union {
   uint8_t Raw[4];
   struct {
     unsigned Reseverd0         : 8;			//LSB
     unsigned Reseverd1         : 8;
     struct {
       unsigned Reseverd2_1     : 1;			//LSbit
       unsigned Reseverd2_2   	: 1;
       unsigned Reseverd2_3  	: 1;
       unsigned Reseverd2_4  	: 1;
       unsigned Reseverd2_5  	: 1;
       unsigned Reseverd2_6   	: 1;
       unsigned Reseverd2_7 	: 1;
       unsigned Reseverd2_8     : 1;			//MSbit
     } Reseverd2;
     struct {
       unsigned Reseverd3_1     : 1;			//LSbit
       unsigned Reseverd3_2   	: 1;
       unsigned Reseverd3_3  	: 1;
       unsigned Reseverd3_4  	: 1;
       unsigned Reseverd3_5  	: 1;
       unsigned Reseverd3_6   	: 1;
       unsigned Reseverd3_7 	: 1;
       unsigned Reseverd3_8     : 1;			//MSbit
     } Reseverd3;								//MSB
   };
} Reseverd_info;
#pragma pack(pop)
#endif

//------------System_config_structure------------//


//------------OCPP_config_structure------------//

//------------OTA_config_structure------------//

//------------System_config_structure------------//

//---------------DEBUG CONFIG-------------------
#define CONFIG_RING_BUFFER_SIZE		100
#define CONFIG_MAX_DATA_BUFFER		20
#define CONFIG_WRITE_COMMAND		0x85
#define CONFIG_READ_COMMAND			0x95

//FUNC LIST
/*
 * COMMON_CONFIG
 */
#define CONFIG_FUNC_COMMON_BASE				0x00
#define CONFIG_FUNC_SAVE_CARDNUM_COUNT		((CONFIG_FUNC_COMMON_BASE) + 0x00)
#define CONFIG_FUNC_SAVE_CARDNUM			((CONFIG_FUNC_COMMON_BASE) + 0x01)
#define CONFIG_FUNC_DELETE_CARDNUM			((CONFIG_FUNC_COMMON_BASE) + 0x02)
#define CONFIG_FUNC_COMMON_END				CONFIG_FUNC_DELETE_CARDNUM
/*
 *
 */

/*
 * EVSE CONFIG
 */

/*
 *
 */

/*
 * FLUG CONFIG
 */

/*
 *
 */


/*
 * PM CONFIG
 */

/*
 *
 */

#define _VRMS_IRMS_CALC_LPF_FILTER_		1
#if	((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
#define _LPF_TIMEINTERVAL_ (2)
#define _LPF_TAU_ (800)
#endif
typedef struct s__app_config
{
	uint8_t found_frame;
	uint8_t rx_buf[CONFIG_MAX_DATA_BUFFER];
	uint8_t tx_buf[CONFIG_MAX_DATA_BUFFER];
	uint16_t tx_cnt;

	//uint8_t send_msg_cmd;
	//uint8_t send_msg_length;
	//uint8_t send_msg_data[_MSGQUEUE_MAX_LENGTH_];
	//osMessageQueueId_t send_msg_mq;

	//uint8_t send_msg_flag;		// 1 : msg send enable, 0 ; msg send disable

	uint8_t usecardnum[MAX_CARDNUM_MEMORY][8];

	uint32_t	reg;
}s__APP_CONFIG;

//-------------------------------------

//extern uint16_t zctadc;

void _APP_SYSTEMCTL_write_memory(uint32_t Sector_num, void* value, uint32_t length);
void _APP_SYSTEMCTL_read_memory(uint32_t Sector_num, void* value, uint32_t length);

void _APP_SYSTEMCTL_req(uint8_t cmd, uint8_t func, uint8_t len, uint8_t *data);
void _APP_SYSTEMCTL_push_ringbuffer(uint8_t comdt);
void _APP_SYSTEMCTL_modbusframesearch();
void _APP_SYSTEMCTL_located_in_timer();
uint8_t _APP_SYSTEMCTL_get_ready_print_mode();
uint8_t _APP_SYSTEMCTL_check_cardnum(uint8_t* cardnumbuf);
//------------------------------------------------------

void _APP_SYSTEMCTL_startup();
void _APP_SYSTEMCTL_process();

#endif /* APP_INC__APP_SYSTEM_CONTROL_H_ */
