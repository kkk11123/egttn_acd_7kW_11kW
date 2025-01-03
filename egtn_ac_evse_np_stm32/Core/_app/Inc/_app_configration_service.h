/*
 * _app_configration_service.h
 *
 *  Created on: 2023. 3. 16.
 *      Author: MSI
 */

#ifndef APP_INC__APP_CONFIGRATION_SERVICE_H_
#define APP_INC__APP_CONFIGRATION_SERVICE_H_

#include "main.h"

typedef enum EConfig_State
{
    C_Init = 1,			//
	C_Ready,
	C_Setting,
	C_Finish,
	C_Fault,
	Configstateend
}eConfig_State;

//------------Config_reg_structure------------//
#pragma pack(push, 1)
typedef union {
   uint64_t Raw;
   struct {
	   uint64_t enable							: 1;
	   uint64_t init_proceeding     			: 1;
	   uint64_t init_complete       			: 1;
	   uint64_t ready_standby        			: 1;
	   uint64_t ready_cardtag         			: 1;
	   uint64_t ready_cardtagfail				: 1;
	   uint64_t setting_proceeding     			: 1;
	   uint64_t setting_ok					    : 1;
	   uint64_t setting_fail					: 1;
	   uint64_t finish_proceeding       		: 1;
	   uint64_t finish_ok       				: 1;
	   uint64_t fault_set         				: 1;
	   uint64_t fault_reset         			: 1;
	   uint64_t state_changed      				: 1;
	   uint64_t is_userconfigmode				: 1;
   };
} e_CONFIGSERV_REG;
#pragma pack(pop)

typedef struct
{
	eConfig_State	state;
	eConfig_State	state_bk;
	e_CONFIGSERV_REG reg;

	uint8_t			read_card_num[30];

}sConfig;

#define CONFIGSERV_READ_MODE_ENABLE				0//((0 == GPI_SET_SW_01_IN))

uint8_t _APP_CONFIGSERV_flag(uint8_t value, uint8_t is_userconfig);
void _APP_CONFIGSERV_startup();
void _APP_CONFIGSERV_init();
void _APP_CONFIGSERV_process();

#endif /* APP_INC__APP_CONFIGRATION_SERVICE_H_ */
