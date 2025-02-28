/*
 * _app_charging_service.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef APP_INC__APP_CHARGING_SERVICE_H_
#define APP_INC__APP_CHARGING_SERVICE_H_


#include "main.h"
#include "math.h"

#include <_mw_gpio.h>
#include <_mw_indiled.h>
#include <_mw_cp.h>
#include <_lib_bitop.h>
#include <_lib_logging.h>
#include <_lib_userdelay.h>
//#include <_lib_LPF.h>
#include <string.h>

#define __CHARGSERV_DEBUG__			1

typedef enum ECharger_State
{
    Init = 1,			
    Ready,
	AutoReady,
    UserCheck,
    Connect,
    Charging,
    Finish,
	OTA,
	Fault, 
	Powercut,
	Chargerstateend
}eCharger_State;

typedef enum ECharger_Mode
{
	mode_3KW,
	mode_5KW,
	mode_7KW,
	mode_11KW,
	mode_config
}eCharger_Mode;

typedef enum e__stnd_wake_up_seq{
	NON,
	START_TIMEOUT,
	CP_OFF,
	PWM_DISABLE,
	FINISH,
}e__STND_WAKE_UP_SEQ;

//220802 PES :
typedef enum ECharger_Charging_state
{
	Charg_Standby,
	Charg_Start,
	Charg_Stable,
	Charg_Full
}eCharger_Charging_State;

#if 0
typedef enum ECharger_fault
{
	MC_START_ERR,
	MC_STOP_ERR,
	AC_OC_ERR,
	AC_OV_ERR,
	AC_UV_ERR,
	CP_ERR,
	OTEMP_ERR,
	DM_COMM_ERR,
	CSMS_COMM_ERR,
	NON_ERR,
	ERR_END
}eCharger_Fault;
#endif

//------------Charger_fault_status_structure------------//
#pragma pack(push, 1)
typedef union {
   uint32_t Raw;
   struct {
	   uint32_t LEAKAGE_ERR       : 1;
	   uint32_t MC_START_ERR      : 1;
	   uint32_t MC_STOP_ERR       : 1;
	   uint32_t AC_OC_ERR         : 1;
	   //uint32_t AC_LV_ERR         : 1;
	   uint32_t AC_OV_ERR         : 1;
	   uint32_t AC_UV_ERR		  : 1;
	   uint32_t CP_ERR     	    	 : 1;
	   //uint32_t OTEMP_ERR         : 1;
	   uint32_t DM_COMM_ERR       : 1;
	   uint32_t CSMS_COMM_ERR     : 1;
	   uint32_t RFID_COMM_ERR      : 1;
	   uint32_t EMG_INPUT        : 1;
	   uint32_t CP_VOLTAGE_ERR    : 1;
	   uint32_t Reseverd13        : 1;
	   uint32_t Reseverd14        : 1;
	   uint32_t Reseverd15        : 1;
   };
} eCharger_Fault;

extern int TEMP_ERR; //_app_system_control.c 파일에서 불러오기 위해 헤더파일에 선언

#pragma pack(pop)
#define CHARGSERV_FAULT_STATUS_LENGTH	32

//------------Charger_reg_structure------------//
#pragma pack(push, 1)
typedef union {
   uint64_t Raw;
   struct {
	   uint64_t dev_flag       					: 1;
	   uint64_t init_proceeding     			: 1;
	   uint64_t init_complete       			: 1;
	   uint64_t ready_standby        			: 1;
	   uint64_t ready_cardtag         			: 1;
	   uint64_t ready_ota_request         		: 1;
	   uint64_t ready_nontag_overpower     	    : 1;
	   uint64_t ready_remote_start				: 1;
	   uint64_t ready_automodeflag				: 1;
	   uint64_t autoready_standby				: 1;
	   uint64_t autoready_manualmodeflag		: 1;
	   uint64_t autoready_connected				: 1;
	   uint64_t usercheck_tagok         		: 1;
	   uint64_t usercheck_serverconnecting      : 1;
	   uint64_t usercheck_authorize_flag		: 1;
	   uint64_t usercheck_userok     			: 1;
	   uint64_t usercheck_userretry        		: 1;
	   uint64_t connect_wait        			: 1;
	   uint64_t connect_wired        			: 1;
	   uint64_t connect_timeout        			: 1;
	   uint64_t charging_proceeding        		: 1;
	   uint64_t charging_is_tagging				: 1;
	   uint64_t charging_id_tagging_stop		: 1;
	   uint64_t charging_ok        				: 1;
	   uint64_t charging_compulsionunplug		: 1;
	   uint64_t charging_continuous				: 1;
	   uint64_t charging_remote_stop			: 1;
	   uint64_t charging_remote_stop_check		: 1;
	   uint64_t finish_proceeding       		: 1;
	   uint64_t finish_stoptranscation_flag		: 1;
	   uint64_t finish_ok       				: 1;
	   uint64_t fault_set         				: 1;
	   uint64_t fault_reset         			: 1;
	   uint64_t ota_proceeding         			: 1;
	   uint64_t ota_ok     	    				: 1;
	   uint64_t powercut_proceeding         	: 1;
	   uint64_t powercut_timepass       		: 1;
	   uint64_t state_changed      				: 1;
	   uint64_t display_enable        			: 1;
	   uint64_t display_cyclefinish        		: 1;
	   uint64_t weld_flag						: 1;
   };
} e_CHARGSERV_REG;
#pragma pack(pop)

//------------Charger_init_structure------------//
#pragma pack(push, 1)
typedef union {
   uint8_t Raw;
   struct {
	   uint8_t rfid_init_ok      	: 1;
	   uint8_t systemctl_init_ok    : 1;
	   uint8_t temp0         		: 1;
	   uint8_t temp1        		: 1;
	   uint8_t temp2     	     	: 1;
	   uint8_t temp3	    		: 1;
   };
} e_CHARGSERV_INIT_FLAG;
#pragma pack(pop)
#define CHARGSERV_INIT_FLAG_INIT_ALL_OK		(0x0F)

#if 0
#define CHARGING_REG_DISP_ENABLE		(1 << 6)
#define CHARGING_REG_DISP_CYCLEFINISH	(1 << 7)

#define CHARGSERV_REG_INIT_PROCEEDING	(1 << 0)
#define CHARGSERV_REG_INIT_COMPLETE		(1 << 1)

#define CHARGSERV_REG_READY_STANDBY		(1 << 0)
#define CHARGSERV_REG_READY_CARDTAG		(1 << 1)
#define CHARGSERV_REG_READY_OTAREQ		(1 << 2)
#define CHARGSERV_REG_READY_NONTAG_OVERPOWER	(1 << 3)

#define CHARGSERV_REG_USERCHECK_TAGOK				(1 << 0)
#define CHARGSERV_REG_USERCHECK_SERVERCONNECTING	(1 << 1)
#define CHARGSERV_REG_USERCHECK_USEROK				(1 << 2)
#define CHARGSERV_REG_USERCHECK_USERRETRY			(1 << 3)

#define CHARGSERV_REG_CONNECT_WAIT				(1 << 0)
#define CHARGSERV_REG_CONNECT_WIRED				(1 << 1)
#define CHARGSERV_REG_CONNECT_TIMEOUT			(1 << 2)

#define CHARGSERV_REG_CHARGING_PROCEEDING		(1 << 0)
#define CHARGSERV_REG_CHARGING_OK				(1 << 1)

#define CHARGSERV_REG_FINISH_PROCEEDING		(1 << 0)
#define CHARGSERV_REG_FINISH_DEBT			(1 << 1)
#define CHARGSERV_REG_FINISH_OK				(1 << 2)

#define CHARGSERV_REG_FAULT_SET				(1 << 0)
#define CHARGSERV_REG_FAULT_RESET			(1 << 1)

#define CHARGSERV_REG_OTA_PROCEEDING		(1 << 0)
#define CHARGSERV_REG_OTA_OK				(1 << 1)

#define CHARGSERV_REG_POWERCUT_PROCEEDING		(1 << 0)
#define CHARGSERV_REG_POWERCUT_TIMEPASS			(1 << 1)
#endif

#define CHARGSERV_BEEP_ON_TIME			100		//unit : freertos tick(1ms)
#define CHARGSERV_DUTY_ONDELAY			200		//unit : freertos tick(1ms)
#define CHARGSERV_WUSQ_NORMAL_TIMEOUT			30000
#define CHARGSERV_WUSQ_CAHRGING_TIMEOUT			120000//40000//
#define CHARGSERV_WUSQ_CAHRGING3_TIMEOUT		600000//50000//
#define CHARGSERV_WUSQ_CAHRGING4_TIMEOUT		1800000//60000//
#define CHARGSERV_WUSQ_LEAKAGE_TIMEOUT			5000
#define CHARGSERV_WUSQ_CP_OFF			3000
#define CHARGSERV_WUSQ_PWM_OFF			800

#define CHARGSERV_FLUG_LOAD_DETECT_VALUE		20
#define CHARGSERV_EVSE_LOAD_DETECT_VALUE		500

//#define CHARGSERV_AMPE_MAX_VALUE			13000
//#define CHARGSERV_AMPE_MAX_HALF_VALUE		(CHARGSERV_AMPE_MAX_VALUE / 2)
#define CHARGSERV_AMPE_MIN_VALUE			4000

//#define CHARGSERV_OVERPOWER_VALUE		130000000UL
//#define CHARGSERV_UNDERPOWER_VALUE		 90000000UL
#define CHARGSERV_MAXIMUM_ENERGY_VALUE	999999UL

#define CHARGSERV_MAXIMUM_AMPE_3KW				13
#define CHARGSERV_MAXIMUM_AMPE_5KW				22
#define CHARGSERV_MAXIMUM_AMPE_7KW				32
#define CHARGSERV_MAXIMUM_AMPE_11KW				50
#define CHARGSERV_AMPE_CONVERSION_STEP		2

#define CHARGSERV_LEAKAGE_DETECT_DELAY			100//5000	//unit : ms

/*
 * ((X) / 3.3 * 4096) , X : LEAKAGE AMPE TO VOLTAGE
 *
 * 2030 : 1.635V : The internal earth leakage breaker of the distribution box operates first.
 *
 * 1950 : 1.571V : SM3 intermittently detects earth leakage in normal state
 *
 * 1850 * 1: 1.5V : normal spec
 *
 * 1750 * 3
 */
#define CHARGSERV_LEAKAGE_AMPE_TO_ADC	1790//1900//1660//1550//1850//1985//1950//1700

#define CHARGSERV_AUTOFINISH_TIMEOUT	10000//600000		//unit : 1ms
#define CHARGSERV_FULL_WAKE_UP_SEQ_COUNT	2
//#define CHARGSERV_FORCESTOP_WAKE_UP_SEQ_COUNT	7


/*
 *  USERCONFIGMODE (Chargermode --> configmode)
 */
#define CHARGSERV_USERCONFIGMODE_INPUT_MAX_COUNT	5

//#define _VRMS_IRMS_CALC_LPF_FILTER_		1
//#if	((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
//#define _LPF_TIMEINTERVAL_ (2)
//#define _LPF_TAU_ (800)
//#endif

/*
 * _SEGMENTLED_Define
 */
#define CHARGSERV_PRINT_BOOTING							("BOOT  ")
#define CHARGSERV_PRINT_READY_CARDTAG					("CARD TAG.")
#define CHARGSERV_PRINT_READY_FWVERSION					("E2.01.01")
#define CHARGSERV_PRINT_READY_POWERCUT					("PWRCUT")
#define CHARGSERV_PRINT_POWERCUT_TIMEPASS				(" PASS ")
#define CHARGSERV_PRINT_USERCHECK_CARDTAGGING1			("CHECK ")
#define CHARGSERV_PRINT_USERCHECK_CARDTAGGING2			("CONNECT USER")
#define CHARGSERV_PRINT_USERCHECK_OK					("  OK  ")
#define CHARGSERV_PRINT_USERCHECK_FAIL					("_RETRY")
#define CHARGSERV_PRINT_CONNECT_WAIT1					('|')
#define CHARGSERV_PRINT_CONNECT_WAIT2					('/')
#define CHARGSERV_PRINT_CONNECT_WAIT3					('-')
#define CHARGSERV_PRINT_CONNECT_WAIT4					('!')	//('\')
#define CHARGSERV_PRINT_CONNECT_WIRED					(" WRIED")
#define CHARGSERV_PRINT_CONNECT_TIMEOUT1				(" TIME ")
#define CHARGSERV_PRINT_CONNECT_TIMEOUT2				(" OUT.  ")
#define CHARGSERV_PRINT_CHARGING_KWH					("123.45k")
#define CHARGSERV_PRINT_CHARGING_WON					("#30.000")
#define CHARGSERV_PRINT_FINISH_DEBT						("_DEBT_")
#define CHARGSERV_PRINT_FINISH_OK						("FINISH")
#define CHARGSERV_PRINT_FINISH_DISCONNECT				("UNPLUG")
#define CHARGSERV_PRINT_FAULT_ERROR						("ERROR_")
#define CHARGSERV_PRINT_FAULT_LEAKAGE					("E 107 ")
#define CHARGSERV_PRINT_FAULT_MC_START					("E 909 ")
#define CHARGSERV_PRINT_FAULT_MC_STOP					("E 909 ")
#define CHARGSERV_PRINT_FAULT_AC_OC						("E 104 ")
#define CHARGSERV_PRINT_FAULT_AC_OV						("E 102 ")
#define CHARGSERV_PRINT_FAULT_AC_UV						("E 103 ")
#define CHARGSERV_PRINT_FAULT_CP						("E 402 ")
#define CHARGSERV_PRINT_FAULT_OVER_TEMP					("E 908 ")
#define CHARGSERV_PRINT_FAULT_DM_COMM					("E 904 ")
#define CHARGSERV_PRINT_FAULT_RFID_COMM					("E 905 ")
#define CHARGSERV_PRINT_FAULT_CSMS_COMM					("E 912 ")
#define CHARGSERV_PRINT_OTA_REBOOT						("REBOOT")
#define CHARGSERV_PRINT_OTA_UPDATE						("SW-UP ")
#define CHARGSERV_PRINT_CLEAN							("      ")
#define CHARGSERV_PRINT_MODE_FLUG						(" PLUG ")
#define CHARGSERV_PRINT_MODE_EVSE						(" EVSE ")
#define CHARGSERV_PRINT_MODE_DEV						("-DEV--")
#define CHARGSERV_PRINT_ACTIVE_ENERGY_UNIT				("   KWH")

/*
 * CHARGSERV MSG Q tx reg
 */
#define CHARGSERV_MQ_TX_RFIDAPP_INIT_OK_ACK		0x01
#define CHARGSERV_MQ_TX_RFIDAPP_TAG_START		0x02
#define CHARGSERV_MQ_TX_DMAPP_INIT_OK_ACK		0x04
#define CHARGSERV_MQ_TX_DMAPP_VALUE_REQ			0x08
/*
 * CHARGSERV MSG Q rx reg
 */
#define CHARGSERV_MQ_RX_RFIDAPP_TAG_ERROR		0x01

//#define CHARGSERV_READ_DEV							(0 == GPI_DEV_IN)
//#define CHARGSERV_READ_MODE_7KW						((1 == GPI_SET_SW_04_IN) && (1 == GPI_SET_SW_03_IN) && (1 == GPI_SET_SW_02_IN))
//#define CHARGSERV_READ_MODE_11KW						((0 == GPI_SET_SW_04_IN) && (1 == GPI_SET_SW_03_IN) && (1 == GPI_SET_SW_02_IN))
#define CHARGSERV_READ_MODE_7KW							((1 == GPI_SET_SW_04_IN) && (1 == GPI_SET_SW_03_IN))
#define CHARGSERV_READ_MODE_11KW						((0 == GPI_SET_SW_04_IN) && (1 == GPI_SET_SW_03_IN))
#define CHARGSERV_READ_DEFAULT_START_MODE				((0 == GPI_SET_SW_02_IN))
#define CHARGSERV_READ_RESTART_FULLDETECT_ACTIVATION	((0 == GPI_SET_SW_01_IN))
#define CHARGSERV_AUTOSTART_MODE						((0 == GPI_DEV_IN))

typedef struct
{
	eCharger_State	state;
	uint8_t state_changed;
	eCharger_Mode	mode;
#if 0
	uint8_t dev_flag;

	uint8_t			init_reg;
	uint8_t			ready_reg;
	uint8_t			usercheck_reg;
	uint8_t			connect_reg;
	uint8_t			charging_reg;
	uint8_t			finish_reg;
	uint8_t			OTA_reg;
	uint8_t			fault_reg;
	uint8_t			powercut_reg;
#else
	e_CHARGSERV_REG reg;
	e_CHARGSERV_INIT_FLAG init_flag;
#endif
	uint16_t		charger_connect_wait_value;
	uint16_t		charger_connect_wait_count;
	uint16_t		charger_powercut_timepass_count;

	uint8_t			current_card_num[30];
	uint8_t			charging_card_num[30];

	uint32_t		current_Active_Energy;
	uint32_t		current_V_rms;
	uint32_t		current_I_rms;
	uint32_t		current_Active_Power;
	uint32_t		startpoint_Energy;

	uint8_t			active_Ampe;

	uint8_t 				wake_up_seq_flag;					// 0 : wake up seq not disable  ,  1 : wake up seq enable
	uint8_t					wake_up_seq_finish_flag;
	e__STND_WAKE_UP_SEQ wake_up_seq_step;

	uint32_t 		use_energy;
	uint32_t		use_won;

	uint8_t 		dm_ver_msb;
	uint8_t			dm_ver_lsb;

	eCharger_State	state_bk;

	eCharger_Charging_State chargstate;
	uint8_t			full_wake_up_seq_count;
	uint8_t			full_wake_up_seq_step;
	uint8_t			forcestop_wake_up_seq_count;
	uint8_t			forcestop_wake_up_seq_step;
	uint8_t			forcestop_wake_up_seq_flag;
	uint8_t			forcestop_wake_up_seq_count_limit_detect;

	uint8_t			userconfigmode_input_count;
	uint8_t			automode_active;

	uint8_t			leakage_instop_flag;
	uint8_t 		leakage_instop_step;

	uint8_t			oc_instop_flag;
	uint8_t			oc_instop_type;			//1 : waring , 2 : overflow
	uint8_t			oc_instop_step;
	//uint8_t			fullcharge_finish_flag;		// 0 : enable , 1 : disable
	uint8_t			stable_full_detect;
	uint8_t			stable_changed_flag;
	uint8_t 		autostartmode_flag;

	uint8_t			default_start_mode;		// 0 : mamual start , 1 : auto start
	uint8_t			restart_fulldetect_act;		// 0 : enable , 1 : disable

	uint8_t			emg_state;
	uint8_t			emg;
}sCharger;

//extern sCharger Charger;

eCharger_State _APP_CHARGSERV_get_current_state();
uint8_t _APP_CHARGSERV_get_current_reg_bit(eCharger_State cstate, uint8_t regvalue);
void _APP_CHARGSERV_state_machine();

uint8_t _APP_CHARGSERV_init_ok();

uint8_t _APP_CHARGSERV_ready_cardtag();
uint8_t _APP_CHARGSERV_ready_otareq();
uint8_t _APP_CHARGSERV_ready_powercutstart();

uint8_t _APP_CHARGSERV_usercheck_serverconnectstart();
uint8_t _APP_CHARGSERV_usercheck_user_ok();
uint8_t _APP_CHARGSERV_usercheck_user_retry();

uint8_t _APP_CHARGSERV_connect_wait();
uint8_t _APP_CHARGSERV_connect_ok();
uint8_t _APP_CHARGSERV_connect_timeout();
void _APP_CHARGSERV_connect_wait_count_forceout();

uint8_t _APP_CHARGSERV_charging_start();
uint8_t _APP_CHARGSERV_charging_ok();

uint8_t _APP_CHARGSERV_finish_start();
uint8_t _APP_CHARGSERV_finish_ok();

uint8_t _APP_CHARGSERV_fault_set();
uint8_t _APP_CHARGSERV_fault_reset();

uint8_t _APP_CHARGSERV_set_duty_ondelay(uint16_t ondelay);
uint8_t _APP_CHARGSERV_complete_duty_ondelay();
uint8_t _APP_CHARGSERV_reset_duty_ondelay();
uint8_t _APP_CHARGSERV_print_won(char * str, uint32_t won);
uint8_t _APP_CHARGSERV_print_kwh(char * str, uint32_t energy, uint8_t printunit);

void _APP_CHARGSERV_wake_up_seq_init();
uint8_t _APP_CHARGSERV_set_wake_up_seq_step(e__STND_WAKE_UP_SEQ value);

uint8_t _APP_CHARGSERV_check_charging_cardnum();
void _APP_CHARGSERV_check_Vrms_loop();
void _APP_CHARGSERV_check_Irms_loop();

void _APP_CHARGSERV_display_loop();
void _APP_CHARGSERV_ctrl_loop();
uint8_t _APP_CHARGSERV_print_state();
uint8_t _APP_CHARGSERV_is_leakage_fault_set();
void _APP_CHARGSERV_leakage_fault_set(uint16_t value);
void _APP_CHARGSERV_leakage_fault_reset();
void _APP_CHARGSERV_cp_fault_set();
void _APP_CHARGSERV_cp_fault_reset();
void _APP_CHARGSERV_dm_fault_set();
void _APP_CHARGSERV_dm_fault_reset();
void _APP_CHARGSERV_rfid_fault_set();
void _APP_CHARGSERV_rfid_fault_reset();
uint8_t _APP_CHARGSERV_is_over_temperature_fault_set();
void _APP_CHARGSERV_over_temperature_fault_set();
void _APP_CHARGSERV_over_temperature_fault_reset();
uint8_t _APP_CHARGSERV_set_active_Ampe(uint8_t mode);
uint8_t _APP_CHARGSERV_get_active_Ampe();
unsigned long _APP_CHARGSERV_get_voltage_rms_V();
unsigned long _APP_CHARGSERV_get_current_rms_A();
void _APP_CHARGSERV_set_voltage_rms_V(uint32_t Vrms);
void _APP_CHARGSERV_set_current_rms_A(uint32_t Irms);
uint8_t _APP_CHARGSERV_deinit();
void _APP_CHARGSERV_main();
void _APP_CHARGSERV_save_dm_value(uint8_t *dmbuf);

uint8_t _APP_CHARGSERV_is_serverconnectstart();
uint8_t _APP_CHARGSERV_is_state_charging_id_tagging();
uint8_t _APP_CHARGSERV_is_Ready_for_charging();
uint8_t _APP_CHARGSERV_is_state_finish();
uint8_t _APP_CHARGSERV_is_id_tagging_stop();
void _APP_CHARGSERV_set_connectiontimeout(int value);
void _APP_CHARGSERV_set_charging_to_unplug();
void _APP_CHARGSERV_clear_charging_to_unplug();
uint8_t _APP_CHARGSERV_is_charging_to_unplug();
float _APP_CHARGSERV_get_active_energy_float();
float _APP_CHARGSERV_get_active_energy_calc_float();
void _APP_CHARGSERV_get_card_number(char* temp);
uint8_t _APP_CHARGSERV_get_plug_state();

uint8_t _APP_CHARGSERV_is_dev_mode();
void _APP_CHARGSERV_get_charger_fault_status(eCharger_Fault *value);

void _APP_CHARGSERV_RemoteStart();
void _APP_CHARGSERV_RemoteStop();
uint8_t _APP_CHARGSERV_is_RemoteStop();

void _APP_CHARGSERV_set_chargingcontinuousmode(uint8_t mode);

void _APP_CHARGSERV_startup();
void _APP_CHARGSERV_process(void);
void charger_emg_fault();

#endif /* APP_INC__APP_CHARGING_SERVICE_H_ */
