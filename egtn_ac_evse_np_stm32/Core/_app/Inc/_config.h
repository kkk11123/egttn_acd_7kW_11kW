/*
 * _config.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef APP_INC__CONFIG_H_
#define APP_INC__CONFIG_H_

#define _ON 		0x01
#define _OFF		0x00
#define _CONTINUE	0x02

#define _TRUE 		0x01
#define _FALSE		0x00

#define _FW_VERSION_	5


#define _CERTIFICATION_NON_			0
#define _CERTIFICATION_KC_EVSE_		1
#define _CERTIFICATION_MODE_	(_CERTIFICATION_NON_)

//#define _DEFAULT_AUTOSTART_ACTIVE_		1

#define _FORCE_ACTIVING_MODE_	0	// Force Activing mode with debug port. 0 : Disable , 1 : Enable

#define _PRINT_UNIT_KWH_	1	// print 'KWH' : 1 , don't print 'KWH' : 0

#define _AUTOCHECKUSER_		0	// EVSE,FLAG MODE, 0 : CHECK USER(Default) , 1 : NON CHECK USER

#define _CONNECT_TIMEOUT_		1	//

#define _DO_NOT_RESET_WAKE_UP_SEQ_COUNT_		1

#define _DO_NOT_FAULT_LEAKAGE_					1

#define _FORCESTOPSEQSTART_MUST_TO_AMPE_OUT_		1

#define _CP_ALWAYS_ON_		1	// 0 : CP turns ON after user authentication / 1 : CP Always ON (However, it is turned off during wake up sequence)(Default)

#define _LEAKAGE_CALC_LPF_FILTER_					1

#define BITDIGIT(X)		( 1 << (X) )


typedef enum e_evo_gpo{
	STUS_LED_01,
	STUS_LED_02,
	SLED_R,
	SLED_G,
	SLED_B,
	MC_TURN_ON,
	MC_TURN_OFF,
	CP_RY,
}e_EVO_GPO;

typedef enum e_evo_gpi{
	SET_SW_01,
	SET_SW_02,
	SET_SW_03,
	SET_SW_04,
	WELD,
	EMG
}e_EVO_GPI;

typedef enum e__modecfg{
	evse,
	dev
}e__MODECFG;


#endif /* APP_INC__CONFIG_H_ */
