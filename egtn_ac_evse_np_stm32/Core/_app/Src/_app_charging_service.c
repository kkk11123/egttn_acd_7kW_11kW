/*
 * _app_charging_service.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */



#include <_app_charging_service.h>
#include <_app_configration_service.h>
#include <_app_system_control.h>
#include <_app_rfid_comm.h>
#include <_config.h>


static gUserDelay gDelay_chargserv_periodic_loop_time;
static gUserDelay gDelay_setduty;
static gUserDelay gDelay_wusqtimeout;
static gUserDelay gDelay_wusqcpoff;
static gUserDelay gDelay_wusqpwmoff;

static gUserDelay gTimeout_usercheck;

static eCharger_Fault charger_fault_status;
static eCharger_Fault charger_fault_status_bak;
static gUserDelay gTimeout_wd_fault;
static gUserDelay gTimeout_ac_oc_fault;
static gUserDelay gTimeout_ac_oc_fast_fault;
static gUserDelay gTimeout_ac_ov_fault;
static gUserDelay gTimeout_ac_lv_fault;
static gUserDelay gTimeout_ac_uv_set_fault;
static gUserDelay gTimeout_ac_uv_clr_fault;
static gUserDelay gTimeout_csms_fault;
static gUserDelay gDelay_cp_fault;
static gUserDelay gTimeout_over_temp_set_fault;
static gUserDelay gTimeout_over_temp_clr_fault;
static gUserDelay gTimeout_auto_finish;
static gUserDelay gDelay_finish_mcoff;

static gUserDelay gTimeout_connect;

static gUserDelay gTimeout_charging_stable;
static gUserDelay gTimeout_charging_full;

static gUserDelay gTimeout_userconfigmode_input;
static gUserDelay gDelay_autostartmode_input;
static gUserDelay gDelay_charging_ok_button_input;

static gUserDelay gTimeout_full_charge_minimum_ampe;

static gUserDelay gDelay_forcestop_wake_up_seq_flag_set;

static gUserDelay gTimeout_forcestop_wake_up_seq_is_finish;
static gUserDelay gTimeout_full_wake_up_seq_is_finish;

static gUserDelay gTimout_Leakage_Relapse;

static gUserDelay gTimout_oc_waring_Relapse;
static gUserDelay gTimout_oc_over_Relapse;

static gUserDelay gDelay_autostartmode_default_set;

//#if ((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
//static s_LIB_LPF Vrms_calc;
//static s_LIB_LPF Irms_calc;
//#endif

static uint8_t charger_reset_reg_all_bit(eCharger_State cstate);
//static uint8_t charger_write_reg_bit(eCharger_State cstate, uint8_t regvalue, uint8_t onoff);
//static uint8_t charger_read_reg_bit(eCharger_State cstate, uint8_t regvalue);
static uint8_t charger_set_state(eCharger_State cstate);
static void charger_change_state(eCharger_State currentstate, eCharger_State changestate);

static sCharger Charger;
static uint8_t charger_duty_ondelay_flag = 0;
static uint8_t charger_duty_ondelay_complete = 0;
int AC_LV_ERR = 0; //저전압 검출을 위한 변수 선언
int TEMP_ERR = 0;

//충전 상태에 따른 플래그 비트 초기화
uint8_t charger_reset_reg_all_bit(eCharger_State cstate) 
{
	switch(cstate)
	{
		case Init :
			Charger.reg.init_complete = 0;
			Charger.reg.init_proceeding = 0;
		break;
		case Ready :
			Charger.reg.ready_cardtag = 0;
			Charger.reg.ready_ota_request = 0;
			Charger.reg.ready_standby = 0;
			Charger.reg.ready_automodeflag = 0;
		break;
		case AutoReady :
			Charger.reg.autoready_connected = 0;
			Charger.reg.autoready_manualmodeflag = 0;
			Charger.reg.autoready_standby = 0;
		break;
		case UserCheck :
			Charger.reg.usercheck_tagok = 0;
			Charger.reg.usercheck_userok = 0;
			Charger.reg.usercheck_userretry = 0;
		break;
		case Connect :
			Charger.reg.connect_timeout = 0;
			Charger.reg.connect_wait = 0;
			Charger.reg.connect_wired = 0;
		break;
		case Charging :
			Charger.reg.charging_ok = 0;
			Charger.reg.charging_proceeding = 0;
			//Charger.reg.charging_id_tagging_stop = 0;
		break;
		case Finish :
			Charger.reg.finish_ok = 0;
			Charger.reg.finish_stoptranscation_flag = 0;
			Charger.reg.finish_proceeding = 0;
		break;
		case OTA :
			Charger.reg.ota_ok = 0;
			Charger.reg.ota_proceeding = 0;
		break;
		case Fault :
			Charger.reg.fault_reset = 0;
			Charger.reg.fault_set = 0;
		break;
		case Powercut :
			Charger.reg.powercut_proceeding = 0;
			Charger.reg.powercut_timepass = 0;
		break;
		default :
			return _FALSE;
		break;
	}

	return _TRUE;
}

uint8_t charger_set_state(eCharger_State cstate)
{
	Charger.state = cstate;
	return _TRUE;
}

uint8_t charger_set_mode(eCharger_Mode cmode)
{
	Charger.mode = cmode;
	return _TRUE;
}

eCharger_State _APP_CHARGSERV_get_current_state()
{
	return Charger.state;
}


eCharger_Mode charger_get_mode()
{
	return Charger.mode;
}

//상태를 변화시키면 현재 상태의 플래그는 초기화, 바꿀 상태의 플래그 설정
void charger_change_state(eCharger_State currentstate, eCharger_State changestate)
{
	charger_reset_reg_all_bit(currentstate);
	charger_set_state(changestate);
	//charger_set_display_cycle_finish(changestate);
	Charger.reg.display_enable = 0;
	Charger.reg.display_cyclefinish = 1;
	//charger_write_reg_bit(changestate, changeregbit, _ON);

	Charger.reg.state_changed = 1;
}

#if 0
uint8_t charger_write_reg_bit(eCharger_State cstate, uint8_t regvalue, uint8_t onoff)
{
	switch(cstate)
	{
		case Init :
			if(onoff == _ON)	Charger.init_reg |= regvalue;
			else					Charger.init_reg &= ~regvalue;
		break;
		case Ready :
			if(onoff == _ON)	Charger.ready_reg |= regvalue;
			else					Charger.ready_reg &= ~regvalue;
		break;
		case UserCheck :
			if(onoff == _ON)	Charger.usercheck_reg |= regvalue;
			else					Charger.usercheck_reg &= ~regvalue;
		break;
		case Connect :
			if(onoff == _ON)	Charger.connect_reg |= regvalue;
			else					Charger.connect_reg &= ~regvalue;
		break;
		case Charging :
			if(onoff == _ON)	Charger.charging_reg |= regvalue;
			else					Charger.charging_reg &= ~regvalue;
		break;
		case Finish :
			if(onoff == _ON)	Charger.finish_reg |= regvalue;
			else					Charger.finish_reg &= ~regvalue;
		break;
		case OTA :
			if(onoff == _ON)	Charger.OTA_reg |= regvalue;
			else					Charger.OTA_reg &= ~regvalue;
		break;
		case Fault :
			if(onoff == _ON)	Charger.fault_reg |= regvalue;
			else					Charger.fault_reg &= ~regvalue;
		break;
		case Powercut :
			if(onoff == _ON)	Charger.powercut_reg |= regvalue;
			else					Charger.powercut_reg &= ~regvalue;
		break;
		default :
			return _FALSE;
		break;
	}

	return _TRUE;
}


uint8_t charger_reset_display_cycle_finish(eCharger_State cstate)
{
	switch(cstate)
	{
		case Init :
			Charger.init_reg |= CHARGING_REG_DISP_ENABLE;
			Charger.init_reg &= ~CHARGING_REG_DISP_CYCLEFINISH;
		break;
		case Ready :
			Charger.ready_reg |= CHARGING_REG_DISP_ENABLE;
			Charger.ready_reg &= ~CHARGING_REG_DISP_CYCLEFINISH;
		break;
		case UserCheck :
			Charger.usercheck_reg |= CHARGING_REG_DISP_ENABLE;
			Charger.usercheck_reg &= ~CHARGING_REG_DISP_CYCLEFINISH;
		break;
		case Connect :
			Charger.connect_reg |= CHARGING_REG_DISP_ENABLE;
			Charger.connect_reg &= ~CHARGING_REG_DISP_CYCLEFINISH;
		break;
		case Charging :
			Charger.charging_reg |= CHARGING_REG_DISP_ENABLE;
			Charger.charging_reg &= ~CHARGING_REG_DISP_CYCLEFINISH;
		break;
		case Finish :
			Charger.finish_reg |= CHARGING_REG_DISP_ENABLE;
			Charger.finish_reg &= ~CHARGING_REG_DISP_CYCLEFINISH;
		break;
		case OTA :
			Charger.OTA_reg |= CHARGING_REG_DISP_ENABLE;
			Charger.OTA_reg &= ~CHARGING_REG_DISP_CYCLEFINISH;
		break;
		case Fault :
			Charger.fault_reg |= CHARGING_REG_DISP_ENABLE;
			Charger.fault_reg &= ~CHARGING_REG_DISP_CYCLEFINISH;
		break;
		case Powercut :
			Charger.powercut_reg |= CHARGING_REG_DISP_ENABLE;
			Charger.powercut_reg &= ~CHARGING_REG_DISP_CYCLEFINISH;
		break;
		default :
			return _FALSE;
		break;
	}

	return _TRUE;
}

uint8_t charger_set_display_cycle_finish(eCharger_State cstate)
{
	switch(cstate)
	{
		case Init :
			Charger.init_reg |= CHARGING_REG_DISP_CYCLEFINISH;
			Charger.init_reg &= ~CHARGING_REG_DISP_ENABLE;
		break;
		case Ready :
			Charger.ready_reg |= CHARGING_REG_DISP_CYCLEFINISH;
			Charger.ready_reg &= ~CHARGING_REG_DISP_ENABLE;
		break;
		case UserCheck :
			Charger.usercheck_reg |= CHARGING_REG_DISP_CYCLEFINISH;
			Charger.usercheck_reg &= ~CHARGING_REG_DISP_ENABLE;
		break;
		case Connect :
			Charger.connect_reg |= CHARGING_REG_DISP_CYCLEFINISH;
			Charger.connect_reg &= ~CHARGING_REG_DISP_ENABLE;
		break;
		case Charging :
			Charger.charging_reg |= CHARGING_REG_DISP_CYCLEFINISH;
			Charger.charging_reg &= ~CHARGING_REG_DISP_ENABLE;
		break;
		case Finish :
			Charger.finish_reg |= CHARGING_REG_DISP_CYCLEFINISH;
			Charger.finish_reg &= ~CHARGING_REG_DISP_ENABLE;
		break;
		case OTA :
			Charger.OTA_reg |= CHARGING_REG_DISP_CYCLEFINISH;
			Charger.OTA_reg &= ~CHARGING_REG_DISP_ENABLE;
		break;
		case Fault :
			Charger.fault_reg |= CHARGING_REG_DISP_CYCLEFINISH;
			Charger.fault_reg &= ~CHARGING_REG_DISP_ENABLE;
		break;
		case Powercut :
			Charger.powercut_reg |= CHARGING_REG_DISP_CYCLEFINISH;
			Charger.powercut_reg &= ~CHARGING_REG_DISP_ENABLE;
		break;
		default :
			return _FALSE;
		break;
	}

	return _TRUE;
}

uint8_t charger_read_reg_bit(eCharger_State cstate, uint8_t regvalue)
{
	uint8_t *preg;

	switch(cstate)
	{
		case Init :
			preg = &(Charger.init_reg);
		break;
		case Ready :
			preg = &(Charger.ready_reg);
		break;
		case UserCheck :
			preg = &(Charger.usercheck_reg);
		break;
		case Connect :
			preg = &(Charger.connect_reg);
		break;
		case Charging :
			preg = &(Charger.charging_reg);
		break;
		case Finish :
			preg = &(Charger.finish_reg);
		break;
		case OTA :
			preg = &(Charger.OTA_reg);
		break;
		case Fault :
			preg = &(Charger.fault_reg);
		break;
		case Powercut :
			preg = &(Charger.powercut_reg);
		break;
		default :
			return _FALSE;
		break;
	}

	if(((*preg) & regvalue) == regvalue)
	{
		return _TRUE;
	}
	else
	{
		return _FALSE;
	}
}
#endif

#if 0
void charger_set_dev()
{
	Charger.dev_flag = _ON;
}
void charger_clr_dev()
{
	Charger.dev_flag = _OFF;
}
uint8_t charger_get_dev_flag()
{
	return Charger.dev_flag;
}
#endif

void charger_set_connect_wait_count(int value)
{
	Charger.charger_connect_wait_value = (value);
}

void _APP_CHARGSERV_connect_wait_count_forceout()
{
	Charger.charger_connect_wait_count = 0;
}

#if 0
void charger_set_active_energy_display(char * str)
{
	//uint32_t temp = 123456;
	uint32_t temp = Charger.current_Active_Energy;

	uint32_t temp_integer = temp / 100;
	uint32_t temp_decimal = temp % 100;

	if(_OFF == loadled)
	{
		sprintf(str,"%04ld.%02ld",temp_integer,temp_decimal);
	}
	else
	{
		sprintf(str,"%04ld.%02ld.",temp_integer,temp_decimal);
	}
}
#endif

void charger_reset_powercut_pass_count()
{
	Charger.charger_powercut_timepass_count = 10;
}
void charger_lapse_powercut_pass_count()
{
	Charger.charger_powercut_timepass_count--;
}
uint16_t charger_get_powercut_pass_count()
{
	return Charger.charger_powercut_timepass_count;
}
void charger_set_powercut_pass_count(char * str)
{
	sprintf(str," %02d   ",charger_get_powercut_pass_count());
	if(str[1] == '0')
	{
		str[1] = ' ';
	}
}


#if 0
uint8_t _APP_CHARGSERV_get_current_reg_bit(eCharger_State cstate, uint8_t regvalue)
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if(stemp != cstate)
	{
		return _FALSE;
	}

	return charger_read_reg_bit(cstate, regvalue);
}
#endif

uint8_t _APP_CHARGSERV_init_ok()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == Init) && (1 == Charger.reg.init_proceeding))
	{
		//charger_write_reg_bit(Init, CHARGSERV_REG_INIT_PROCEEDING, _OFF);
		Charger.reg.init_proceeding = 0;
		//charger_write_reg_bit(Init, CHARGSERV_REG_INIT_COMPLETE, _ON);
		Charger.reg.init_complete = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : init ok #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

uint8_t _APP_CHARGSERV_ready_cardtag()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == Ready) && (1 == Charger.reg.ready_standby) &&
			               (0 == Charger.reg.ready_ota_request) &&
						   (0 == Charger.reg.ready_nontag_overpower) )
	{
		//charger_write_reg_bit(Ready, CHARGSERV_REG_READY_STANDBY, _OFF);
		Charger.reg.ready_standby = 0;
		//charger_write_reg_bit(Ready, CHARGSERV_REG_READY_CARDTAG, _ON);
		Charger.reg.ready_cardtag = 1;

#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : Card Tag #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

uint8_t _APP_CHARGSERV_ready_otareq()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == Ready) && (1 == Charger.reg.ready_standby) &&
			               (0 == Charger.reg.ready_cardtag) &&
						   (0 == Charger.reg.ready_nontag_overpower) )
	{
		//charger_write_reg_bit(Ready, CHARGSERV_REG_READY_STANDBY, _OFF);
		Charger.reg.ready_standby = 0;
		//charger_write_reg_bit(Ready, CHARGSERV_REG_READY_OTAREQ, _ON);
		Charger.reg.ready_ota_request = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : OTA Req #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

uint8_t _APP_CHARGSERV_ready_powercutstart()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == Ready) && (1 == Charger.reg.ready_standby) &&
			               (0 == Charger.reg.ready_cardtag) &&
						   (0 == Charger.reg.ready_ota_request) )
	{
		//charger_write_reg_bit(Ready, CHARGSERV_REG_READY_STANDBY, _OFF);
		Charger.reg.ready_standby = 0;
		//charger_write_reg_bit(Ready, CHARGSERV_REG_READY_NONTAG_OVERPOWER, _ON);
		Charger.reg.ready_nontag_overpower = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : Powercut start #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

//자동 시작 모드 플래그 설정
uint8_t _APP_CHARGSERV_ready_automodeflag()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == Ready) && (1 == Charger.reg.ready_standby))
	{
		//charger_write_reg_bit(Ready, CHARGSERV_REG_READY_STANDBY, _OFF);
		Charger.reg.ready_standby = 0;
		//charger_write_reg_bit(Ready, CHARGSERV_REG_READY_NONTAG_OVERPOWER, _ON);
		Charger.reg.ready_automodeflag = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : ready_Automodeflag #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

// 자동 시작 모드 -> 수동 시작 모드 플래그 설정
uint8_t _APP_CHARGSERV_autoready_manualmodeflag()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == AutoReady) && (1 == Charger.reg.autoready_standby) &&
			               (0 == Charger.reg.autoready_connected))
	{
		//charger_write_reg_bit(Ready, CHARGSERV_REG_READY_STANDBY, _OFF);
		Charger.reg.autoready_standby = 0;
		//charger_write_reg_bit(Ready, CHARGSERV_REG_READY_NONTAG_OVERPOWER, _ON);
		Charger.reg.autoready_manualmodeflag = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : AutoReady_maunalmodeflag #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

//자동시작모드에서 충전기 연결시 플래그 설정
uint8_t _APP_CHARGSERV_autoready_connected()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == AutoReady) && (1 == Charger.reg.autoready_standby) &&
			               (0 == Charger.reg.autoready_manualmodeflag))
	{
		//charger_write_reg_bit(Ready, CHARGSERV_REG_READY_STANDBY, _OFF);
		Charger.reg.autoready_standby = 0;
		//charger_write_reg_bit(Ready, CHARGSERV_REG_READY_NONTAG_OVERPOWER, _ON);
		Charger.reg.autoready_connected = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : AutoReady_connected #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

//Usercheck 완료 시 서버 연결 플래그 설정
uint8_t _APP_CHARGSERV_usercheck_serverconnectstart()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == UserCheck) && (1 == Charger.reg.usercheck_tagok))
	{
		//charger_write_reg_bit(UserCheck, CHARGSERV_REG_USERCHECK_TAGOK, _OFF);
		Charger.reg.usercheck_tagok = 0;
		//charger_write_reg_bit(UserCheck, CHARGSERV_REG_USERCHECK_SERVERCONNECTING, _ON);
		Charger.reg.usercheck_serverconnecting = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : Server Connect Start #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

//유저 확인 완료 플래그 설정
uint8_t _APP_CHARGSERV_usercheck_user_ok()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == UserCheck) && (1 == Charger.reg.usercheck_serverconnecting))
	{
		//charger_write_reg_bit(UserCheck, CHARGSERV_REG_USERCHECK_SERVERCONNECTING, _OFF);
		Charger.reg.usercheck_serverconnecting = 0;
		Charger.reg.usercheck_authorize_flag = 0;
		//charger_write_reg_bit(UserCheck, CHARGSERV_REG_USERCHECK_USEROK, _ON);
		Charger.reg.usercheck_userok = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : UserCheck ok #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

uint8_t _APP_CHARGSERV_usercheck_user_retry()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == UserCheck) && (1 == Charger.reg.usercheck_serverconnecting))
	{
		//charger_write_reg_bit(UserCheck, CHARGSERV_REG_USERCHECK_SERVERCONNECTING, _OFF);
		Charger.reg.usercheck_serverconnecting = 0;
		Charger.reg.usercheck_authorize_flag = 0;
		//charger_write_reg_bit(UserCheck, CHARGSERV_REG_USERCHECK_USERRETRY, _ON);
		Charger.reg.usercheck_userretry = 1;

		_LIB_USERDELAY_stop(&gTimeout_usercheck);

#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : UserCheck Fail. Retry #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

uint8_t _APP_CHARGSERV_connect_ok()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == Connect) && (1 == Charger.reg.connect_wait))
	{
		//charger_write_reg_bit(Connect, CHARGSERV_REG_CONNECT_WAIT, _OFF);
		Charger.reg.connect_wait = 0;
		//charger_write_reg_bit(Connect, CHARGSERV_REG_CONNECT_WIRED, _ON);
		Charger.reg.connect_wired = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : EV Connect ok #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

//충전건 연결 시간 초과 플래그 설정 
uint8_t _APP_CHARGSERV_connect_timeout()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == Connect) && (1 == Charger.reg.connect_wait))
	{
		//charger_write_reg_bit(Connect, CHARGSERV_REG_CONNECT_WAIT, _OFF);
		Charger.reg.connect_wait = 0;
		//charger_write_reg_bit(Connect, CHARGSERV_REG_CONNECT_TIMEOUT, _ON);
		Charger.reg.connect_timeout = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : EV Connect Timeout #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

//충전 완료 플래그 설정
uint8_t _APP_CHARGSERV_charging_ok()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == Charging) && (1 == Charger.reg.charging_proceeding))
	{
		//charger_write_reg_bit(Charging, CHARGSERV_REG_CHARGING_PROCEEDING, _OFF);
		Charger.reg.charging_proceeding = 0;
		//charger_write_reg_bit(Charging, CHARGSERV_REG_CHARGING_OK, _ON);
		Charger.reg.charging_ok = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : EV Charging ok #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

// 충전 끝 플래그
uint8_t _APP_CHARGSERV_finish_ok()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == Finish) && (1 == Charger.reg.finish_proceeding))
	{
		//charger_write_reg_bit(Finish, CHARGSERV_REG_FINISH_PROCEEDING, _OFF);
		Charger.reg.finish_proceeding = 0;
		//charger_write_reg_bit(Finish, CHARGSERV_REG_FINISH_OK, _ON);
		Charger.reg.finish_ok = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : Finish ok #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

uint8_t _APP_CHARGSERV_powercut_timepass_ok()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == Powercut) && (1 == Charger.reg.powercut_proceeding))
	{
		//charger_write_reg_bit(Powercut, CHARGSERV_REG_POWERCUT_PROCEEDING, _OFF);
		Charger.reg.powercut_proceeding = 0;
		//charger_write_reg_bit(Powercut, CHARGSERV_REG_POWERCUT_TIMEPASS, _ON);
		Charger.reg.powercut_timepass = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : Powercut Timepass #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

//상태를 fault로 변경 후 오류 플래그 설정
uint8_t _APP_CHARGSERV_fault_set()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if(stemp != Fault)
	{
		charger_change_state(stemp, Fault);
		//CHARGSERV_REG_FAULT_SET
		Charger.reg.fault_reset = 0;
		Charger.reg.fault_set = 1;
	#if ((__CHARGSERV_DEBUG__)==1)
			_LIB_LOGGING_printf("#### CHARGSERV : Fault Set #### \r\n");
	#endif

		return _TRUE;
	}

	return _FALSE;
}

//오류 리셋 플래그 설정
uint8_t _APP_CHARGSERV_fault_reset()
{
	eCharger_State stemp = _APP_CHARGSERV_get_current_state();

	if((stemp == Fault) && (1 == Charger.reg.fault_set))
	{
		//charger_write_reg_bit(Fault, CHARGSERV_REG_FAULT_SET, _OFF);
		//charger_write_reg_bit(Fault, CHARGSERV_REG_FAULT_RESET, _ON);
		Charger.reg.fault_reset = 1;
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : Fault Reset #### \r\n");
#endif
		return _TRUE;
	}

	return _FALSE;
}

uint8_t _APP_CHARGSERV_set_duty_ondelay(uint16_t ondelay)
{
#if 0
	osStatus_t  status;
#endif

	if(0 == charger_duty_ondelay_flag)
	{
#if 0
		status = osTimerStart(T2dutydelayHandle, ondelay);

		if(osOK == status)
		{
			charger_duty_ondelay_flag = 1;
			charger_duty_ondelay_complete = 0;
		}
		return _TRUE;

#else
		_LIB_USERDELAY_set(&gDelay_setduty, ondelay);
		_LIB_USERDELAY_start(&gDelay_setduty, DELAY_RENEW_OFF);

		charger_duty_ondelay_flag = 1;
		charger_duty_ondelay_complete = 0;

		return _TRUE;
#endif
	}
	return _FALSE;
}

uint8_t _APP_CHARGSERV_complete_duty_ondelay()
{

#if 0
	if(1 == charger_duty_ondelay_complete)
	{
		charger_duty_ondelay_flag = 0;
		charger_duty_ondelay_complete = 0;
		return _TRUE;
	}
#else
	if(1 == _LIB_USERDELAY_isfired(&gDelay_setduty))
	{
		_LIB_USERDELAY_stop(&gDelay_setduty);
		charger_duty_ondelay_flag = 0;
		charger_duty_ondelay_complete = 0;
		return _TRUE;
	}
#endif
	else
	{
		return _FALSE;
	}
}

uint8_t _APP_CHARGSERV_reset_duty_ondelay()
{
	charger_duty_ondelay_complete = 1;
	return _TRUE;
}

uint8_t _APP_CHARGSERV_print_won(char * str, uint32_t won)
{
	uint32_t won1 = won / 1000;
	uint32_t won2 = won % 1000;

	sprintf(str,"#%02d%03d",(uint16_t)won1, (uint16_t)won2);

	return _TRUE;
}

#if ((_PRINT_UNIT_KWH_)==0)
uint8_t _APP_CHARGSERV_print_kwh(char * str, uint32_t energy, uint8_t printunit)
{
	uint32_t energyint = energy / 100;
	uint32_t energydecimal = energy % 100;

	if(_OFF == loadled)
	{
		sprintf(str,"%04d.%02d",(uint16_t)energyint, (uint16_t)energydecimal);
	}
	else
	{
		sprintf(str,"%04d.%02d.",(uint16_t)energyint, (uint16_t)energydecimal);
	}

	return _TRUE;
}
#else
uint8_t _APP_CHARGSERV_print_kwh(char * str, uint32_t energy, uint8_t printunit)
{
	uint32_t energyint = energy / 100;
	uint32_t energydecimal = energy % 100;

	if(_OFF == printunit)
	{
		sprintf(str,"%04d.%02d",(uint16_t)energyint, (uint16_t)energydecimal);
	}
	else
	{
#if 0
		sprintf(str,"%04d.%02dKWH",(uint16_t)energyint, (uint16_t)energydecimal);
#else
		sprintf(str,"%04d.%02d",(uint16_t)energyint, (uint16_t)energydecimal);
#endif
	}

	return _TRUE;
}
#endif


//모드 설정 : 기본 7kW
uint8_t charger_set_modecfg()
{
	eCharger_Mode mode_temp = mode_11KW;

	/*
	 * DEV MODE
	 * 		1). Fault skip  (except MCCB BREAK CURRENT)
	 * 		    --> can use imaginary load
	 * 		2). if EVSE , MC RELAY TURN ON
	 * 		3). if EVSE,FLUG , disable usercheck csms comm
	 * 		4). if FLUG , disable overpowercut
	 *
	 */

	if(CHARGSERV_READ_MODE_7KW)
	{
		_LIB_LOGGING_printf("mode : EVSE_7KW\r\n");
		mode_temp = mode_7KW;
	}
	else if(CHARGSERV_READ_MODE_11KW)
	{
		_LIB_LOGGING_printf("mode : EVSE_11KW\r\n");
		mode_temp = mode_11KW;
	}
	else
	{
		_LIB_LOGGING_printf("mode : EVSE_7KW\r\n");
		mode_temp = mode_7KW;
	}

#if 0
	if(CHARGSERV_READ_FULLCHARGE_FINISH_DISABLE)
	{
		_LIB_LOGGING_printf("Option : FullCharge Finish Disable \r\n");
		Charger.fullcharge_finish_flag = 1;
	}

	if(CHARGSERV_READ_MODE_DISABLE)
	{
		_LIB_LOGGING_printf("mode : Configration mode ___ Chargserv Disable \r\n");
		mode_temp = mode_config;
	}
#endif
	//기본 시작 -> 자동 시작 모드
	if(CHARGSERV_READ_DEFAULT_START_MODE)
	{
		_LIB_LOGGING_printf("Option __ DEFAULT_START : Auto START \r\n");
		Charger.default_start_mode = 1;
	}

	if(CHARGSERV_READ_RESTART_FULLDETECT_ACTIVATION) //DIPSW 1이 OFF시
	{
		_LIB_LOGGING_printf("Option __ RESTART_FULLDETECT_ACTIVATION : Disable \r\n");
		Charger.restart_fulldetect_act = 1;
	}

	charger_set_mode(mode_temp);

	if(CHARGSERV_AUTOSTART_MODE) //I_TEST가 ON시
	{
		_LIB_LOGGING_printf("mode : Auto Start Mode!! \r\n");
		Charger.autostartmode_flag = 1;
	}

	//수동 시작 모드 시
	if(0 == Charger.autostartmode_flag)
	{
		//charger_clr_dev();
		Charger.reg.dev_flag = 0;
		GPO_MC_TURN_OFF_ENABLE;//_MW_GPIO_set_gpo(MC_TURN_OFF, _ON);
		GPO_MC_TURN_ON_DISABLE;//_MW_GPIO_set_gpo(MC_TURN_ON, _OFF);
		HAL_Delay(50);
		GPO_MC_TURN_OFF_DISABLE;//_MW_GPIO_set_gpo(MC_TURN_OFF, _OFF);
		GPO_MC_TURN_ON_DISABLE;//_MW_GPIO_set_gpo(MC_TURN_ON, _OFF);
	}
	else //자동 시작 모드 시
	{
		Charger.reg.dev_flag = 1;
		GPO_MC_TURN_OFF_DISABLE;//_MW_GPIO_set_gpo(MC_TURN_OFF, _ON);
		GPO_MC_TURN_ON_ENABLE;//_MW_GPIO_set_gpo(MC_TURN_ON, _OFF);
		HAL_Delay(50);
		GPO_MC_TURN_OFF_DISABLE;//_MW_GPIO_set_gpo(MC_TURN_OFF, _OFF);
		GPO_MC_TURN_ON_DISABLE;//_MW_GPIO_set_gpo(MC_TURN_ON, _OFF);
	}


	return _TRUE;
}

uint8_t charger_get_wake_up_seq_flag()
{
	//if(multi_ch_model[slave_id].wake_up_seq_flag == 1)
	if(Charger.wake_up_seq_flag == 1)
	{
		return 1;
	}

	return 0;
}

e__STND_WAKE_UP_SEQ charger_get_wake_up_seq_step()
{
	//return multi_ch_model[slave_id].wake_up_seq_step;
	return Charger.wake_up_seq_step;
}

void charger_wake_up_seq_enable()
{
	//multi_ch_model[slave_id].wake_up_seq_flag = 1;
	if(Charger.wake_up_seq_flag == 0)
	{
		_LIB_LOGGING_printf("wuseq enable\r\n");
	}
	Charger.wake_up_seq_flag = 1;
	//_LOGGING_Printf("%d ch, wake up seq enable\r\n", slave_id);
}

void charger_wake_up_seq_disable()
{
	//multi_ch_model[slave_id].wake_up_seq_flag = 0;
	//multi_ch_model[slave_id].wake_up_seq_step = NON;

	Charger.wake_up_seq_flag = 0;
	Charger.wake_up_seq_step = NON;

	_LIB_LOGGING_printf("wuseq disable\r\n");
	//_LOGGING_Printf("%d ch, wake up seq disable\r\n", slave_id);
}

uint8_t charger_wake_up_seq_is_finish()
{
	uint8_t ret_value = _FALSE;

	if(1 == Charger.wake_up_seq_finish_flag)
	{
		Charger.wake_up_seq_finish_flag = 0;
		ret_value = _TRUE;
	}

	return ret_value;
}

void _APP_CHARGSERV_wake_up_seq_init()
{
	charger_wake_up_seq_disable();

	_LIB_USERDELAY_set(&gDelay_wusqtimeout, CHARGSERV_WUSQ_NORMAL_TIMEOUT);
	_LIB_USERDELAY_set(&gDelay_wusqcpoff, CHARGSERV_WUSQ_CP_OFF);
	_LIB_USERDELAY_set(&gDelay_wusqpwmoff, CHARGSERV_WUSQ_PWM_OFF);
}
uint8_t charger_wake_up_seq_start(uint8_t mode)
{
	if((charger_get_wake_up_seq_flag() == 1) && (charger_get_wake_up_seq_step() == NON))
	{
		//multi_ch_model[slave_id].wake_up_seq_step = START_TIMEOUT;
		Charger.wake_up_seq_step = START_TIMEOUT;

		if(0 == mode)
		{
			_LIB_LOGGING_printf("wuseq start : 30\r\n");
			_LIB_USERDELAY_set(&gDelay_wusqtimeout, CHARGSERV_WUSQ_NORMAL_TIMEOUT);
		}
		else if(1 == mode)
		{
			_LIB_LOGGING_printf("wuseq start : 120\r\n");
			_LIB_USERDELAY_set(&gDelay_wusqtimeout, CHARGSERV_WUSQ_CAHRGING_TIMEOUT);
		}
		else if(2 == mode)
		{
			_LIB_LOGGING_printf("wuseq start : 600\r\n");
			_LIB_USERDELAY_set(&gDelay_wusqtimeout, CHARGSERV_WUSQ_CAHRGING3_TIMEOUT);
		}
		else if(3 == mode)
		{
			_LIB_LOGGING_printf("wuseq start : 1800\r\n");
			_LIB_USERDELAY_set(&gDelay_wusqtimeout, CHARGSERV_WUSQ_CAHRGING4_TIMEOUT);
		}
		else if(99 == mode)
		{
			_LIB_LOGGING_printf("wuseq start : 5\r\n");
			_LIB_USERDELAY_set(&gDelay_wusqtimeout, CHARGSERV_WUSQ_LEAKAGE_TIMEOUT);
		}
		else
		{
			_LIB_LOGGING_printf("wuseq start : 1800\r\n");
			_LIB_USERDELAY_set(&gDelay_wusqtimeout, CHARGSERV_WUSQ_CAHRGING4_TIMEOUT);
		}


		return _TRUE;
	}

	return _FALSE;
}

uint8_t _APP_CHARGSERV_set_wake_up_seq_step(e__STND_WAKE_UP_SEQ value)
{
	if(charger_get_wake_up_seq_flag() == 1)
	{
		Charger.wake_up_seq_step = value;
		return _TRUE;
	}

	return _FALSE;
}

#if 0
uint8_t charger_wake_up_seq_starttimer(osTimerId_t timer_id, uint32_t ticks)
{
	osStatus_t  status;

	status = osTimerIsRunning(timer_id);

	if(1 == status)
	{
		return _FALSE;
	}

	status = osTimerStart(timer_id, ticks);

	if(osOK == status)
	{
		return _TRUE;
	}
	else
	{
		return _FALSE;
	}
}

uint8_t charger_wake_up_seq_stoptimer(osTimerId_t timer_id)
{
	osStatus_t  status;

	status = osTimerIsRunning(timer_id);

	if(0 == status)
	{
		return _FALSE;
	}

	status = osTimerStop(timer_id);

	if(osOK == status)
	{
		return _TRUE;
	}
	else
	{
		return _FALSE;
	}
}
#endif

void charger_wake_up_seq_forced_stop()
{
	if(charger_get_wake_up_seq_flag() == 1)
	{
		//charger_wake_up_seq_stoptimer(T3wusqtimeoutHandle);
		//charger_wake_up_seq_stoptimer(T4wusqcpoffHandle);
		//charger_wake_up_seq_stoptimer(T5wusqpwmoffHandle);

		_LIB_USERDELAY_stop(&gDelay_wusqtimeout);
		_LIB_USERDELAY_stop(&gDelay_wusqcpoff);
		_LIB_USERDELAY_stop(&gDelay_wusqpwmoff);

		charger_wake_up_seq_disable();
	}
}

uint8_t charger_state_is_changed()
{
	if(_ON == Charger.state_changed)
	{
		Charger.state_changed = _OFF;
		return _ON;
	}
	else
	{
		return _OFF;
	}
}

#if 0
void charger_msg_set(e__MSGQUEUE_DIR dir, uint16_t value)
{
	switch(dir)
	{
		case msg_tx :
			Charger.msg_tx_reg |= value;
		break;
		case msg_rx :
			Charger.msg_rx_reg |= value;
		break;
		default :

		break;
	}
}

uint8_t charger_msg_is_set(e__MSGQUEUE_DIR dir, uint16_t value)
{
	uint8_t temp;

	switch(dir)
	{
		case msg_tx :
			if((Charger.msg_tx_reg & value) == value)
			{
				temp = _ON;
			}
			else
			{
				temp = _OFF;
			}
		break;
		case msg_rx :
			if((Charger.msg_rx_reg & value) == value)
			{
				temp = _ON;
			}
			else
			{
				temp = _OFF;
			}
		break;
		default :

		break;
	}

	return temp;
}

void charger_msg_clear(e__MSGQUEUE_DIR dir, uint16_t value)
{
	switch(dir)
	{
		case msg_tx :
			Charger.msg_tx_reg &= ~value;
		break;
		case msg_rx :
			Charger.msg_rx_reg &= ~value;
		break;
		default :

		break;
	}
}
#endif

uint8_t charger_is_load_detect_flug() //Irms >= 20ms
{
	if(Charger.current_I_rms >= CHARGSERV_FLUG_LOAD_DETECT_VALUE)
	{
		return _TRUE;
	}
	else
	{
		return _FALSE;
	}
}

uint8_t charger_is_load_detect_evse()
{
	if(Charger.current_I_rms >= CHARGSERV_EVSE_LOAD_DETECT_VALUE)
	{
		return _TRUE;
	}
	else
	{
		return _FALSE;
	}
}

#if 0
uint8_t charger_is_overpower()
{
#if ((_PLUG_TEST_MODE_)==0)
	if(Charger.current_Active_Power > CHARGSERV_OVERPOWER_VALUE)
	{
		return _TRUE;
	}
	else
	{
		return _FALSE;
	}
#else
	uint8_t value;

	value = charger_is_load_detect_flug();

	if(_TRUE == value)
	{
		return _TRUE;
	}
	else
	{
		return _FALSE;
	}

#endif
}
#endif

#if 0
uint8_t charger_is_underpower()
{
#if ((_PLUG_TEST_MODE_)==0)
	if(Charger.current_Active_Power <= CHARGSERV_UNDERPOWER_VALUE)
	{
		return _TRUE;
	}
	else
	{
		return _FALSE;
	}
#else
	uint8_t value;

	value = charger_is_load_detect_flug();

	if(_FALSE == value)
	{
		return _TRUE;
	}
	else
	{
		return _FALSE;
	}
#endif
}
#endif

void charger_set_startpoint_energy(uint32_t energy)
{
	Charger.startpoint_Energy = energy;
}

void Charger_reset_use_energy()
{
	Charger.use_energy = 0;
}

uint32_t charger_calc_use_energy()
{
	uint32_t ret_value;

	if(Charger.current_Active_Energy > Charger.startpoint_Energy)
	{
		ret_value = (Charger.current_Active_Energy - Charger.startpoint_Energy);
	}
	else if(Charger.current_Active_Energy < Charger.startpoint_Energy)
	{
		ret_value = ((CHARGSERV_MAXIMUM_ENERGY_VALUE) - Charger.startpoint_Energy + Charger.current_Active_Energy + 1);
	}
	else
	{
		ret_value = 0;
	}

	Charger.use_energy = ret_value;

	return ret_value;
}

uint32_t charger_get_use_energy()
{
	return Charger.use_energy;
}

uint32_t charger_calc_won(double unit_price)
{
#if (((_CERTIFICATION_MODE_)!=_CERTIFICATION_NON_))
	uint32_t temp  = 0;
	double db_temp = 0.0;
	double won = 0.0;
	uint32_t gu32_won = 0;

	temp = Charger.use_energy;
	db_temp = ((double)temp) / 100;

	won = db_temp * unit_price; //(에너지/100) * 1kW당 요금
	gu32_won = (uint32_t)(round(won));

	Charger.use_won = gu32_won;

	return gu32_won;

#else

	//Todo
	// 1). cals delta time to use energy
	// 2). won = delta time to use energy * uint_price

	return 0;
#endif
}

uint32_t charger_get_won()
{
	return Charger.use_won;
}

void _APP_CHARGSERV_state_machine()
{
	eCharger_State state = _APP_CHARGSERV_get_current_state();

	switch(state)
	{
		case Init :


			if((1 == Charger.reg.init_complete) &&
			   (1 == Charger.reg.display_cyclefinish))
			{
				charger_change_state(Init, Ready);
				//, CHARGSERV_REG_READY_STANDBY
				Charger.reg.ready_standby = 1;
			}
		break;

		case Ready :
			//if(_ON == charger_state_is_changed())
			if(1 == Charger.reg.state_changed)
			{
				Charger.reg.state_changed = 0;
				//CHARGSERV --> RFIDAPP : TAG_START
				//charger_msg_set(msg_tx, CHARGSERV_MQ_TX_RFIDAPP_TAG_START);


#if ((_AUTOCHECKUSER_) == 0)
				if(0 == Charger.autostartmode_flag)
				{
					_APP_RFID_set_start_tagging(0);
				}
#endif
			}

			if(1 == Charger.reg.ready_cardtag)
			{
				charger_change_state(Ready, UserCheck);
				//, CHARGSERV_REG_USERCHECK_TAGOK
				Charger.reg.usercheck_tagok = 1;
			}
#if 0
			else if(1 == Charger.reg.ready_ota_request)
			{
				charger_change_state(Ready, OTA);
				//, CHARGSERV_REG_OTA_PROCEEDING
				Charger.reg.ota_proceeding = 1;
			}
			else if(1 == Charger.reg.ready_remote_start)
			{
				_APP_RFID_set_start_tagging(1);
				Charger.reg.ready_remote_start = 0;
				charger_change_state(Ready, Connect);
				Charger.reg.connect_wait = 1;
			}
#endif

			if(1 == Charger.reg.ready_automodeflag)
			{
				charger_change_state(Ready, AutoReady);
				//, CHARGSERV_REG_READY_STANDBY
				Charger.reg.autoready_standby = 1;
			}

		break;

		case AutoReady :
			if(1 == Charger.reg.state_changed)
			{
				Charger.reg.state_changed = 0;
				//CHARGSERV --> RFIDAPP : TAG_START
				//charger_msg_set(msg_tx, CHARGSERV_MQ_TX_RFIDAPP_TAG_START);
				_APP_RFID_set_start_tagging(1);
			}

			if(1 == Charger.reg.autoready_connected)
			{
				charger_change_state(AutoReady, Connect);
				Charger.reg.connect_wait = 1;
			}
			else if(1 == Charger.reg.autoready_manualmodeflag)
			{
				charger_change_state(AutoReady, Ready);
				//, CHARGSERV_REG_READY_STANDBY
				Charger.reg.ready_standby = 1;
			}

		break;

		case UserCheck :
			if(1 == Charger.reg.state_changed)
			{
				Charger.reg.state_changed = 0;

				//start timeout.
#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV : UserCheck_Timeout_start #### \r\n");
#endif
				_LIB_USERDELAY_start(&gTimeout_usercheck,DELAY_RENEW_OFF);

			}

			if(_LIB_USERDELAY_isfired(&gTimeout_usercheck))
			{
				_LIB_USERDELAY_stop(&gTimeout_usercheck);
#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV : UserCheck_Timeout. go to Ready #### \r\n");
#endif
				_APP_CHARGSERV_usercheck_user_retry();
			}

			if((1 == Charger.reg.usercheck_userok) &&
			   (1 == Charger.reg.display_cyclefinish))
			{
				_LIB_USERDELAY_stop(&gTimeout_usercheck);
				charger_change_state(UserCheck, Connect);
				//, CHARGSERV_REG_CONNECT_WAIT
				Charger.reg.connect_wait = 1;
			}
			else if((1 == Charger.reg.usercheck_userretry)&&
					(1 == Charger.reg.display_cyclefinish))
			{
				charger_change_state(UserCheck, Ready);
				//, CHARGSERV_REG_READY_STANDBY
				Charger.reg.ready_standby = 1;
			}
		break;

		case Connect :
			if((1 == Charger.reg.connect_wired) &&
			   (1 == Charger.reg.display_cyclefinish))
			{
				charger_change_state(Connect, Charging);
				//, CHARGSERV_REG_CHARGING_PROCEEDING
				Charger.reg.charging_proceeding = 1;
			}
			else if((1 == Charger.reg.connect_timeout) &&
					(1 == Charger.reg.display_cyclefinish))
			{
				charger_change_state(Connect, Ready);
				//, CHARGSERV_REG_READY_STANDBY
				Charger.reg.ready_standby = 1;
			}
		break;

		case Charging :

			if(1 == Charger.reg.state_changed)
			{
				Charger.reg.state_changed = 0;
#if ((_AUTOCHECKUSER_) == 0)
				//CHARGSERV --> RFIDAPP : TAG_START
				//charger_msg_set(msg_tx, CHARGSERV_MQ_TX_RFIDAPP_TAG_START);
				//_APP_RFID_set_start_tagging(0);
#else

#endif
				_APP_RFID_set_start_tagging(0);

			if(1 == Charger.reg.charging_ok)
			{
				charger_change_state(Charging, Finish);
				//, CHARGSERV_REG_FINISH_PROCEEDING
				Charger.reg.finish_proceeding = 1;
				Charger.reg.finish_stoptranscation_flag = 0;
			}
		}
		break;

		case Finish :

			if((1 == Charger.reg.finish_ok) &&
			   (1 == Charger.reg.display_cyclefinish))
			{
				if(1 == Charger.automode_active)
				{
					charger_change_state(Finish, AutoReady);
					Charger.reg.autoready_standby = 1;

				}
				else
				{
					charger_change_state(Finish, Ready);
					//, CHARGSERV_REG_READY_STANDBY
					Charger.reg.ready_standby = 1;
				}
			}
		break;

		case OTA :


			if(1 == Charger.reg.ota_ok)
			{
				charger_change_state(OTA, Ready);
				//, CHARGSERV_REG_READY_STANDBY
				Charger.reg.ready_standby = 1;
			}
		break;

		case Fault :

			if((1 == Charger.reg.fault_reset) &&
			   (1 == Charger.reg.display_cyclefinish))
			{
				if(1 == Charger.automode_active)
				{
					charger_change_state(Fault, AutoReady);
					Charger.reg.autoready_standby = 1;
				}
				else
				{
					charger_change_state(Fault, Ready);
					//, CHARGSERV_REG_READY_STANDBY
					Charger.reg.ready_standby = 1;
				}
			}

		break;

		case Powercut :

			if((1 == Charger.reg.powercut_timepass) &&
			   (1 == Charger.reg.display_cyclefinish))
			{
				_LIB_LOGGING_printf("Powercut Timepass\r\n");
				charger_change_state(Powercut, Ready);
				//, CHARGSERV_REG_READY_STANDBY
				Charger.reg.ready_standby = 1;
			}
		break;

		default :

		break;
	}
}

void charger_indiled_display()
{
	eCharger_State state = _APP_CHARGSERV_get_current_state();
	CP_StateDef	cp_state = _MW_CP_get_cp_state();
	static uint8_t indiledtickcount = 0;
	

	indiledtickcount++;

	switch(state)
	{
		case Init :
			_MW_INDILED_sled_ctl(PURPLE);
		break;

		case Ready :
			if(_OFF == _MW_GPIO_get_gpi(EMG))
			{
				_MW_INDILED_sled_ctl(RED);
			}
			else
			{
				_MW_INDILED_sled_ctl(WHITE);
			}

		break;

		case AutoReady :

			if(TEMP_ERR == 1) //온도 에러 상태 표시
			{
				if((indiledtickcount == 1) || (indiledtickcount == 2) || (indiledtickcount == 3))
				{
					_MW_INDILED_sled_ctl(GREEN);
				}
				else if((indiledtickcount == 4) || (indiledtickcount == 5) || (indiledtickcount == 6))
				{
					_MW_INDILED_sled_ctl(PURPLE);
				}
			}
			else
			{
				_MW_INDILED_sled_ctl(BLUE);
			}
		break;

		case UserCheck :
		case Connect :
			_MW_INDILED_sled_ctl(BLUE);
		break;

		case Charging :
			//if((DC_6V == cp_state) || (PWM_6V == cp_state))
			if(PWM_6V == cp_state)
			{
				if(AC_LV_ERR == 1) //저전압 상태 표시
				{
					if((indiledtickcount == 1) || (indiledtickcount == 2) || (indiledtickcount == 3))
					{
						_MW_INDILED_sled_ctl(GREEN);
					}
					else if((indiledtickcount == 4) || (indiledtickcount == 5) || (indiledtickcount == 6))
					{
						_MW_INDILED_sled_ctl(YELLOW);
					}
				}
				else if(TEMP_ERR == 1) //온도 에러 상태 표시
				{
					if((indiledtickcount == 1) || (indiledtickcount == 2) || (indiledtickcount == 3))
					{
						_MW_INDILED_sled_ctl(GREEN);
					}
					else if((indiledtickcount == 4) || (indiledtickcount == 5) || (indiledtickcount == 6))
					{
						_MW_INDILED_sled_ctl(PURPLE);
					}
				}
				else
				{
					_MW_INDILED_sled_ctl(GREEN);
				}

			}
			else if (DC_6V == cp_state)
			{
				_MW_INDILED_sled_ctl(RED);
			}
			else
			{

				if(1 == Charger.forcestop_wake_up_seq_count_limit_detect) //wake_up_seq_count_limit에 도달하면
				{
					if((indiledtickcount == 1) || (indiledtickcount == 2) || (indiledtickcount == 3))
					{
						_MW_INDILED_sled_ctl(BLUE);
					}
					else if((indiledtickcount == 4) || (indiledtickcount == 5) || (indiledtickcount == 6))
					{
						_MW_INDILED_sled_ctl(BLACK);
					}
				}
				else if(1 == Charger.stable_full_detect)
				{
					_MW_INDILED_sled_ctl(SKY);
				}
				else
				{
					_MW_INDILED_sled_ctl(BLUE);
				}

			}

		break;

		case Finish :
			_MW_INDILED_sled_ctl(SKY);
		break;

		case OTA :
			_MW_INDILED_sled_ctl(BLACK);
		break;

		case Fault :
			
			if((indiledtickcount == 1) || (indiledtickcount == 2))
			{
				if((0 == charger_fault_status_bak.Raw))
				{
					_MW_INDILED_sled_ctl(RED);
					charger_fault_status_bak = charger_fault_status;

				}
//				if((0 == charger_fault_status_bak.Raw) && (charger_fault_status.OTEMP_ERR ==_OFF))
//				{
//					_MW_INDILED_sled_ctl(RED);
//					charger_fault_status_bak = charger_fault_status;
//
//				}
				// else if ((0 == charger_fault_status_bak.Raw) && (charger_fault_status.OTEMP_ERR ==_ON))
				// {
				// 	_MW_INDILED_sled_ctl(GREEN);
				// 	charger_fault_status_bak = charger_fault_status;
				// }
			}
			else if((indiledtickcount == 3) || (indiledtickcount == 4))
			{

				if(_ON == charger_fault_status_bak.AC_OC_ERR)
				{
					_MW_INDILED_sled_ctl(YELLOW);
				}
				else if(_ON == charger_fault_status_bak.LEAKAGE_ERR)
				{
					_MW_INDILED_sled_ctl(GREEN);
				}
				else if(_ON == charger_fault_status_bak.MC_START_ERR)
				{
					_MW_INDILED_sled_ctl(BLUE);
				}
				else if(_ON == charger_fault_status_bak.MC_STOP_ERR)
				{
					_MW_INDILED_sled_ctl(BLUE);
				}
				else if(_ON == charger_fault_status_bak.AC_OV_ERR)
				{
					_MW_INDILED_sled_ctl(SKY);
				}
//				else if(AC_fault_status_bak == 1)
//				{
//					_MW_INDILED_sled_ctl(YELLOW);
//				}
				else if(_ON == charger_fault_status_bak.CP_ERR)
				{
					_MW_INDILED_sled_ctl(PURPLE);
				}
				else if(_ON == charger_fault_status_bak.CP_VOLTAGE_ERR)
				{
					_MW_INDILED_sled_ctl(PURPLE);
				}
				// else if(_ON == charger_fault_status_bak.OTEMP_ERR)
				// {
				// 	_MW_INDILED_sled_ctl(PURPLE);
				// }
				else if(_ON == charger_fault_status_bak.RFID_COMM_ERR)
				{
					_MW_INDILED_sled_ctl(WHITE);
				}
			}
			else if((indiledtickcount == 5) || (indiledtickcount == 6))
			{
				if(_ON == charger_fault_status_bak.AC_OC_ERR)
				{
					_MW_INDILED_sled_ctl(YELLOW);
					charger_fault_status_bak.AC_OC_ERR = _OFF;
				}
				else if(_ON == charger_fault_status_bak.LEAKAGE_ERR)
				{
					_MW_INDILED_sled_ctl(GREEN);
					charger_fault_status_bak.LEAKAGE_ERR = _OFF;
				}
				else if(_ON == charger_fault_status_bak.MC_START_ERR)
				{
					_MW_INDILED_sled_ctl(BLUE);
					charger_fault_status_bak.MC_START_ERR = _OFF;
				}
				else if(_ON == charger_fault_status_bak.MC_STOP_ERR)
				{
					_MW_INDILED_sled_ctl(BLUE);
					charger_fault_status_bak.MC_STOP_ERR = _OFF;
				}
				else if(_ON == charger_fault_status_bak.AC_OV_ERR)
				{
					_MW_INDILED_sled_ctl(SKY);
					charger_fault_status_bak.AC_OV_ERR = _OFF;
				}
//				else if(AC_fault_status_bak == 1)
//				{
//					_MW_INDILED_sled_ctl(YELLOW);
//					AC_fault_status_bak = 0;
//				}
				else if(_ON == charger_fault_status_bak.CP_ERR)
				{
					_MW_INDILED_sled_ctl(PURPLE);
					charger_fault_status_bak.CP_ERR = _OFF;
				}
				else if(_ON == charger_fault_status_bak.CP_VOLTAGE_ERR)
				{
					_MW_INDILED_sled_ctl(SKY);
					charger_fault_status_bak.CP_VOLTAGE_ERR = _OFF;
				}
				// else if(_ON == charger_fault_status_bak.OTEMP_ERR)
				// {
				// 	_MW_INDILED_sled_ctl(PURPLE);
				// 	charger_fault_status_bak.OTEMP_ERR = _OFF;
				// }
				else if(_ON == charger_fault_status_bak.RFID_COMM_ERR)
				{
					_MW_INDILED_sled_ctl(WHITE);
					charger_fault_status_bak.RFID_COMM_ERR = _OFF;
				}
			}

		default :

		case Powercut :
#if 0
			if((indiledtickcount == 1) || (indiledtickcount == 2))
			{
				_MW_INDILED_sled_ctl(RED);
			}
			else if((indiledtickcount == 3) || (indiledtickcount == 4))
			{
				_MW_INDILED_sled_ctl(BLACK);
			}
#endif
		break;

		break;
	}

	if(indiledtickcount >= 6)
	{
		indiledtickcount = 0;
	}

}

void charger_dipled_display()
{
	eCharger_Mode mode = charger_get_mode();
	static uint8_t dipledtickcount = 0;
	static uint8_t load_led_flag = 0;

	dipledtickcount++;

	//if(_ON == _MW_CP_get_mc_relay_state())
	if(_ON == _MW_GPIO_get_gpi(WELD))
	{
#if 0
		_MW_GPIO_set_gpo(KWH_LED, _ON);
		if(_TRUE == charger_is_load_detect_evse())
		{
			if((dipledtickcount >= 0) && (dipledtickcount < 50))
			{
				//_MW_GPIO_set_gpo(LOAD_LED, _ON);
				load_led_flag = _ON;
			}
			else if((dipledtickcount >= 50) && (dipledtickcount < 100))
			{
				//_MW_GPIO_set_gpo(LOAD_LED, _OFF);
				load_led_flag = _OFF;
			}
		}
		else
		{
			_MW_GPIO_set_gpo(LOAD_LED, _OFF);
		}
#else

#endif
	}
	else
	{
#if 0
		_MW_GPIO_set_gpo(KWH_LED, _OFF);
		_MW_GPIO_set_gpo(LOAD_LED, _OFF);
#else

#endif
	}

	if((dipledtickcount >= 0) && (dipledtickcount < 50))
	{
		if(_ON == load_led_flag)
		{
			//_MW_GPIO_set_gpo(LOAD_LED, _ON);
		}
		_MW_GPIO_set_gpo(STUS_LED_01, _ON);
	}
	else if((dipledtickcount >= 50) && (dipledtickcount < 100))
	{
		if(_OFF == load_led_flag)
		{
			//_MW_GPIO_set_gpo(LOAD_LED, _OFF);
		}
		_MW_GPIO_set_gpo(STUS_LED_01, _OFF);
	}

	if(dipledtickcount >= 100)
	{
		dipledtickcount = 0;
	}
}

void _APP_CHARGSERV_display_loop()
{
	static uint8_t tickcyclecnt = 0;

	if(tickcyclecnt >= 25)
	{
		charger_indiled_display();
		tickcyclecnt = 0;
	}
	tickcyclecnt++;

	charger_dipled_display();
}

void charger_cp_ry_control()
{
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();
	CP_StateDef	cp_state = _MW_CP_get_cp_state();

	switch(charger_state)
	{
		case Ready :
		case AutoReady :
		case UserCheck :
#if ((_CP_ALWAYS_ON_) == 0)
			_MW_CP_cp_relay_ctl(_OFF);
#else
			_MW_CP_cp_relay_ctl(_ON);
#endif
		break;

		case Connect :
			_MW_CP_cp_relay_ctl(_ON);
		break;

		case Charging :
			if(charger_get_wake_up_seq_step() == CP_OFF)
			{
				_MW_CP_cp_relay_ctl(_OFF);
			}
			else
			{
				_MW_CP_cp_relay_ctl(_ON);
			}
		break;

		case Finish :
			if(cp_state == DC_12V)
			{
#if ((_CP_ALWAYS_ON_) == 0)
				_MW_CP_cp_relay_ctl(_OFF);
#else
				_MW_CP_cp_relay_ctl(_ON);
#endif
				//_APP_CHARGSERV_finish_ok();
			}
		break;

		case Fault :
			//_MW_CP_cp_relay_ctl(_OFF);
		break;

		default :

		break;
	}
}

#if 0
void charger_cp_state_control()
{
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();
	CP_StateDef	cp_state = _MW_CP_get_cp_state();

	static uint8_t is_charging_sign = 0; // 0 : It never goes to 'PWM_6V' after starting the first charge.	// 1 : It becomes 'PWM_6V' at least once after the first charging starts.

	switch(charger_state)
	{
		case Connect :
			if((cp_state == DC_9V) || (cp_state == DC_6V))
			{
				if(cp_state != DC_6V)
				{
					charger_wake_up_seq_enable();
				}

				_APP_CHARGSERV_set_duty_ondelay(CHARGSERV_DUTY_ONDELAY);

				if(_TRUE == _APP_CHARGSERV_complete_duty_ondelay())
				{
					_MW_CP_set_pwm_duty(_MW_CP_cal_ampe_to_duty(_APP_CHARGSERV_get_active_Ampe()));
					charger_wake_up_seq_start();
					_APP_CHARGSERV_connect_ok();
					//Todo
					//calculate use active Energy start
					charger_set_startpoint_energy(Charger.current_Active_Energy);
					Charger_reset_use_energy();
				}
			}
		break;

		case Charging :
			//if((_TRUE == _MW_CP_get_mc_relay_state()) && ((cp_state == PWM_12V)))

			charger_calc_use_energy();
			charger_calc_won(250);
			//Todo
			//charger_update_use_energy(); 	// 1). update use energy to CSMS APP TASK
			//charger_update_won();			// 2). update won from CSMS APP TASK

			if((charger_get_wake_up_seq_step() == CP_OFF) || (charger_get_wake_up_seq_step() == PWM_DISABLE))
			{
				_MW_CP_set_pwm_duty(100);
			}
			else
			{
				_MW_CP_set_pwm_duty(_MW_CP_cal_ampe_to_duty(_APP_CHARGSERV_get_active_Ampe()));
			}

			if((cp_state == Err_PWMH) || (cp_state == Err_PWML) || (cp_state == Err_DC))
			{
				charger_wake_up_seq_forced_stop();
				_APP_CHARGSERV_charging_ok();	// Todo : must to change fault processing
			}

			if(cp_state == PWM_6V)
			{
				charger_wake_up_seq_forced_stop();
				if(is_charging_sign == 0)
				{
					is_charging_sign = 1;
#if ((__CHARGSERV_DEBUG__)==1)
					_LIB_LOGGING_printf("#### CHARGSERV : is_charging_sign = 1 #### \r\n");
#endif
				}
				_LIB_USERDELAY_stop(&gTimeout_auto_finish);
			}

			if(cp_state == PWM_9V)
			{
				if(is_charging_sign == 1)
				{
					_LIB_USERDELAY_start(&gTimeout_auto_finish, DELAY_RENEW_OFF);

					if(_LIB_USERDELAY_isfired(&gTimeout_auto_finish))
					{
						_LIB_USERDELAY_stop(&gTimeout_auto_finish);
#if ((__CHARGSERV_DEBUG__)==1)
						_LIB_LOGGING_printf("#### CHARGSERV : auto finish #### \r\n");
#endif
						_APP_CHARGSERV_charging_ok();
					}
				}
			}

#if ((_FORCE_ACTIVING_MODE_)==0)
			//In '_FORCE_ACTIVING_MODE_' mode, the following functions must be deactivated to maintain 'Charging' state.
			if(cp_state == PWM_12V)
			{
				charger_wake_up_seq_forced_stop();

				_LIB_USERDELAY_stop(&gTimeout_auto_finish);

				//if(_MW_CP_get_mc_relay_state() == _ON)
				if(is_charging_sign == 1)
				{
#if 0
					_APP_CHARGSERV_charging_ok();  // after, must to change fault processing
					//Todo
					//calculate use active Energy end
#else
#if ((__CHARGSERV_DEBUG__)==1)
						if(0 == Charger.reg.charging_compulsionunplug)
						{
							_LIB_LOGGING_printf("#### CHARGSERV : compulsion unplug #### \r\n");
						}
#endif

						_APP_CHARGSERV_set_charging_to_unplug();
						if(Charger.reg.charging_continuous == 0)
						{
							//fast mc off
							_MW_CP_mc_relay_off_fast();
#if ((__CHARGSERV_DEBUG__)==1)
							_LIB_LOGGING_printf("#### CHARGSERV : fast mc off #### \r\n");
#endif
							_APP_CHARGSERV_charging_ok();
						}
#if 0					// if 'StopTransactionOnEVSideDisconnect' is 'false', 'CP open Fault' don't support.
						else
						{
							if(0)
							{
								_APP_CHARGSERV_charging_ok();
							}
						}
#endif
#endif
				}
				else
				{
					//fast mc off
					_MW_CP_mc_relay_off_fast();
#if ((__CHARGSERV_DEBUG__)==1)
					_LIB_LOGGING_printf("#### CHARGSERV : fast mc off #### \r\n");
#endif
					_APP_CHARGSERV_charging_ok();
				}
			}
#endif
		break;

		case Finish :
			if(is_charging_sign == 1)
			{
				is_charging_sign = 0;
			}

#if 0
			if((cp_state == PWM_12V) || (cp_state == PWM_9V))
#else		//220623 PES : PWM_6V also
			if((cp_state == PWM_12V) || (cp_state == PWM_9V) || (cp_state == PWM_6V))
#endif
			{
				_MW_CP_set_pwm_duty(100);
			}
			if(cp_state == DC_12V)
			{
				_APP_CHARGSERV_finish_ok();
				Charger.reg.charging_id_tagging_stop = 0;
				//Charger_reset_use_energy();
			}
		break;

		case Fault :
			_MW_CP_set_pwm_duty(100);
		default :

		break;
	}

}
#else	//220802 PES : wake-up squence , full charging detect
void charger_cp_state_control()
{
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();
	CP_StateDef	cp_state = _MW_CP_get_cp_state();

	int wake_up_seq_count_limit = 0;

	//static uint8_t is_charging_sign = 0; // 0 : It never goes to 'PWM_6V' after starting the first charge.	// 1 : It becomes 'PWM_6V' at least once after the first charging starts.
	//static uint8_t is_charging_stable = 0; //
	//static uint8_t is_charging_full = 0;//

	switch(charger_state)
	{
		case UserCheck :
			if(Charger.chargstate > Charg_Standby)//if(is_charging_sign == 1)
			{
				Charger.chargstate = Charg_Standby;//is_charging_sign = 0;
#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV : chargstate : Standby(UserCheck) #### \r\n");
#endif
			}
		break;

		case Connect :
			if(((cp_state == DC_9V) || (cp_state == DC_6V)) && (0 == Charger.reg.connect_timeout))
			{
				_LIB_USERDELAY_stop(&gTimeout_connect);
				_APP_CHARGSERV_set_duty_ondelay(CHARGSERV_DUTY_ONDELAY);

				if(_TRUE == _APP_CHARGSERV_complete_duty_ondelay())
				{
					_MW_CP_set_pwm_duty(_MW_CP_cal_ampe_to_duty(_APP_CHARGSERV_get_active_Ampe())); //전류값에 따라서 pwm duty 설정
					_APP_CHARGSERV_connect_ok();
					Charger.full_wake_up_seq_count = 0;
					Charger.forcestop_wake_up_seq_count = 0;
					Charger.forcestop_wake_up_seq_flag = 0;
					Charger.forcestop_wake_up_seq_count_limit_detect = 0;
					Charger.stable_full_detect = 0;
					//Todo
					//calculate use active Energy start
					charger_set_startpoint_energy(Charger.current_Active_Energy); //충전 시작 부분 유효전력 저장
					Charger_reset_use_energy(); //초기화
				}
			}
		break;

		case Charging :
			int cp_v_check = ((int)_MW_CP_get_h_final_voltage() * 100);
			//if((_TRUE == _MW_CP_get_mc_relay_state()) && ((cp_state == PWM_12V)))

			charger_calc_use_energy();
			charger_calc_won(250);
			//Todo
			//charger_update_use_energy(); 	// 1). update use energy to CSMS APP TASK
			//charger_update_won();			// 2). update won from CSMS APP TASK

			if((charger_get_wake_up_seq_step() == CP_OFF) || (charger_get_wake_up_seq_step() == PWM_DISABLE) || (1 == Charger.leakage_instop_flag))
			{
				_MW_CP_set_pwm_duty(100);
			}
			else
			{
				_MW_CP_set_pwm_duty(_MW_CP_cal_ampe_to_duty(_APP_CHARGSERV_get_active_Ampe()));
			}

			if((cp_state == Err_PWMH) || (cp_state == Err_PWML) || (cp_state == Err_DC))
			{
				if(Charger.chargstate > Charg_Standby)//if(is_charging_sign == 1) //start, stable, full
				{
					Charger.chargstate = Charg_Standby;//is_charging_sign = 0;
#if ((__CHARGSERV_DEBUG__)==1)
					_LIB_LOGGING_printf("#### CHARGSERV : chargstate : Standby(fault) #### \r\n");
#endif
				}
				charger_wake_up_seq_forced_stop();
				_LIB_USERDELAY_stop(&gTimeout_auto_finish);

				//cp 전압 오류 cp 동작오류 구분 로직
				if(cp_v_check == 0)
				{
					//printf("cp_v : %d\r\n", cp_v_check);
					_APP_CHARGSERV_cp_fault_set(); //cp 동작오류
				}
				else
				{
					_APP_CHARGSERV_cp_voltage_fault_set(); //cp 전압 오류
				}
				

				
				
			}

			if(1 == Charger.leakage_instop_flag)
			{
				switch(Charger.leakage_instop_step)
				{
					case 0 :
						charger_wake_up_seq_enable();
						charger_wake_up_seq_start(99); //wake_up
						Charger.leakage_instop_step = 1;
#if ((__CHARGSERV_DEBUG__)==1)
						_LIB_LOGGING_printf("#### CHARGSERV : Leakage_instop_wake_up_seq_start #### \r\n");
#endif
					break;
					case 1 :
						if(0 == charger_get_wake_up_seq_flag())
						{
							Charger.leakage_instop_step = 2;
#if ((__CHARGSERV_DEBUG__)==1)
						_LIB_LOGGING_printf("#### CHARGSERV : Leakage_instop_wake_up_seq_OK #### \r\n");
#endif
						}
					break;
					case 2 :
						charger_wake_up_seq_forced_stop();

						Charger.leakage_instop_flag = 0;
						Charger.leakage_instop_step = 3;

						_LIB_USERDELAY_start(&gTimout_Leakage_Relapse, DELAY_RENEW_OFF);
#if ((__CHARGSERV_DEBUG__)==1)
						_LIB_LOGGING_printf("#### CHARGSERV : Leakage_instop_Relapse_timeout_start #### \r\n");
#endif
					break;
					default :

					break;
				}


				break;
			}
			if(3 == Charger.leakage_instop_step)
			{
				if(_LIB_USERDELAY_isfired(&gTimout_Leakage_Relapse))
				{
					_LIB_USERDELAY_stop(&gTimout_Leakage_Relapse);
					Charger.leakage_instop_step = 0;
#if ((__CHARGSERV_DEBUG__)==1)
						_LIB_LOGGING_printf("#### CHARGSERV : Leakage_instop_Relapse_timeout_clear #### \r\n");
#endif
				}
			}

			if(cp_state == PWM_6V)
			{
					//전류값이 최소 값 40A 이상일 때
				if((0 == Charger.forcestop_wake_up_seq_flag) && (Charger.current_I_rms > CHARGSERV_AMPE_MIN_VALUE))
				{
					_LIB_USERDELAY_start(&gDelay_forcestop_wake_up_seq_flag_set, DELAY_RENEW_OFF); //3초

					if(_LIB_USERDELAY_isfired(&gDelay_forcestop_wake_up_seq_flag_set))
					{
						_LIB_USERDELAY_stop(&gDelay_forcestop_wake_up_seq_flag_set);
						Charger.forcestop_wake_up_seq_flag = 1;
#if ((__CHARGSERV_DEBUG__)==1)
						_LIB_LOGGING_printf("#### CHARGSERV : forcestop_wake_up_seq start!! #### \r\n");
#endif
					}
				}
				else
				{
					_LIB_USERDELAY_stop(&gDelay_forcestop_wake_up_seq_flag_set);
				}

				switch(Charger.chargstate)
				{
					case Charg_Standby :
						charger_wake_up_seq_forced_stop();
						_LIB_USERDELAY_stop(&gTimeout_auto_finish);
						_LIB_USERDELAY_stop(&gTimeout_charging_stable);
						_LIB_USERDELAY_stop(&gTimeout_charging_full);
						_LIB_USERDELAY_stop(&gTimeout_forcestop_wake_up_seq_is_finish);
						_LIB_USERDELAY_stop(&gTimeout_full_wake_up_seq_is_finish);
						Charger.forcestop_wake_up_seq_count = 0;
						Charger.forcestop_wake_up_seq_count_limit_detect = 0;
						Charger.chargstate = Charg_Start;//is_charging_sign = 1;
#if ((__CHARGSERV_DEBUG__)==1)
						_LIB_LOGGING_printf("#### CHARGSERV : chargstate : Start #### \r\n");
#endif
					break;
					case Charg_Start :
						//if(Charger.current_I_rms > CHARGSERV_AMPE_MAX_HALF_VALUE)
						if(Charger.current_I_rms > (Charger.active_Ampe * 500)) //기준 전류 절반 이상으로 90분 지속했을 경우
						{
							_LIB_USERDELAY_start(&gTimeout_charging_stable, DELAY_RENEW_OFF); //90분

							if(_LIB_USERDELAY_isfired(&gTimeout_charging_stable))
							{
								_LIB_USERDELAY_stop(&gTimeout_charging_stable);
								Charger.chargstate = Charg_Stable;
								Charger.forcestop_wake_up_seq_count = 0;
#if ((__CHARGSERV_DEBUG__)==1)
								_LIB_LOGGING_printf("#### CHARGSERV : chargstate : Stable(PWM6V) #### \r\n");
#endif
							}
						}
						else
						{
							_LIB_USERDELAY_stop(&gTimeout_charging_stable);
						}
					break;
					case Charg_Stable :
						//if((Charger.current_I_rms <= CHARGSERV_AMPE_MAX_HALF_VALUE) && (Charger.current_I_rms >= CHARGSERV_AMPE_MIN_VALUE))
						if(1 == Charger.stable_changed_flag)
						{
							Charger.stable_changed_flag = 0;
							Charger.full_wake_up_seq_count = 0;
							Charger.stable_full_detect = 0;
							charger_wake_up_seq_forced_stop();
							_LIB_USERDELAY_stop(&gTimeout_charging_full);
							_LIB_USERDELAY_stop(&gTimeout_full_wake_up_seq_is_finish);
#if ((__CHARGSERV_DEBUG__)==1)
							_LIB_LOGGING_printf("#### CHARGSERV : chargstate : Stable(PWM6V)_full_wake_up_seq_reinit #### \r\n");
#endif
						}
#if ((_DO_NOT_RESET_WAKE_UP_SEQ_COUNT_) == 0)
						if(Charger.full_wake_up_seq_count > 0)
						{
							Charger.full_wake_up_seq_count = 0;
						}
#endif

					break;

					case Charg_Full :
						//_LIB_USERDELAY_stop(&gTimeout_auto_finish);
					break;

					default :

					break;
				}
			}
			if(cp_state == PWM_9V)
			{
				switch(Charger.chargstate)
				{

					case Charg_Standby :

						//230711 PES PWM 9V_ForceStop Wakeup Seq count

						if(0 == Charger.forcestop_wake_up_seq_flag)//충전 중이면 1
						{
							wake_up_seq_count_limit = 1; // 커플러와 차량이 연결되고도 일정 시간 경과 후에도 충전이 시작되지 않을 경우
						}
						else
						{
							wake_up_seq_count_limit = 3; //안정적 충전 상태가 아닌 상황에서 충전 중에 일시 정지 된 경우
						}

						if(Charger.forcestop_wake_up_seq_count >= wake_up_seq_count_limit)//wake_up 시퀀스가 특정 횟수 초과 시
						{
							_LIB_USERDELAY_stop(&gTimeout_forcestop_wake_up_seq_is_finish);

							if(0 == Charger.forcestop_wake_up_seq_count_limit_detect)
							{
								charger_wake_up_seq_forced_stop();
								Charger.forcestop_wake_up_seq_count_limit_detect = 1; //특정 횟수 초과 감지
#if ((__CHARGSERV_DEBUG__)==1)
								_LIB_LOGGING_printf("#### CHARGSERV : chargstate : ForceStop -> count limit(PWM9V) #### \r\n");
#endif
							}

						}
						else
						{
							if(0 == charger_get_wake_up_seq_flag())
							{
								if(0 == Charger.forcestop_wake_up_seq_flag) //40A이상일 때 1
								{
									charger_wake_up_seq_enable(); //wake_up_seq_flag = 1
									charger_wake_up_seq_start(0); //wake_up_seq 30초 시작
								}
								else
								{
									if(0 == Charger.restart_fulldetect_act)//DIPSW 1이 ON -> 충전재시작, 완충감지 기능 설정 해제
									{
										charger_wake_up_seq_enable();
										charger_wake_up_seq_start(Charger.forcestop_wake_up_seq_count + 1); //wake_up_seq 120초, 600초, 1800초
									}
								}
							}

						}

#if 1					//240626 PES : always wake up seq count
						if(_TRUE == charger_wake_up_seq_is_finish()) //wake_up_seq 완료
						{
							_LIB_USERDELAY_start(&gTimeout_forcestop_wake_up_seq_is_finish, DELAY_RENEW_OFF); //25초
#if ((__CHARGSERV_DEBUG__)==1)
							_LIB_LOGGING_printf("#### CHARGSERV : forcestop_wake_up_seq_ is finish start #### \r\n");
#endif
						}
#endif

						if(_LIB_USERDELAY_isfired(&gTimeout_forcestop_wake_up_seq_is_finish)) //25초 경과 시
						{
							_LIB_USERDELAY_stop(&gTimeout_forcestop_wake_up_seq_is_finish);

#if ((_DO_NOT_FAULT_LEAKAGE_)==1)
							//if((3 != Charger.leakage_instop_step) && (3 != Charger.oc_instop_step))
							if((3 != Charger.leakage_instop_step)) //누설전류 감지가 되지 않았다면						
							{
								Charger.forcestop_wake_up_seq_count++;
							}
#else
							Charger.forcestop_wake_up_seq_count++;
#endif
#if ((__CHARGSERV_DEBUG__)==1)
							_LIB_LOGGING_printf("#### CHARGSERV : chargstate : Standby_wake-up-squ(PWM9V) #### \r\n");
							_LIB_LOGGING_printf("#### CHARGSERV : chargstate : forcestop_wake_up_seq_count : %d #### \r\n", Charger.forcestop_wake_up_seq_count);
#endif
						}
					break;

					case Charg_Start :
						Charger.chargstate = Charg_Standby;
#if ((__CHARGSERV_DEBUG__)==1)
						_LIB_LOGGING_printf("#### CHARGSERV : chargstate : Standby(PMW6V -> PWM9V) #### \r\n");
#endif
					break;

					case Charg_Stable : //충전 상태가 안정적일 경우
						
						if(Charger.full_wake_up_seq_count >= CHARGSERV_FULL_WAKE_UP_SEQ_COUNT)
						{
							_LIB_USERDELAY_stop(&gTimeout_full_wake_up_seq_is_finish);

							if(0 == Charger.stable_full_detect)
							{
								charger_wake_up_seq_forced_stop();
								Charger.stable_full_detect = 1;
#if ((__CHARGSERV_DEBUG__)==1)
								_LIB_LOGGING_printf("#### CHARGSERV : chargstate : Stable -> Full(PWM9V) #### \r\n");
#endif
							}

						}
						else
						{
							if(0 == charger_get_wake_up_seq_flag())
							{
								if(0 == Charger.restart_fulldetect_act) //충전 재시작 및 완충 감지 기능 설정 시
								{
									charger_wake_up_seq_enable();
									charger_wake_up_seq_start(Charger.full_wake_up_seq_count + 1); //120초 2번
								}
							}
						}

						if(_TRUE == charger_wake_up_seq_is_finish())
						{
							_LIB_USERDELAY_start(&gTimeout_full_wake_up_seq_is_finish, DELAY_RENEW_OFF); //30초
							Charger.stable_changed_flag = 1;
#if ((__CHARGSERV_DEBUG__)==1)
							_LIB_LOGGING_printf("#### CHARGSERV : full_wake_up_seq_ is finish start #### \r\n");
#endif
						}

						if(_LIB_USERDELAY_isfired(&gTimeout_full_wake_up_seq_is_finish))
						{
							_LIB_USERDELAY_stop(&gTimeout_full_wake_up_seq_is_finish);

#if ((_DO_NOT_FAULT_LEAKAGE_)==1)
							//if((3 != Charger.leakage_instop_step) && (3 != Charger.oc_instop_step))
							if((3 != Charger.leakage_instop_step))	 //누설전류가 감지되지 않았을 경우						
							{
								Charger.full_wake_up_seq_count++;
							}
#else
							Charger.full_wake_up_seq_count++;
#endif
#if ((__CHARGSERV_DEBUG__)==1)
							_LIB_LOGGING_printf("#### CHARGSERV : chargstate : Stable_wake-up-squ(PWM9V) : %d #### \r\n",Charger.full_wake_up_seq_count);
#endif
						}
					break;

					case Charg_Full :
						_LIB_USERDELAY_start(&gTimeout_auto_finish, DELAY_RENEW_OFF); //10초

						if(_LIB_USERDELAY_isfired(&gTimeout_auto_finish))
						{
							_LIB_USERDELAY_stop(&gTimeout_auto_finish);

							Charger.forcestop_wake_up_seq_count = 0;
							Charger.full_wake_up_seq_count = 0;
							Charger.forcestop_wake_up_seq_flag = 0;
#if ((__CHARGSERV_DEBUG__)==1)
							_LIB_LOGGING_printf("#### CHARGSERV : auto finish #### \r\n");
#endif
							_APP_CHARGSERV_charging_ok();
						}
					break;
				}
			}

#if ((_FORCE_ACTIVING_MODE_)==0)
			//In '_FORCE_ACTIVING_MODE_' mode, the following functions must be deactivated to maintain 'Charging' state.
			if(cp_state == PWM_12V)
			{
				charger_wake_up_seq_forced_stop(); // pwm 12V -> DC 12V
				_LIB_USERDELAY_stop(&gTimeout_auto_finish);

				//if(_MW_CP_get_mc_relay_state() == _ON)

				if(Charg_Start >= Charger.chargstate) //chargstate = standby일 경우만 참
				{
#if ((__CHARGSERV_DEBUG__)==1)
						if(0 == Charger.reg.charging_compulsionunplug)
						{
							_LIB_LOGGING_printf("#### CHARGSERV : compulsion unplug #### \r\n");
						}
#endif
						//pwm 12V 상태인데 충전건을 뽑으면 mc_relay_off_fast 동작
						_APP_CHARGSERV_set_charging_to_unplug();//Charger.reg.charging_compulsionunplug = 1;
						if(Charger.reg.charging_continuous == 0)
						{
							//fast mc off
							_MW_CP_mc_relay_off_fast();

							_APP_CHARGSERV_charging_ok();
						}
				}
				else
				{
					//fast mc off
					_MW_CP_mc_relay_off_fast();

					_APP_CHARGSERV_charging_ok();
				}
			}
#endif
		break;

		case Finish :

			if(Charger.chargstate > Charg_Standby)//if(is_charging_sign == 1)
			{
				Charger.chargstate = Charg_Standby;//is_charging_sign = 0;
#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV : chargstate : Standby(finish) #### \r\n");
#endif
			}


			//220623 PES : PWM_6V also
			if((cp_state == PWM_12V) || (cp_state == PWM_9V) || (cp_state == PWM_6V))
			{
				_MW_CP_set_pwm_duty(100);
			}
			if(cp_state == DC_12V)
			{
				_APP_CHARGSERV_finish_ok();
				Charger.reg.charging_id_tagging_stop = 0;
			}
		break;

		case Fault :
			_MW_CP_set_pwm_duty(100);
		default :

		break;
	}

}
#endif

void charger_mc_ry_control()
{

	static uint8_t bak_state = 0;

	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();
	CP_StateDef	cp_state = _MW_CP_get_cp_state();
#if ((CP_N_VOLTAGE_CHECK)==1)
	double temp_l_final_volt = _MW_CP_get_l_final_voltage();
	double temp_H_final_volt = _MW_CP_get_h_final_voltage();
#endif


	switch(charger_state)
	{
		case Connect :

		break;

		case Charging :
			if(1 == Charger.leakage_instop_flag)
			{
				_MW_CP_mc_relay_ctl(_OFF);
				break;
			}

			if(cp_state == PWM_6V)
			{
#if ((CP_N_VOLTAGE_CHECK)==1)	//220726 PES : ZE_READY_'Diode presence'
				if(temp_l_final_volt > CP_N12V_MINIMUM)
				{
#if ((__CHARGSERV_DEBUG__)==1)
					if(bak_state != 1)
					{
						_LIB_LOGGING_printf("#### CHARGSERV : -12V is err : %d, %d #### \r\n", (int)(temp_l_final_volt * 100), (int)(temp_H_final_volt * 100));

						bak_state = 1;
					}
#endif
					_MW_CP_mc_relay_ctl(_OFF);
				}
				else
				{
#if ((__CHARGSERV_DEBUG__)==1)
					if(bak_state != 2)
					{
						_LIB_LOGGING_printf("#### CHARGSERV : -12V is good #### \r\n");
						bak_state = 2;
					}
#endif
					_MW_CP_mc_relay_ctl(_ON);
				}
#else
				_MW_CP_mc_relay_ctl(_ON);
#endif

			}
			if(cp_state == PWM_9V)
			{
#if ((__CHARGSERV_DEBUG__)==1)
				if(bak_state != 3)
				{
					_LIB_LOGGING_printf("#### CHARGSERV : PWM_9V #### \r\n");
					bak_state = 3;
				}
#endif
				_MW_CP_mc_relay_ctl(_OFF);
			}
		break;

		case Finish :
			//_MW_CP_mc_relay_ctl(_OFF);
			if(_ON == _MW_CP_get_mc_relay_state())
			{
				if(_TRUE == _LIB_USERDELAY_start(&gDelay_finish_mcoff, DELAY_RENEW_OFF))
				{
#if ((__CHARGSERV_DEBUG__)==1)
					_LIB_LOGGING_printf("#### CHARGSERV : finish_mc off delay start #### \r\n");
#endif
				}
			}

			//ZE-READY PWM38 TEST CASE
			if(cp_state == DC_9V)
			{
				_MW_CP_mc_relay_ctl(_OFF);
			}
		break;

		case Fault :
			_MW_CP_mc_relay_ctl(_OFF);
		default :

		break;
	}

	if(_LIB_USERDELAY_isfired(&gDelay_finish_mcoff))
	{
#if 1
		_LIB_USERDELAY_stop(&gDelay_finish_mcoff);
		_MW_CP_mc_relay_ctl(_OFF);
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : finish_mc off delay ok #### \r\n");
#endif

#else	//221228 PES MC off signal init
		if(_TRUE == _MW_CP_mc_relay_ctl(_OFF))
		{
			_LIB_USERDELAY_stop(&gDelay_finish_mcoff);
#if ((__CHARGSERV_DEBUG__)==1)
			_LIB_LOGGING_printf("#### CHARGSERV : finish_mc off delay ok #### \r\n");
#endif
		}
#endif
	}

	_MW_CP_mc_relay_clear();

}

void charger_wake_up_seq_control()
{
	if(charger_get_wake_up_seq_flag() == 0)
	{
		return;
	}

	switch(charger_get_wake_up_seq_step())
	{
		case START_TIMEOUT :
/*				StartDelay(&gDelay_wake_up_seq_start_timeout[i], DELAY_RENEW_OFF);
			if(isFired(&gDelay_wake_up_seq_start_timeout[i]))
			{
				StopDelay(&gDelay_wake_up_seq_start_timeout[i]);

				ctrl_ry(_OFF, i);
				multi_ch_model[i].pwm_duty = PWM_DUTY_0A;

				multi_ch_model[i].wake_up_seq_step = CP_OFF;
			}*/
#if 0
			if(_TRUE == charger_wake_up_seq_starttimer(T3wusqtimeoutHandle,30000))
			{
				_LIB_LOGGING_printf("wuseq timeout\r\n");
			}
#endif
			if(_TRUE == _LIB_USERDELAY_start(&gDelay_wusqtimeout, DELAY_RENEW_OFF))
			{
				_LIB_LOGGING_printf("wuseq timeout\r\n");
			}
			if(_TRUE == _LIB_USERDELAY_isfired(&gDelay_wusqtimeout))
			{
				_LIB_USERDELAY_stop(&gDelay_wusqtimeout);
				_APP_CHARGSERV_set_wake_up_seq_step(CP_OFF);
			}

		break;
		case CP_OFF :
/*				StartDelay(&gDelay_wake_up_seq_cp_off[i], DELAY_RENEW_OFF);

			ctrl_ry(_OFF, i);
			multi_ch_model[i].pwm_duty = PWM_DUTY_0A;

			if(isFired(&gDelay_wake_up_seq_cp_off[i]))
			{
				StopDelay(&gDelay_wake_up_seq_cp_off[i]);

				ctrl_ry(_ON, i);

				multi_ch_model[i].wake_up_seq_step = PWM_DISABLE;
			}*/
#if 0
			if(_TRUE == charger_wake_up_seq_starttimer(T4wusqcpoffHandle,3000))
			{
				_LIB_LOGGING_printf("wuseq cp off\r\n");
			}
#endif
			if(_TRUE == _LIB_USERDELAY_start(&gDelay_wusqcpoff, DELAY_RENEW_OFF))
			{
				_LIB_LOGGING_printf("wuseq cp off\r\n");
			}
			if(_TRUE == _LIB_USERDELAY_isfired(&gDelay_wusqcpoff))
			{
				_LIB_USERDELAY_stop(&gDelay_wusqcpoff);
				_APP_CHARGSERV_set_wake_up_seq_step(PWM_DISABLE);
			}
		break;
		case PWM_DISABLE :
/*				StartDelay(&gDelay_wake_up_seq_pwm_off[i], DELAY_RENEW_OFF);

			ctrl_ry(_ON, i);
			multi_ch_model[i].pwm_duty = PWM_DUTY_0A;

			if(isFired(&gDelay_wake_up_seq_pwm_off[i]))
			{
				StopDelay(&gDelay_wake_up_seq_pwm_off[i]);

				//_LOGGING_Printf("%d ch, wake up seq finish\r\n", i);
				multi_ch_model[i].wake_up_seq_step = FINISH;
				_CTRL_adjust_adc_vol_init(i);
			}*/
#if 0
			if(_TRUE == charger_wake_up_seq_starttimer(T5wusqpwmoffHandle,800))
			{
				_LIB_LOGGING_printf("wuseq pwm off\r\n");
			}
#endif
			if(_TRUE == _LIB_USERDELAY_start(&gDelay_wusqpwmoff, DELAY_RENEW_OFF))
			{
				_LIB_LOGGING_printf("wuseq pwm off\r\n");
			}
			if(_TRUE == _LIB_USERDELAY_isfired(&gDelay_wusqpwmoff))
			{
				_LIB_USERDELAY_stop(&gDelay_wusqpwmoff);
				_APP_CHARGSERV_set_wake_up_seq_step(FINISH);
			}
		break;
		case FINISH :
/*				StartDelay(&gDelay_wake_up_seq_finish_ok[i], DELAY_RENEW_OFF);

			if(isFired(&gDelay_wake_up_seq_finish_ok[i]))
			{
				StopDelay(&gDelay_wake_up_seq_finish_ok[i]);

				_CTRL_wake_up_seq_disable(i);
			}*/
			_LIB_LOGGING_printf("wuseq finish\r\n");
			Charger.wake_up_seq_finish_flag = 1;
			charger_wake_up_seq_disable();
		break;

		default :

		break;
	}
}

#if 0
void charger_power_state_control_flug()
{
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();
	static eCharger_State bak_charger_state = Chargerstateend;
	static uint8_t state_changed = _OFF;

	switch(charger_state)
	{
		case Ready :
#if ((_NON_USERCEHCK_) == 0)
			if((_TRUE == charger_is_overpower()) && (_ON == _MW_CP_get_mc_relay_state()))
			{
				printf("%lu\r\n", Charger.current_Active_Power);
				_APP_CHARGSERV_ready_powercutstart();
			}
#endif
		break;

		case UserCheck :

		break;

		case Connect :
			if(_ON == state_changed)
			{
				//calculate use active Energy start
				charger_set_startpoint_energy(Charger.current_Active_Energy);
			}
			if(_TRUE == charger_is_overpower())
			{
				_APP_CHARGSERV_connect_ok();
			}

		break;

		case Charging :
			charger_calc_use_energy();
			charger_calc_won(250);
			//Todo
			//charger_update_use_energy(); 	// 1). update use energy to CSMS APP TASK
			//charger_update_won();			// 2). update won from CSMS APP TASK
			if(_TRUE == charger_is_underpower())
			{
				_APP_CHARGSERV_charging_ok();  // after, must to change fault processing
				//Todo
				//calculate use active Energy end
			}

		break;

		case Finish :
			if(_FALSE == charger_is_load_detect_flug())
			{
				_APP_CHARGSERV_finish_ok();
				Charger.reg.charging_id_tagging_stop = 0;
			}
		break;

		case Fault :

		default :

		break;
	}

	if(bak_charger_state != charger_state)
	{
		bak_charger_state = charger_state;
		state_changed = _ON;
	}
	else
	{
		state_changed = _OFF;
	}
}
#endif

#if 0
#if 0
void charger_mc_ry_control_flug()
{
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();

	switch(charger_state)
	{
		case Powercut :
		case Fault :
		case UserCheck :
			_MW_CP_mc_relay_ctl(_OFF);
		break;

		default :
			_MW_CP_mc_relay_ctl(_ON);
		break;
	}
}
#else
void charger_mc_ry_control_flug()
{
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();

	switch(charger_state)
	{
		case Init :
			_MW_CP_mc_relay_ctl(_OFF);
		break;
		case Ready :
			_MW_CP_mc_relay_ctl(_OFF);
		break;
		case UserCheck :
			_MW_CP_mc_relay_ctl(_OFF);
		break;
		case Connect :
			_MW_CP_mc_relay_ctl(_ON);
		break;
		case Charging :
			_MW_CP_mc_relay_ctl(_ON);
		break;
		case Finish :
			_MW_CP_mc_relay_ctl(_OFF);
		break;
		case OTA :
			_MW_CP_mc_relay_ctl(_OFF);
		break;
		case Fault :
			_MW_CP_mc_relay_ctl(_OFF);
		break;
		case Powercut :
			_MW_CP_mc_relay_ctl(_OFF);
		break;
		default :

		break;
	}
}
#endif
#endif

void charger_common_control()
{
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();
	CP_StateDef	cp_state = _MW_CP_get_cp_state();

	uint8_t ret_value = 0;
	static uint8_t step = 0;

	uint8_t	button_value = 0;
	uint8_t button_pushed = 0;

	static uint8_t emg_value = 0;
	static uint8_t emg_value_bak = 0;
	static uint8_t emg_value_count = 0;


	switch(charger_state)
	{
		case Ready :

			if(_OFF == _MW_GPIO_get_gpi(EMG))
			{
				emg_value = 1;
			}
			else
			{
				emg_value = 0;
			}

			if(emg_value_bak != emg_value)
			{
				emg_value_bak = emg_value;

				switch(emg_value)
				{
					case 0 :
						_APP_RFID_set_start_tagging(0);
					break;
					case 1 :
						_APP_RFID_set_start_tagging(1);
					break;
				}
			}


#if ((_AUTOCHECKUSER_) == 0)
			if(0 == Charger.autostartmode_flag)
			{
				ret_value = _APP_RFID_get_tagging_result();
				if(_CONTINUE != ret_value)
				{
					_APP_CHARGSERV_ready_cardtag();
				}
			}
			else
			{
				if((DC_9V == cp_state) || (DC_6V == cp_state))
				{
					_APP_CHARGSERV_ready_cardtag();
				}
			}
#else

#endif
		break;
		case AutoReady :
			if((DC_9V == cp_state) || (DC_6V == cp_state))
			{
				_APP_CHARGSERV_autoready_connected(); //autoready -> connect
			}
		break;
		case UserCheck :
#if ((_AUTOCHECKUSER_) == 0)
			if(0 == Charger.autostartmode_flag)
			{
				switch(step)
				{
					case 0 :
#if 1
						ret_value = _APP_RFID_get_tagging_result();
						if(_TRUE == ret_value)
						{
							_APP_RFID_get_card_number(Charger.current_card_num);
							_APP_CHARGSERV_usercheck_serverconnectstart();
							step = 1;
						}
						else
						{
							_APP_CHARGSERV_usercheck_serverconnectstart();
							_APP_CHARGSERV_usercheck_user_retry();
							step = 0;
						}
#else
						ret_value = _APP_RFID_get_tagging_result();
						if(_TRUE == ret_value)
						{
							_APP_RFID_get_card_number(Charger.current_card_num);
						}
						_APP_CHARGSERV_usercheck_serverconnectstart();
						step = 1;
#endif
					break;
					case 1 :
						if(_TRUE == _APP_SYSTEMCTL_check_cardnum(Charger.current_card_num))//TODO : 230109 PES : check eeprom cardnumber data == tagging cardnumber
						{
							memcpy(Charger.charging_card_num,Charger.current_card_num,30);

							_APP_CHARGSERV_usercheck_user_ok();
							step = 0;
						}
						else
						{
							_APP_CHARGSERV_usercheck_user_retry();
							step = 0;
						}
					break;
				}
			}
			else
			{
				if((DC_9V == cp_state) || (DC_6V == cp_state))
				{
					_APP_CHARGSERV_usercheck_serverconnectstart();
					_APP_CHARGSERV_usercheck_user_ok();
				}
			}

#else

#endif
		break;

		case Connect :
#if ((_CONNECT_TIMEOUT_) == 1)
			switch(step)
			{
				case 0 :
					_LIB_USERDELAY_stop(&gTimeout_connect);
					if(1 == _LIB_USERDELAY_start(&gTimeout_connect, DELAY_RENEW_OFF))
					{
#if ((__CHARGSERV_DEBUG__)==1)
						_LIB_LOGGING_printf("#### CHARGSERV : Connect_Timeout start #### \r\n");
#endif
						step = 1;
					}
					else
					{
#if ((__CHARGSERV_DEBUG__)==1)
						_LIB_LOGGING_printf("#### CHARGSERV : Connect_Timeout start fail #### \r\n");
#endif
					}
				break;

				case 1 :
					if(1 == _LIB_USERDELAY_isfired(&gTimeout_connect))
					{
#if ((__CHARGSERV_DEBUG__)==1)
						_LIB_LOGGING_printf("#### CHARGSERV : Connect_Timeout... go to ready #### \r\n");
#endif
						_LIB_USERDELAY_stop(&gTimeout_connect);
						_APP_CHARGSERV_connect_timeout();
						step = 0;
					}
				break;
			}
#endif
		break;

		case Charging :
#if 1
			ret_value = _APP_RFID_get_tagging_result();
#if 0
			if(_TRUE == ret_value)
			{
#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV : tagging Stop squence #### \r\n");
#endif
				char temp_card_num[30] = {'\0',};
				_APP_RFID_get_card_number(temp_card_num);

				if(!strcmp(temp_card_num,Charger.current_card_num))
				{
#if ((__CHARGSERV_DEBUG__)==1)
					_LIB_LOGGING_printf("#### CHARGSERV : tagging card num is same. go to finish #### \r\n");
#endif
					Charger.reg.charging_id_tagging_stop = 1;
					_APP_CHARGSERV_charging_ok();
				}
				else
				{
#if ((__CHARGSERV_DEBUG__)==1)
					_LIB_LOGGING_printf("#### CHARGSERV : tagging card num is diff. retry tagging #### \r\n");
#endif
					_APP_RFID_set_start_tagging(0);
				}

			}
			else if(_FALSE == ret_value)
			{
				_LIB_LOGGING_printf("#### CHARGSERV : tagging error #### \r\n");
			}
#else
			if(_TRUE == ret_value)
			{
#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV : tagging state to charging #### \r\n");
#endif
				_APP_RFID_get_card_number(Charger.current_card_num);
#if 0
				if(1 == Charger.reg.fault_automodeflag)
				{
#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV : automode_tag... charging ok #### \r\n");
#endif
					_APP_CHARGSERV_charging_ok();
				}
#endif

				if(_TRUE == _APP_CHARGSERV_check_charging_cardnum())//TODO : 230109 PES : check start cardnumber data == tagging cardnumber
				{
#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV : charging cardnum == tagging cardnum. charging ok #### \r\n");
#endif
					_APP_CHARGSERV_charging_ok();
				}
				else
				{
#if ((__CHARGSERV_DEBUG__)==1)
					_LIB_LOGGING_printf("#### CHARGSERV : charging cardnum != tagging cardnum. retry tagging #### \r\n");
#endif
					_APP_RFID_set_start_tagging(0);
				}
			}

			if(0)
			{
				_APP_CHARGSERV_charging_ok();
				Charger.reg.charging_id_tagging_stop = 1;
			}
			else if(0)
			{
				_APP_RFID_set_start_tagging(0);
			}
#if 0
			if((1 == Charger.reg.charging_remote_stop) && (1 == Charger.reg.charging_remote_stop_check))
			{
				Charger.reg.charging_remote_stop = 0;
				Charger.reg.charging_remote_stop_check = 0;
#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV : remote_stop #### \r\n");
				_MW_CP_set_pwm_duty(100);
#endif
				_APP_CHARGSERV_charging_ok();
			}
#endif
#endif
#else
			//check button value
			button_value = _MW_GPIO_get_gpi(EMG);
			if((_OFF == button_value))
			{
				button_pushed = 1;
			}


			if(1 == button_pushed)
			{
				if(1 == _LIB_USERDELAY_start(&gDelay_charging_ok_button_input, DELAY_RENEW_OFF))
				{
#if ((__CHARGSERV_DEBUG__)==1)
					_LIB_LOGGING_printf("#### CHARGSERV : charging_ok_button push start #### \r\n");
#endif
				}
				if(1 == _LIB_USERDELAY_isfired(&gDelay_charging_ok_button_input))
				{
					_LIB_USERDELAY_stop(&gDelay_charging_ok_button_input);

					_APP_CHARGSERV_charging_ok();
				}
			}
			else
			{
				_LIB_USERDELAY_stop(&gDelay_charging_ok_button_input);
			}


#endif
		break;

		case Finish :
			_APP_RFID_set_start_tagging(1);
		break;

		case Fault :

		break;

		default :

		break;
	}

}

uint8_t _APP_CHARGSERV_check_charging_cardnum()
{
	uint8_t ret_value = _FALSE;

	int i = 0;

	for(i = 0; i<8;i++)
	{
		if(Charger.current_card_num[i] != Charger.charging_card_num[i])
		{
			break;
		}
	}

	if(8 == i)
	{
		ret_value = _TRUE;
		memset(Charger.charging_card_num,0xFF,30);
	}

	return ret_value;
}

#if 0

/*
 *
 * PT+Peak Detecter (15.2K)
 *
 * 242VAC = 2.6V
 * 220VAC = 2.36V
 * 198VAC = 2.12V
 * dvac = 0.01091
 * dvac_100x = 0.0001091
 * Peak_Vf = 0.65V
 * adc_max_value = 2^12 = 4096
 * Vadc_ref = 3.3V
 *
 * VRMS = ((input adc value / adc_max_value) * Vadc_ref) / dvac
 *
 * VRMS(+Peak Vf) = ((input adc value / adc_max_value) * Vadc_ref + Peak_Vf) / dvac
 *
 */

#if 1
void _APP_CHARGSERV_check_Vrms_loop()
{
	uint16_t temp = gADCData[ADC_AC_V_INDEX_];

	static uint16_t dtemp = 0;
	static uint32_t adc_temp[300] = {0,};
	static uint16_t adc_temp_index = 0;
	uint32_t adc_temp_upper = 0;

	double vrms_adc_value = (double)temp;

#if	((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
	uint32_t adc_temp_lpf = 0;
#endif

#if 0
	double vrms_adc_input_voltage = ((vrms_adc_value / 4096.0F) * (3.3F));
#else
	double vrms_adc_input_voltage = (((vrms_adc_value / 4096.0F) * (3.3F)) + (1.25F));
#endif

	double vrms_voltage = ((vrms_adc_input_voltage * 10000000.0F) / 1091.0F);

	adc_temp[adc_temp_index++] = (uint32_t)vrms_voltage;
	if(adc_temp_index >= 300)	adc_temp_index = 0;


	for(int i = 0; i<300; i++)
	{
		if(adc_temp[i] > adc_temp_upper)	adc_temp_upper = adc_temp[i];
	}

#if	((_VRMS_IRMS_CALC_LPF_FILTER_) == 0)
	Charger.current_V_rms = adc_temp_upper;
#else

	adc_temp_lpf = _LIB_LPF_calc(&Vrms_calc, adc_temp_upper);

	Charger.current_V_rms = adc_temp_lpf;
#endif

#if 1
	dtemp++;

	if(dtemp > 1000)
	{
		dtemp = 0;
		printf("VRMS : %ld \r\n", Charger.current_V_rms);
		printf("zct : %ld \r\n", gADCData[ADC_ZCT_INDEX_]);
	}
#endif

}
#endif
/*
 * CT ratio : 2500:1  (50A:20mA)
 *
 * CT_burden_r = 10R
 *
 * CT_input_v_max = 0.25V
 *
 * absolute value circuit ratio : 5x
 *
 * CT_mcu_input_v_max = 1.25V
 *
 * ARMS = (50/1.25) * (3.3/4096) * input_adc_value
 * 		= 0.02418
 *
 */
#if 1
void _APP_CHARGSERV_check_Irms_loop()
{
	uint16_t temp = gADCData[ADC_AC_A_INDEX_];

	static uint16_t dtemp = 0;
	static uint32_t adc_temp[500] = {0,};
	static uint16_t adc_temp_index = 0;
	uint32_t adc_temp_upper = 0;

	double Irms_adc_value = (double)temp;
#if ((CP_N_VOLTAGE_CHECK)==1)
	double temp_l_final_volt = _MW_CP_get_l_final_voltage();
	double temp_H_final_volt = _MW_CP_get_h_final_voltage();
#endif

#if	((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
	uint32_t adc_temp_lpf = 0;
#endif

	//adc_temp[adc_temp_index++] = (uint32_t)(Irms_adc_value * 30.22);
	adc_temp[adc_temp_index++] = (uint32_t)(Irms_adc_value * 29.22);
	//adc_temp[adc_temp_index++] = (uint32_t)(Irms_adc_value * 24.18);
	//adc_temp[adc_temp_index++] = (uint32_t)(Irms_adc_value * 23.4);

	if(adc_temp_index >= 500)	adc_temp_index = 0;


	for(int i = 0; i<500; i++)
	{
		if(adc_temp[i] > adc_temp_upper)	adc_temp_upper = adc_temp[i];
	}

#if	((_VRMS_IRMS_CALC_LPF_FILTER_) == 0)
	Charger.current_I_rms = adc_temp_upper;
#else

	adc_temp_lpf = _LIB_LPF_calc(&Irms_calc, adc_temp_upper);

	Charger.current_I_rms = adc_temp_lpf;
#endif

#if 1
	dtemp++;

	if(dtemp > 200)
	{
		dtemp = 0;
		printf("IRMS : %ld \r\n", Charger.current_I_rms);
		printf("CP_H : %d \r\n", (int)(temp_H_final_volt * 100));
		printf("CP_L : %d \r\n", (int)(temp_l_final_volt * 100));

	}
#endif
}
#else
void _APP_CHARGSERV_check_Irms_loop()
{
	uint16_t temp = gADCData[ADC_AC_A_INDEX_];

	static uint16_t dtemp = 0;
	static uint16_t adc_temp[500] = {0,};
	static uint16_t adc_temp_index = 0;
	uint16_t adc_temp_max = 0;
	uint16_t adc_temp_min = 0;
	uint32_t adc_temp_avr = 0;

	double Irms_adc_value = (double)temp;

	//adc_temp[adc_temp_index++] = (uint32_t)(Irms_adc_value * 30.22);
	adc_temp[adc_temp_index++] = (uint16_t)(Irms_adc_value * 24.18);
	if(adc_temp_index >= 500)
	{
		adc_temp_index = 0;

		int i = 0;

		adc_temp_max = 0;
		adc_temp_min = adc_temp[0];
		adc_temp_avr = 0;

		for(int i = 0; i<500; i++)
		{
			if(adc_temp[i] > adc_temp_max)	adc_temp_max = adc_temp[i];
			if(adc_temp[i] < adc_temp_min)	adc_temp_min = adc_temp[i];

		}

		adc_temp_avr = ((((uint32_t)adc_temp_max) + ((uint32_t)adc_temp_min)) / 2);

		Charger.current_I_rms = adc_temp_avr;

#if 1
		//dtemp++;

		//if(dtemp > 200)
		//{
		//	dtemp = 0;
			printf("IRMS : %ld \r\n", Charger.current_I_rms);
		//}
#endif
	}
}
#endif

#endif
void charger_userconfigmode_handler()
{
	//230407 PES
	// 비상정지버튼 누름 감지(상승펄스)를 각 각 1초 이내로 5번 연속 감지했을 경우, userconfigmode로 진입한다.

	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();
	static uint8_t bak_button_value = 0;
	uint8_t	button_value = 0;
	uint8_t button_pushed = 0;


	//check button value
	button_value = _MW_GPIO_get_gpi(EMG);
	if((_OFF == button_value) && (_ON == bak_button_value))
	{
		button_pushed = 1;
	}
	bak_button_value = button_value;

	if(0 == button_pushed)		return;

	//first try is alway count++
	if(0 == Charger.userconfigmode_input_count)
	{
		Charger.userconfigmode_input_count++;
		_LIB_USERDELAY_start(&gTimeout_userconfigmode_input, DELAY_RENEW_ON);
	}
	else
	{
		//is xsec under?
		if(0 == _LIB_USERDELAY_isfired(&gTimeout_userconfigmode_input))
		{
			Charger.userconfigmode_input_count++;
			_LIB_USERDELAY_start(&gTimeout_userconfigmode_input, DELAY_RENEW_ON);
		}
		else
		{
			_LIB_USERDELAY_stop(&gTimeout_userconfigmode_input);
			Charger.userconfigmode_input_count = 0;
			printf("userconfigmode_handler_timeout\r\n");
		}
	}

	printf("userconfigmode_handler_button_pushed : %d\r\n",Charger.userconfigmode_input_count);

	//go to configmode
	if(Charger.userconfigmode_input_count >= CHARGSERV_USERCONFIGMODE_INPUT_MAX_COUNT)
	{
		Charger.userconfigmode_input_count = 0;
		if((Ready == charger_state))
		{
			printf("userconfigmode_handler_go to configmode\r\n");


			charger_set_mode(mode_config);
			_APP_CONFIGSERV_flag(_TRUE,_TRUE);
		}
		else
		{
			printf("userconfigmode_handler_go to configmode _ NO EFFECT \r\n");
		}

	}
}

#define EMG_INPUT_BIT 0
void charger_autostartmode_handler()
{
	//230531 PES
	//Manual start mode에서 비상정지버튼을 PUSH상태로 10초 이상 유지할 경우, Auto start mode으로 진입한다.
	//Auto start mode에서 비상정지버튼을 PUSH상태로 10초 이상 유지할 경우, Manual start mode으로 진입한다.

	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();
	uint8_t	button_value = 0;
	uint8_t button_pushed = 0;

	static uint8_t handler_first_loop = 0;

	if(1 == Charger.default_start_mode)
	{
		//is first_loop?
		if(0 == handler_first_loop)
		{

			if(1 == _LIB_USERDELAY_start(&gDelay_autostartmode_default_set, DELAY_RENEW_OFF))
			{
				printf("_DEFAULT_AUTOSTART_ACTIVE_ delay start\r\n");
			}

			if(1 == _LIB_USERDELAY_isfired(&gDelay_autostartmode_default_set))
			{


				if(1 == _APP_CHARGSERV_ready_automodeflag())
				{
					_LIB_USERDELAY_stop(&gDelay_autostartmode_default_set);
					handler_first_loop = 1;

					Charger.automode_active = 1;
					printf("_DEFAULT_AUTOSTART_ACTIVE_ __ Manualstartmode --> Autostartmode \r\n");
				}

			}

		}
	}

	//check button value
	button_value = _MW_GPIO_get_gpi(EMG);
	if((_OFF == button_value))
	{
		button_pushed = 1;
	}

	if(charger_state == Charging || charger_state == Fault)
	{
		if(button_pushed == 1)
		{
			charger_emg_fault();
			charger_fault_status.Raw |= (1 << EMG_INPUT_BIT);
			_MW_INDILED_sled_ctl(RED);



		}
		else
		{
			charger_emg_fault();
			charger_fault_status.Raw &= ~(1 << EMG_INPUT_BIT);



		}
	}

	if(1 == button_pushed)
	{
		if(1 == _LIB_USERDELAY_start(&gDelay_autostartmode_input, DELAY_RENEW_OFF))
		{
			printf("Autostartmode_handler_button_pushed...\r\n");
		}
		if(1 == _LIB_USERDELAY_isfired(&gDelay_autostartmode_input))
		{
			_LIB_USERDELAY_stop(&gDelay_autostartmode_input);

			if((Ready == charger_state) || (AutoReady == charger_state))
			{
				printf("Autostartmode_handler_pushed on delay timer pass! \r\n");

				if(1 == _APP_CHARGSERV_ready_automodeflag())
				{
					printf("Manualstartmode --> Autostartmode \r\n");
					Charger.automode_active = 1;
				}
				else if(1 == _APP_CHARGSERV_autoready_manualmodeflag())
				{
					printf("Autostartmode --> Manualstartmode \r\n");
					Charger.automode_active = 0;
				}
				else
				{
					printf("Autostartmode --> Manualstartmode No Effect \r\n");
				}
			}
		}
	}

	else
	{
		_LIB_USERDELAY_stop(&gDelay_autostartmode_input);
	}
}

#if 0
void _APP_CHARGSERV_ctrl_loop()
{
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();
	CP_StateDef	cp_state = _MW_CP_get_cp_state();

	switch(charger_state)
	{
		case Connect :
			_MW_CP_cp_relay_ctl(_ON);

			if((cp_state == DC_9V) || (cp_state == DC_6V))
			{
				_APP_CHARGSERV_set_duty_ondelay(CHARGSERV_DUTY_ONDELAY);

				if(_TRUE == _APP_CHARGSERV_complete_duty_ondelay())
				{
					_MW_CP_set_pwm_duty(_MW_CP_cal_ampe_to_duty(15));
					_APP_CHARGSERV_connect_ok();
				}
			}
		break;

		case Charging :
			if(cp_state == PWM_6V)
			{
				_MW_CP_mc_relay_ctl(_ON);
			}

			if(cp_state == PWM_9V)
			{
				_MW_CP_mc_relay_ctl(_OFF);
			}

			//if((_TRUE == _MW_CP_get_mc_relay_state()) && ((cp_state == PWM_9V) || (cp_state == PWM_12V)))
			if((_TRUE == _MW_CP_get_mc_relay_state()) && ((cp_state == PWM_12V)))
			{
				_APP_CHARGSERV_charging_ok();
			}
		break;

		case Finish :
			_MW_CP_mc_relay_ctl(_OFF);

			if(cp_state == PWM_12V)
			{
				_MW_CP_set_pwm_duty(100);
			}
			else if(cp_state == DC_12V)
			{
				_MW_CP_cp_relay_ctl(_OFF);
				_APP_CHARGSERV_finish_ok();
			}

		break;

		case Fault :

		default :

		break;
	}

}
#else
void _APP_CHARGSERV_ctrl_loop()
{
	charger_wake_up_seq_control();
	charger_cp_ry_control();
	charger_cp_state_control();
	charger_mc_ry_control();

	charger_common_control();

	charger_userconfigmode_handler();
	charger_autostartmode_handler();
}
#endif

uint8_t _APP_CHARGSERV_print_state()
{
	eCharger_State state = _APP_CHARGSERV_get_current_state();

	if(state != Charger.state_bk)
	{
		switch(state)
		{
			case Init :
				_LIB_LOGGING_printf("#### CHARGSERV State : Init #### \r\n");
			break;

			case Ready :
				_LIB_LOGGING_printf("#### CHARGSERV State : Ready #### \r\n");
			break;

			case AutoReady :
				_LIB_LOGGING_printf("#### CHARGSERV State : AutoReady #### \r\n");
			break;

			case UserCheck :
				_LIB_LOGGING_printf("#### CHARGSERV State : UserCheck #### \r\n");
			break;
			case Connect :
				_LIB_LOGGING_printf("#### CHARGSERV State : Connect #### \r\n");
			break;

			case Charging :
				_LIB_LOGGING_printf("#### CHARGSERV State : Charging #### \r\n");
			break;

			case Finish :
				_LIB_LOGGING_printf("#### CHARGSERV State : Finish #### \r\n");
			break;

			case OTA :
				_LIB_LOGGING_printf("#### CHARGSERV State : OTA #### \r\n");
			break;

			case Fault :
				_LIB_LOGGING_printf("#### CHARGSERV State : Fault #### \r\n");
			break;

			case Powercut :
				_LIB_LOGGING_printf("#### CHARGSERV State : Powercut #### \r\n");
			break;

			default :
				_LIB_LOGGING_printf("#### CHARGSERV State : default #### \r\n");
			break;
		}

		Charger.state_bk = state;
	}
	return _TRUE;
}

/*
 * ------------------------------chargserv fault---------------------------------
 */

void _APP_CHARGSERV_device_stop(uint8_t mode)
{
	if(mode == 1)
	{
		_MW_CP_mc_relay_off_fast();
		_MW_CP_mc_relay_ctl(_OFF);
		_MW_CP_set_pwm_duty(100);
	}
	else
	{
		_MW_CP_cp_relay_ctl(_OFF);
		_MW_CP_mc_relay_ctl(_OFF);
		_MW_CP_set_pwm_duty(100);
	}
}

#if 0
uint8_t _APP_CHARGSERV_set_active_Ampe(uint8_t mode)
{
	uint8_t ret_value = _FALSE;
	static uint8_t mode_bak = 0xff;


	if(mode_bak != mode)
	{
		if((mode > 0) && (CHARGSERV_AMPE_CONVERSION_STEP >= mode))
		{
			Charger.active_Ampe = CHARGSERV_MAXIMUM_AMPE - (1 * mode);
			ret_value = _TRUE;
		}
		else if(0 == mode)
		{
			Charger.active_Ampe = CHARGSERV_MAXIMUM_AMPE;
			ret_value = _TRUE;
		}
	}

	return ret_value;
}
#else
uint8_t _APP_CHARGSERV_set_active_Ampe(uint8_t step) //기준 전류값 설정
{
	uint8_t ret_value = _FALSE; //step = 2
	static uint8_t ampe_bak = 0xff;
	eCharger_Mode temp_mode;
	uint8_t max_ampe;
	float step_constant;

	temp_mode = charger_get_mode();

	switch(temp_mode) //충전 모드를 가지고 와서 
	{
		case mode_3KW :
			max_ampe = CHARGSERV_MAXIMUM_AMPE_3KW; //3kW 일 때 최대 전류 13
		break;
		case mode_5KW :
			max_ampe = CHARGSERV_MAXIMUM_AMPE_5KW; //5kW 일 때 최대 전류 22
		break;
		case mode_7KW :
			max_ampe = CHARGSERV_MAXIMUM_AMPE_7KW; //7kW 일 때 최대 전류 32
		break;
		case mode_11KW :
			max_ampe = CHARGSERV_MAXIMUM_AMPE_11KW; //11kW 일 때 최대 전류 50
		break;
		default :
			max_ampe = CHARGSERV_MAXIMUM_AMPE_11KW; //기본값 최대 전류 50
		break;
	}

	switch(step)
	{
		case 1 :
			step_constant = 0.9;
		break;
		case 2 :
			step_constant = 0.8; //step이 2니까 0.8
		break;
		default :
			step_constant = 1.0;
		break;
	}

	Charger.active_Ampe = (uint8_t)(max_ampe * step_constant);
	//활성 전류 = step 상수 * 모드 별 최대 전류

	if(ampe_bak != Charger.active_Ampe) //활성 전류가 이전 값과 다르면 
	{
#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV_set_active_Ampe :%d #### \r\n",Charger.active_Ampe);
#endif
		ampe_bak = Charger.active_Ampe;
		ret_value = _TRUE;
	}

	return ret_value;
}
#endif
uint8_t _APP_CHARGSERV_get_active_Ampe()
{
	return Charger.active_Ampe;
}

unsigned long _APP_CHARGSERV_get_voltage_rms_V()
{
	unsigned long temp = 0;

	temp = (Charger.current_V_rms / 100); //mv -> v단위로 변환

	return temp;
}

void _APP_CHARGSERV_set_voltage_rms_V(uint32_t Vrms)
{
	Charger.current_V_rms = Vrms;
}
unsigned long _APP_CHARGSERV_get_current_rms_A()
{
	unsigned long temp = 0;

	if(_TRUE == charger_is_load_detect_evse()) //충전기의 부하 감지 되었으면 rms 전류값 가져오기
	{
		temp = (Charger.current_I_rms / 1000); //mA -> A로 변환
	}

	return temp;
}

void _APP_CHARGSERV_set_current_rms_A(uint32_t Irms)
{
	Charger.current_I_rms = Irms;
}

void charger_emg_fault()
{
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();

	if((AutoReady == charger_state) || (1 == Charger.automode_active)| (0 == Charger.automode_active))
	{
		charger_fault_status.EMG_INPUT = _OFF;
		return;
	}

	if((Charging == charger_state) || (Fault == charger_state))
	{
		if(_OFF == _MW_GPIO_get_gpi(EMG))
		{
			if(_OFF == charger_fault_status.EMG_INPUT)
			{
	#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV_FAULT_SET : EMG_INPUT #### \r\n");
	#endif
			}

			charger_fault_status.EMG_INPUT = _ON;


		}
		else if((_ON == charger_fault_status.EMG_INPUT) && (_ON == _MW_GPIO_get_gpi(EMG)))
		{
			charger_fault_status.EMG_INPUT = _OFF;

	#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV_FAULT_CLR : EMG_INPUT #### \r\n");
	#endif
		}
	}

}

void charger_wd_fault()
{
	if((_ON == _MW_CP_get_mc_relay_state()) && (_OFF == _MW_GPIO_get_gpi(WELD)))
	{
		if(_TRUE == _LIB_USERDELAY_start(&gTimeout_wd_fault, DELAY_RENEW_OFF))
		{

		}
		if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_wd_fault))
		{
			_LIB_USERDELAY_stop(&gTimeout_wd_fault);
			charger_fault_status.MC_START_ERR = _ON;
#if ((__CHARGSERV_DEBUG__)==1)
			_LIB_LOGGING_printf("#### CHARGSERV_FAULT_SET : MC_START_ERR #### \r\n");
#endif
		}
	}
	else if((_OFF == _MW_CP_get_mc_relay_state()) && (_ON == _MW_GPIO_get_gpi(WELD)))
	{
		if(_TRUE == _LIB_USERDELAY_start(&gTimeout_wd_fault, DELAY_RENEW_OFF))
		{

		}
		if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_wd_fault))
		{
			_LIB_USERDELAY_stop(&gTimeout_wd_fault);
			charger_fault_status.MC_STOP_ERR = _ON;
#if ((__CHARGSERV_DEBUG__)==1)
			_LIB_LOGGING_printf("#### CHARGSERV_FAULT_SET : MC_STOP_ERR #### \r\n");
#endif
		}
	}
	else if((charger_fault_status.MC_START_ERR == _ON)
			&& (_MW_CP_get_mc_relay_state() == _MW_GPIO_get_gpi(WELD))
			&& (_MW_CP_get_cp_state() == DC_12V))
	{
		if(_TRUE == _LIB_USERDELAY_start(&gTimeout_wd_fault, DELAY_RENEW_OFF))
		{

		}
		if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_wd_fault))
		{
			_LIB_USERDELAY_stop(&gTimeout_wd_fault);
			charger_fault_status.MC_START_ERR = _OFF;
#if ((__CHARGSERV_DEBUG__)==1)
			_LIB_LOGGING_printf("#### CHARGSERV_FAULT_CLR : MC_START_ERR #### \r\n");
#endif
		}
	}
	else if((charger_fault_status.MC_STOP_ERR == _ON)
			&& (_MW_CP_get_mc_relay_state() == _MW_GPIO_get_gpi(WELD)))
	{
		if(_TRUE == _LIB_USERDELAY_start(&gTimeout_wd_fault, DELAY_RENEW_OFF))
		{

		}
		if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_wd_fault))
		{
			_LIB_USERDELAY_stop(&gTimeout_wd_fault);
			charger_fault_status.MC_STOP_ERR = _OFF;
#if ((__CHARGSERV_DEBUG__)==1)
			_LIB_LOGGING_printf("#### CHARGSERV_FAULT_CLR : MC_STOP_ERR #### \r\n");
#endif
		}
	}
	else
	{
		_LIB_USERDELAY_stop(&gTimeout_wd_fault);
	}

}
void charger_over_voltage_fault()
{
	unsigned int fault_over_voltage = 24800;

	if(charger_fault_status.AC_OV_ERR == _OFF)
	{
		if(Charger.current_V_rms >= fault_over_voltage)
		{
			_LIB_USERDELAY_start(&gTimeout_ac_ov_fault, DELAY_RENEW_OFF);
		}
		else
		{
			_LIB_USERDELAY_stop(&gTimeout_ac_ov_fault);
		}

		if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_ac_ov_fault))
		{
			_LIB_USERDELAY_stop(&gTimeout_ac_ov_fault);
			charger_fault_status.AC_OV_ERR = _ON;
			//eCharger_State temp = _APP_CHARGSERV_get_current_state();
			//printf("current_state = %d \r\n", temp);

		}
	}
	else if(charger_fault_status.AC_OV_ERR == _ON)
	{
		if(Charger.current_V_rms < fault_over_voltage)
		{
			_LIB_USERDELAY_start(&gTimeout_ac_ov_fault, DELAY_RENEW_OFF);
		}
		else
		{
			_LIB_USERDELAY_stop(&gTimeout_ac_ov_fault);
		}

		if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_ac_ov_fault))
		{
			_LIB_USERDELAY_stop(&gTimeout_ac_ov_fault);
			charger_fault_status.AC_OV_ERR = _OFF;
		}
	}
}
void charger_under_voltage_fault()
{
	// //220V * (-10%) = 198V
	// unsigned int fault_low_voltage = 19800;
	
	// if(charger_fault_status.AC_LV_ERR == _OFF)
	// {
	// 	if(Charger.current_V_rms <= fault_low_voltage)
	// 	{
	// 		_LIB_USERDELAY_start(&gTimeout_ac_lv_fault, DELAY_RENEW_OFF);
	// 	}
	// 	else
	// 	{
	// 		_LIB_USERDELAY_stop(&gTimeout_ac_lv_fault);
	// 	}

	// 	if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_ac_lv_fault))
	// 	{
	// 		_LIB_USERDELAY_stop(&gTimeout_ac_lv_fault);
	// 		charger_fault_status.AC_LV_ERR = _ON;
	// 	}
	// }
	// else if(charger_fault_status.AC_LV_ERR == _ON)
	// {
	// 	if(Charger.current_V_rms > fault_low_voltage)
	// 	{
	// 		_LIB_USERDELAY_start(&gTimeout_ac_lv_fault, DELAY_RENEW_OFF);
	// 	}
	// 	else
	// 	{
	// 		_LIB_USERDELAY_stop(&gTimeout_ac_lv_fault);
	// 	}

	// 	if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_ac_lv_fault))
	// 	{
	// 		_LIB_USERDELAY_stop(&gTimeout_ac_lv_fault);
	// 		charger_fault_status.AC_LV_ERR = _OFF;
	// 	}
	// }
}

void charger_over_current_fault()
{
	//110% <= I < 125%, 1000s
	//125% <= I       , 0s

	eCharger_State state = _APP_CHARGSERV_get_current_state();
	CP_StateDef	cp_state = _MW_CP_get_cp_state();
#if 0
	unsigned int waring_over_current = 15500;//CHARGSERV_MAXIMUM_AMPE * 1100;
	unsigned int fault_over_current = 16000;//CHARGSERV_MAXIMUM_AMPE * 1140;
#else
	//unsigned int waring_over_current = Charger.active_Ampe * 1100;//CHARGSERV_MAXIMUM_AMPE * 1100;
	//unsigned int fault_over_current = Charger.active_Ampe * 1250;//CHARGSERV_MAXIMUM_AMPE * 1140;
	unsigned int waring_over_current = Charger.active_Ampe * 1200; //과전류 위험
	unsigned int fault_over_current = Charger.active_Ampe * 1250;//과전류 에러 CHARGSERV_MAXIMUM_AMPE * 1140;
#endif

	if(charger_fault_status.AC_OC_ERR == _OFF)
	{
		if((state == Charging) && (charger_is_load_detect_evse() == _TRUE))
		{
			if((Charger.current_I_rms >= waring_over_current) &&
			(Charger.current_I_rms < fault_over_current))
			{
				if(_TRUE == _LIB_USERDELAY_start(&gTimeout_ac_oc_fault, DELAY_RENEW_OFF))
				{

				}
				if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_ac_oc_fault))
				{
					_LIB_USERDELAY_stop(&gTimeout_ac_oc_fault);
					charger_fault_status.AC_OC_ERR = _ON;
					//Charger.oc_instop_type = 1;
#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV_FAULT_SET : WARING OVER CURRENT #### \r\n");
#endif
				}
			}
			else if(Charger.current_I_rms >= fault_over_current)
			{
#if 0
				_LIB_USERDELAY_stop(&gTimeout_ac_oc_fault);
				charger_fault_status.AC_OC_ERR = _ON;
#if ((__CHARGSERV_DEBUG__)==1)
			_LIB_LOGGING_printf("#### CHARGSERV_FAULT_SET : FAULT OVER CURRENT #### \r\n");
#endif
#else
				if(_TRUE == _LIB_USERDELAY_start(&gTimeout_ac_oc_fast_fault, DELAY_RENEW_OFF))
				{

				}
				if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_ac_oc_fast_fault))
				{
					_LIB_USERDELAY_stop(&gTimeout_ac_oc_fast_fault);
					charger_fault_status.AC_OC_ERR = _ON;
					//Charger.oc_instop_type = 2;
#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("#### CHARGSERV_FAULT_SET : OVERFLOW CURRENT #### \r\n");
#endif
				}
#endif
			}
			else
			{
				_LIB_USERDELAY_stop(&gTimeout_ac_oc_fault);
				_LIB_USERDELAY_stop(&gTimeout_ac_oc_fast_fault);
			}
#if 0
			if(0 != Charger.oc_instop_type)
			{

				_APP_CHARGSERV_device_stop(1);

				if(3 == Charger.oc_instop_step)
				{
#if ((__CHARGSERV_DEBUG__)==1)
					_LIB_LOGGING_printf("#### CHARGSERV : OC_Relapse....Fault detected #### \r\n");
#endif
					_LIB_USERDELAY_stop(&gTimout_oc_waring_Relapse);
					_LIB_USERDELAY_stop(&gTimout_oc_over_Relapse);
					Charger.oc_instop_step = 0;

					charger_fault_status.AC_OC_ERR = _ON;
				}
				else
				{
					Charger.oc_instop_flag = 1;
					Charger.oc_instop_step = 0;

					if(1 == Charger.oc_instop_type)
					{
						_LIB_USERDELAY_start(&gTimout_oc_waring_Relapse, DELAY_RENEW_OFF);
					}
					else if(2 == Charger.oc_instop_type)
					{
						_LIB_USERDELAY_start(&gTimout_oc_over_Relapse, DELAY_RENEW_OFF);
					}

#if ((__CHARGSERV_DEBUG__)==1)
					_LIB_LOGGING_printf("#### CHARGSERV : OC_instop #### \r\n");
#endif
				}

				Charger.oc_instop_type = 0;
			}
#endif
		}
	}
	else if(charger_fault_status.AC_OC_ERR == _ON)
	{
		if(cp_state == DC_12V)
		{
			charger_fault_status.AC_OC_ERR = _OFF;
		}
	}
}
void charger_over_temperature_under_voltage_fault()
{
	uint8_t total_step = 0;
	static uint8_t over_temp_step = 0;
	static uint8_t under_voltage_step = 0;
	//uint8_t total_step = 0;
	unsigned int fault_under_voltage = 18000;
	eCharger_State state = _APP_CHARGSERV_get_current_state();

	if(1 == _APP_CHARGSERV_is_over_temperature_fault_set()) //OTEMP_ERR이 ON이면
	{
		//_APP_CHARGSERV_over_temperature_fault_set();
		_LIB_USERDELAY_start(&gTimeout_over_temp_set_fault, DELAY_RENEW_OFF);//600초 == 10분

		if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_over_temp_set_fault))
		{
			_LIB_USERDELAY_stop(&gTimeout_over_temp_set_fault);
			if(over_temp_step < CHARGSERV_AMPE_CONVERSION_STEP)
			{
				over_temp_step++;
#if ((__CHARGSERV_DEBUG__)==1)
				_LIB_LOGGING_printf("^^^^ over_temperature_step++ : %d \r\n",over_temp_step);
#endif
			}
		}
	}
	else //OTEMP_ERR이 OFF이면
	{
		//_APP_CHARGSERV_over_temperature_fault_reset();
		_LIB_USERDELAY_stop(&gTimeout_over_temp_set_fault);
		if(over_temp_step != 0)
		{
			over_temp_step = 0;
	#if ((__CHARGSERV_DEBUG__)==1)
			_LIB_LOGGING_printf("^^^^ over_temperature_step_clear \r\n");
	#endif

		}
	}

	if(Charging == state)//충전 상태
	{
		if(Charger.current_V_rms < fault_under_voltage)//현재 전압값이 180V 미만일 때 step 증가
		{
			AC_LV_ERR = 1; //활성화
			_LIB_USERDELAY_start(&gTimeout_ac_uv_set_fault, DELAY_RENEW_OFF); //30초

			if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_ac_uv_set_fault))
			{
				_LIB_USERDELAY_stop(&gTimeout_ac_uv_set_fault);
				if(under_voltage_step < CHARGSERV_AMPE_CONVERSION_STEP)
				{
					under_voltage_step++;
#if ((__CHARGSERV_DEBUG__)==1)
					_LIB_LOGGING_printf("^^^^ under_voltage_step++ : %d \r\n",under_voltage_step);
					//printf("current-V_rms1 = %ld \r\n", Charger.current_V_rms);
#endif
				}
				
				_LIB_LOGGING_printf("AC_LV_ERR activated (Voltage < 180V)\r\n");

			}
		}
		else
		{
			AC_LV_ERR = 0; //비활성화
			_LIB_USERDELAY_stop(&gTimeout_ac_uv_set_fault);
		}
	}
	else if((Ready == state) && (under_voltage_step > 0)) //준비 상태이고 저전압 step이 0 초과이면
	{
		if(Charger.current_V_rms >= fault_under_voltage) //현재 전압 값이 180V 이상이면 step = 0
		{
			_LIB_USERDELAY_start(&gTimeout_ac_uv_clr_fault, DELAY_RENEW_OFF);

			if(_TRUE == _LIB_USERDELAY_isfired(&gTimeout_ac_uv_clr_fault))
			{
				_LIB_USERDELAY_stop(&gTimeout_ac_uv_clr_fault);
				if(under_voltage_step != 0)
				{
					under_voltage_step = 0;
#if ((__CHARGSERV_DEBUG__)==1)
					_LIB_LOGGING_printf("^^^^ under_voltage_step_clear \r\n");
#endif
				}
				AC_LV_ERR = 0; //비활성화

				_LIB_LOGGING_printf("AC_LV_ERR deactivated (Ready state, Voltage >= 180V)\r\n");

			}
		}
		else
		{
			_LIB_USERDELAY_stop(&gTimeout_ac_uv_clr_fault);
		}
	}

	total_step = over_temp_step + under_voltage_step; //초과 온도 스텝과 저전압 스텝을 합쳤을 때 2보다 크면
	if(total_step >= CHARGSERV_AMPE_CONVERSION_STEP)
	{
		total_step = CHARGSERV_AMPE_CONVERSION_STEP; //step 을 2로
	}

	_APP_CHARGSERV_set_active_Ampe(total_step);

}

uint8_t _APP_CHARGSERV_is_over_temperature_fault_set()
{
	return TEMP_ERR;
}

void _APP_CHARGSERV_over_temperature_fault_set()
{
	TEMP_ERR = 1;
}

void _APP_CHARGSERV_over_temperature_fault_reset()
{
	TEMP_ERR = 0;
}

void charger_cp_fault()
{
	CP_StateDef	cp_state = _MW_CP_get_cp_state();
	

	if(_ON == charger_fault_status.CP_ERR)
	{
		if(cp_state == DC_12V)
		{
			charger_fault_status.CP_ERR = _OFF;
		}
	}

	else if(_ON == charger_fault_status.CP_VOLTAGE_ERR)
	{
		if(cp_state == DC_12V)
		{
			charger_fault_status.CP_VOLTAGE_ERR = _OFF;
		}
	}
}

void charger_leakage_fault()
{
	CP_StateDef	cp_state = _MW_CP_get_cp_state();

	if(cp_state == DC_12V)
	{
		_APP_CHARGSERV_leakage_fault_reset();
	}
}

uint8_t _APP_CHARGSERV_is_leakage_fault_set()
{
	return charger_fault_status.LEAKAGE_ERR;
}

void _APP_CHARGSERV_leakage_fault_set(uint16_t value)
{

	if(_OFF == Charger.reg.dev_flag)
	{
#if ((_DO_NOT_FAULT_LEAKAGE_) == 0)
		_APP_CHARGSERV_device_stop(1);
		_APP_CHARGSERV_fault_set();
		charger_fault_status.LEAKAGE_ERR = _ON;

#if ((__CHARGSERV_DEBUG__)==1)
		_LIB_LOGGING_printf("#### CHARGSERV : leakage_Fault detected  : %d #### \r\n",value);
#endif
#else
		_APP_CHARGSERV_device_stop(1);

		if(3 == Charger.leakage_instop_step)
		{
#if ((__CHARGSERV_DEBUG__)==1)
			_LIB_LOGGING_printf("#### CHARGSERV : leakage_Relapse....Fault detected  : %d #### \r\n",value);
#endif
			_LIB_USERDELAY_stop(&gTimout_Leakage_Relapse);
			Charger.leakage_instop_step = 0;

			_APP_CHARGSERV_fault_set();
			charger_fault_status.LEAKAGE_ERR = _ON;
		}
		else
		{
			Charger.leakage_instop_flag = 1;
			Charger.leakage_instop_step = 0;
#if ((__CHARGSERV_DEBUG__)==1)
			_LIB_LOGGING_printf("#### CHARGSERV : leakage_instop  : %d #### \r\n",value);
#endif
		}

#endif
	}
}

void _APP_CHARGSERV_leakage_fault_reset()
{
	charger_fault_status.LEAKAGE_ERR = _OFF;
}

void _APP_CHARGSERV_cp_voltage_fault_set()
{
	charger_fault_status.CP_VOLTAGE_ERR = _ON;
}

void _APP_CHARGSERV_cp_voltage_fault_reset()
{
	charger_fault_status.CP_VOLTAGE_ERR = _OFF;
}

void _APP_CHARGSERV_cp_fault_set()
{
	charger_fault_status.CP_ERR = _ON;
}

void _APP_CHARGSERV_cp_fault_reset()
{
	charger_fault_status.CP_ERR = _OFF;
}

void _APP_CHARGSERV_dm_fault_set()
{
	charger_fault_status.DM_COMM_ERR = _ON;
}

void _APP_CHARGSERV_dm_fault_reset()
{
	charger_fault_status.DM_COMM_ERR = _OFF;
}

void _APP_CHARGSERV_rfid_fault_set()
{
	charger_fault_status.RFID_COMM_ERR = _ON;
}

void _APP_CHARGSERV_rfid_fault_reset()
{
	charger_fault_status.RFID_COMM_ERR = _OFF;
}

void _APP_CHARGSERV_save_dm_version(uint16_t value)
{
	Charger.dm_ver_msb = _LIB_BITOP_ext_u16_to_u8one(value, _FILED_MSB_);
	Charger.dm_ver_lsb = _LIB_BITOP_ext_u16_to_u8one(value, _FILED_LSB_);
}

void _APP_CHARGSERV_save_dm_value(uint8_t *dmbuf)
{
	uint32_t temp_active_energy = 0;
	uint32_t temp_vrms = 0;
	uint32_t temp_irms = 0;
	uint32_t temp_active_power = 0;
	uint32_t temp_power_reg = 0;
	uint32_t inputbuf[20] = {0, };
	uint8_t temp_dmbuf = 0x00;

#if ((__CHARGSERV_DEBUG__)==1)
	static uint8_t printf_count = 0;
#endif

	for(int i = 0; i < 14 ; i++)
	{
		temp_dmbuf = dmbuf[i];
		inputbuf[i] = (uint32_t)temp_dmbuf;
	}


	temp_active_energy = (((inputbuf[0] << 24) & 0xFF000000) + ((inputbuf[1] << 16) & 0x00FF0000) + ((inputbuf[2] << 8) & 0x0000FF00) + ((inputbuf[3]) & 0x000000FF));
	temp_vrms = (((inputbuf[4] << 8) & 0x0000FF00) + ((inputbuf[5]) & 0x000000FF));
	temp_irms = (((inputbuf[6] << 24) & 0xFF000000) + ((inputbuf[7] << 16) & 0x00FF0000) + ((inputbuf[8] << 8) & 0x0000FF00) + ((inputbuf[9]) & 0x000000FF));
	temp_power_reg = (((inputbuf[10] << 24) & 0xFF000000) + ((inputbuf[11] << 16) & 0x00FF0000) + ((inputbuf[12] << 8) & 0x0000FF00) + ((inputbuf[13]) & 0x000000FF));
	temp_active_power = temp_vrms * temp_irms;

	if(temp_active_energy != 0)
	{
		Charger.current_Active_Energy = temp_active_energy;
	}

	Charger.current_V_rms = temp_vrms;
	Charger.current_I_rms = temp_irms;
	Charger.current_Active_Power = temp_active_power;
#if ((__CHARGSERV_DEBUG__)==1)
	//_LIB_LOGGING_printf("%lu, %lu, %lu, %lu \r\n", Charger.current_Active_Energy, Charger.current_V_rms, Charger.current_I_rms, Charger.current_Active_Power);
	if(printf_count >= 60)
	{
		_LIB_LOGGING_printf("%lu, %lu, %lu, %lu,\r\n", temp_active_energy, temp_power_reg, temp_vrms, temp_irms);
		printf_count = 0;
	}
	else
	{
		printf_count++;
	}

#endif
}

float _APP_CHARGSERV_get_active_energy_float()
{
	uint32_t temp = Charger.current_Active_Energy;
	float temp1 = ((float)temp) * 1.f;
	float temp2 = 10.f;
	//*value = (float)(temp / 100);
	return (temp1 * temp2);
}

float _APP_CHARGSERV_get_active_energy_calc_float()
{
	uint32_t temp = charger_get_use_energy();
	float temp1 = ((float)temp) * 1.f;
	float temp2 = 10.f;
	//*value = (float)(temp / 100);
	return (temp1 * temp2);
}


#if 0
void _APP_CHARGSERV_get_card_number(char* temp)
{
	//strcpy(temp,Charger.current_card_num);
	sprintf(temp,"%s",Charger.current_card_num);
}
#endif

uint8_t _APP_CHARGSERV_get_plug_state()
{
	//0 : EV_UNPLUG
	//1 : EV_PLUG
	uint8_t ret_value = 0;

	CP_StateDef	cp_state = _MW_CP_get_cp_state();
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();

#if 0
	if((DC_12V == cp_state) || (PWM_12V == cp_state))
#else
	if((DC_12V == cp_state))
#endif
	{
		//if(Charger.reg.charging_continuous == 0)
		//{
			ret_value = 0;
		//}
	}
	else if(PWM_12V == cp_state)
	{
		if(Charger.reg.charging_continuous == 1)
		{
			ret_value = 1;
		}
	}
	else if(((DC_9V == cp_state) || (DC_6V == cp_state) || (PWM_9V == cp_state) || (PWM_6V == cp_state))
			&& (0 == Charger.reg.connect_timeout)
			&& ((UserCheck != charger_state)))
	{
		ret_value = 1;
	}
	else if(1 == charger_get_wake_up_seq_flag())
	{
		ret_value = 1;
	}

	return ret_value;
}

void _APP_CHARGSERV_RemoteStart()
{
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();

	if((charger_state == Ready) && (1 == Charger.reg.ready_standby))
	{
		Charger.reg.ready_standby = 0;
		Charger.reg.ready_remote_start = 1;
	}
}

void _APP_CHARGSERV_RemoteStop()
{
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();

	//if((charger_state == Charging) && (1 == Charger.reg.charging_proceeding))
	//{
		Charger.reg.charging_remote_stop = 1;
		Charger.reg.charging_remote_stop_check = 0;
	//}

}

uint8_t _APP_CHARGSERV_is_RemoteStop()
{
	uint8_t ret_value = _FALSE;
	//eCharger_State charger_state = _APP_CHARGSERV_get_current_state();

	//if((charger_state == Charging) && (1 == Charger.reg.charging_remote_stop))
	if((1 == Charger.reg.charging_remote_stop))
	{
		if(0 == Charger.reg.charging_remote_stop_check)
		{
			Charger.reg.charging_remote_stop_check = 1;
			ret_value = _TRUE;
		}
	}

	return ret_value;
}

void _APP_CHARGSERV_set_chargingcontinuousmode(uint8_t mode)
{
	if(mode == 0)
	{
		Charger.reg.charging_continuous = 0;
	}
	else
	{
		Charger.reg.charging_continuous = 1;
	}
#if ((__CHARGSERV_DEBUG__)==1)
	_LIB_LOGGING_printf("_APP_CHARGSERV_set_charging_continuous : %d. \r\n", mode);
#endif
}

uint8_t _APP_CHARGSERV_is_Ready_for_charging()
{
	uint8_t ret_value = _FALSE;
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();
	CP_StateDef	cp_state = _MW_CP_get_cp_state();

	//if((charger_state == Charging) && (cp_state == PWM_6V))
	//if((cp_state == PWM_6V))
#if 0
	if(((charger_state == Charging) || (charger_state == Finish)) && (cp_state == PWM_6V))
#else
	if(((charger_state == Connect) || (charger_state == Charging) || (charger_state == Finish)) && ((cp_state == PWM_6V) || (cp_state == DC_6V)) && (0 == Charger.reg.connect_timeout))
#endif
	{
		ret_value = _TRUE;
	}

	return ret_value;
}

uint8_t _APP_CHARGSERV_is_state_finish()
{
	uint8_t ret_value = _FALSE;
	eCharger_State charger_state = _APP_CHARGSERV_get_current_state();

	if((charger_state == Finish) && (0 == Charger.reg.finish_stoptranscation_flag))
	{
		Charger.reg.finish_stoptranscation_flag = 1;
		ret_value = _TRUE;
	}

	return ret_value;
}

uint8_t _APP_CHARGSERV_is_id_tagging_stop()
{
	uint8_t ret_value = _FALSE;

	if(1 == Charger.reg.charging_id_tagging_stop)
	{
		//Charger.reg.charging_id_tagging_stop = 0;
		ret_value = _TRUE;
	}

	return ret_value;
}

void _APP_CHARGSERV_set_connectiontimeout(int value)
{
	if(-1 != value)
	{
		charger_set_connect_wait_count(value);
#if ((__CHARGSERV_DEBUG__)==1)
	_LIB_LOGGING_printf("_APP_CHARGSERV_set_connectiontimeout : %d. \r\n", value);
#endif
	}
	else
	{
#if ((__CHARGSERV_DEBUG__)==1)
	_LIB_LOGGING_printf("_APP_CHARGSERV_set_connectiontimeout_fail. \r\n");
#endif
	}
}

void _APP_CHARGSERV_set_charging_to_unplug()
{
	Charger.reg.charging_compulsionunplug = 1;
}

void _APP_CHARGSERV_clear_charging_to_unplug()
{
	Charger.reg.charging_compulsionunplug = 0;
}

uint8_t _APP_CHARGSERV_is_charging_to_unplug()
{
	uint8_t ret_value = _FALSE;

	if(1 == Charger.reg.charging_compulsionunplug)
	{
#if ((__CHARGSERV_DEBUG__)==1)
	//_LIB_LOGGING_printf("charging_compulsionunplug clear \r\n");
#endif
		//Charger.reg.charging_compulsionunplug = 0;
		ret_value = _TRUE;
	}

	return ret_value;
}

uint8_t _APP_CHARGSERV_is_serverconnectstart()
{
	uint8_t ret_value = _FALSE;

	if((1 == Charger.reg.usercheck_serverconnecting) && (0 == Charger.reg.usercheck_authorize_flag))
	{
		Charger.reg.usercheck_authorize_flag = 1;
		ret_value = _TRUE;
	}

	return ret_value;
}

uint8_t _APP_CHARGSERV_is_state_charging_id_tagging()
{
	uint8_t ret_value = _FALSE;

	if(1 == Charger.reg.charging_is_tagging)
	{
		Charger.reg.charging_is_tagging = 0;
		ret_value = _TRUE;
	}

	return ret_value;
}

uint8_t _APP_CHARGSERV_is_dev_mode()
{
	if(1 == Charger.reg.dev_flag)
	{
		return _TRUE;
	}
	else
	{
		return _FALSE;
	}
}

void _APP_CHARGSERV_get_charger_fault_status(eCharger_Fault *value)
{
	*value = charger_fault_status;
}

void _APP_CHARGSERV_fault_loop()
{
#if ((_CERTIFICATION_MODE_)==_CERTIFICATION_NON_)
	if(0 == Charger.reg.dev_flag)//수동 시작 모드
	{
		charger_emg_fault();//추가
		charger_wd_fault();
		charger_over_voltage_fault();
		//charger_under_voltage_fault();
		charger_over_current_fault();
		charger_over_temperature_under_voltage_fault();
		charger_cp_fault();
		charger_leakage_fault();
	}
	else //자동 시작 모드
	{
		charger_emg_fault();//추가
		charger_over_voltage_fault();//추가
		//charger_under_voltage_fault();//추가
		charger_over_current_fault();
		//charger_over_temperature_fault();
		charger_over_temperature_under_voltage_fault();
	}
#elif ((_CERTIFICATION_MODE_)==_CERTIFICATION_KC_EVSE_)
		charger_leakage_fault();
		charger_over_current_fault();
#endif


	if(charger_fault_status.Raw != 0)
	{
		_APP_CHARGSERV_fault_set();
		
		if((charger_fault_status.AC_OC_ERR == _OFF)
			&& (charger_fault_status.LEAKAGE_ERR == _OFF)
			&& (charger_fault_status.CP_ERR == _OFF)
			&& (charger_fault_status.CP_VOLTAGE_ERR == _OFF)
			&& (charger_fault_status.CSMS_COMM_ERR == _OFF)
			&& (charger_fault_status.MC_START_ERR == _OFF))
		{
			_APP_CHARGSERV_device_stop(0); //MC_relay fast stop
		}
		else
		{
			_APP_CHARGSERV_device_stop(1); //MC_relay stop
		}
		
	}

	if(charger_fault_status.Raw == 0/*fault status bits is reset*/)
	{
		_APP_CHARGSERV_fault_reset();
	}
}
/*
 * ------------------------------------------------------------------------------
 */

void _APP_CHARGSERV_get_event(void)
{
#if 0
	if(1 == _LIB_FREERTOS_message_receive(ChargServMQHandle, 0, msgrx))
	{
		switch(msgrx->cmd)
		{
			case ITC_RFIDAPP_STATE_CMD :
				if(ITC_RFIDAPP_STATE_INIT_OK == msgrx->data[ITC_RFIDAPP_STATE_INDEX])
				{
					//msg tx to rfidapp_INIT_OK_ACK
					charger_msg_set(msg_tx, CHARGSERV_MQ_TX_RFIDAPP_INIT_OK_ACK);
					//_LIB_LOGGING_printf("rfid_init_ok_rx\r\n");
				}
				else if(ITC_RFIDAPP_STATE_TAG_ERROR == msgrx->data[ITC_RFIDAPP_STATE_INDEX])
				{
					//Fail Tagging. error Display, Try again
					charger_msg_set(msg_rx, CHARGSERV_MQ_RX_RFIDAPP_TAG_ERROR);
				}
			break;

			case ITC_RFIDAPP_CARDNUM_CMD :
				//Success Tagging.
				_APP_CHARGSERV_ready_cardtag();
				//Save Tagging card number
				//TODO
				//
			break;

			case ITC_DMAPP_STATE_CMD :
				if(ITC_DMAPP_STATE_INIT_OK == msgrx->data[ITC_DMAPP_STATE_INDEX])
				{
					charger_msg_set(msg_tx, CHARGSERV_MQ_TX_DMAPP_INIT_OK_ACK);

					//save dm version
					Charger.dm_ver_msb = msgrx->data[ITC_DMAPP_STATE_INDEX_VERSION_MSB];
					Charger.dm_ver_lsb = msgrx->data[ITC_DMAPP_STATE_INDEX_VERSION_LSB];


				}
				//Todo
				// DMAPP ERROR(FAULT) Process
			break;

			case ITC_DMAPP_VALUE_CMD :
				//_LIB_LOGGING_printf("dm_value_input\r\n");
				charger_save_dm_value(msgrx->data);
			break;

			//Todo
			//go to system_control!!
			case ITC_CONFIGAPP_TO_CHARGSERV_STATUS_CMD :
				if(msgrx->data[ITC_CONFIGAPP_TO_CHARGSERV_STATUS_INDEX] == ITC_CONFIGAPP_TO_CHARGSERV_STATUS_OVER_TEMP)
				{
					// OVER TEMP ACTIVITED.
					_APP_CHARGSERV_over_temperature_fault_set();
				}
				else if(msgrx->data[ITC_CONFIGAPP_TO_CHARGSERV_STATUS_INDEX] == ITC_CONFIGAPP_TO_CHARGSERV_STATUS_NOR_TEMP)
				{
					// NOR TEMP ACTIVIED.
					_APP_CHARGSERV_over_temperature_fault_reset();
				}
			break;
		}
	}
#endif
}

void _APP_CHARGSERV_set_event(void)
{
#if 0
	osMessageQueueId_t temp_mq;

	if(charger_msg_is_set(msg_tx, CHARGSERV_MQ_TX_RFIDAPP_INIT_OK_ACK))
	{
		charger_msg_clear(msg_tx, CHARGSERV_MQ_TX_RFIDAPP_INIT_OK_ACK);//Charger.msg_tx_reg &= ~CHARGSERV_MQ_TX_RFIDAPP_INIT_OK_ACK;
		msgtx->cmd = ITC_RFIDAPP_STATE_CMD;
		msgtx->length = ITC_RFIDAPP_STATE_LENGTH;
		msgtx->data[ITC_RFIDAPP_STATE_INDEX] = ITC_RFIDAPP_STATE_INIT_OK_ACK;
		temp_mq = RfidcommMQHandle;
		_LIB_FREERTOS_message_send(temp_mq, msgtx);
	}
	else if(charger_msg_is_set(msg_tx, CHARGSERV_MQ_TX_RFIDAPP_TAG_START))
	{
		charger_msg_clear(msg_tx, CHARGSERV_MQ_TX_RFIDAPP_TAG_START);//Charger.msg_tx_reg &= ~CHARGSERV_MQ_TX_RFIDAPP_TAG_START;
		msgtx->cmd = ITC_RFIDAPP_STATE_CMD;
		msgtx->length = ITC_RFIDAPP_STATE_LENGTH;
		msgtx->data[ITC_RFIDAPP_STATE_INDEX] = ITC_RFIDAPP_STATE_TAG_START;
		temp_mq = RfidcommMQHandle;
		_LIB_FREERTOS_message_send(temp_mq, msgtx);
	}
	else if(charger_msg_is_set(msg_tx, CHARGSERV_MQ_TX_DMAPP_INIT_OK_ACK))
	{
		charger_msg_clear(msg_tx, CHARGSERV_MQ_TX_DMAPP_INIT_OK_ACK);
		msgtx->cmd = ITC_DMAPP_STATE_CMD;
		msgtx->length = ITC_DMAPP_STATE_LENGTH;
		msgtx->data[ITC_DMAPP_STATE_INDEX] = ITC_DMAPP_STATE_INIT_OK_ACK;
		temp_mq = DMcommMQHandle;
		_LIB_FREERTOS_message_send(temp_mq, msgtx);
	}
	else if(charger_msg_is_set(msg_tx, CHARGSERV_MQ_TX_DMAPP_VALUE_REQ))
	{
		charger_msg_clear(msg_tx, CHARGSERV_MQ_TX_DMAPP_VALUE_REQ);
		msgtx->cmd = ITC_DMAPP_VALUEREQ_CMD;
		msgtx->length = ITC_DMAPP_VALUEREQ_LENGTH;
		msgtx->data[ITC_DMAPP_VALUEREQ_INDEX] = ITC_DMAPP_VALUEREQ_VALUE_ALL;
		temp_mq = DMcommMQHandle;
		_LIB_FREERTOS_message_send(temp_mq, msgtx);
	}

#endif
}

void _APP_CHARGSERV_startup()
{
	Charger.reg.Raw = 0;
	Charger.init_flag.Raw = 0x00;

	charger_set_state(Init);
	Charger.reg.init_proceeding = 1;

	//charger_reset_connect_wait_count();
	charger_reset_powercut_pass_count();

	//Charger.state_changed = _OFF;
	Charger.state_bk = Chargerstateend;
	//Charger.msg_rx_reg = 0x00;
	//Charger.msg_tx_reg = 0x00;

	Charger.userconfigmode_input_count = 0;
	Charger.automode_active = 0;

	Charger.forcestop_wake_up_seq_count = 0;
	Charger.forcestop_wake_up_seq_step = 0;
	Charger.forcestop_wake_up_seq_flag = 0;
	Charger.forcestop_wake_up_seq_count_limit_detect = 0;

	Charger.full_wake_up_seq_step = 0;

	//Charger.fullcharge_finish_flag = 0;
	Charger.stable_changed_flag = 0;
	Charger.stable_full_detect = 0;
	Charger.autostartmode_flag = 0;

	Charger.default_start_mode = 0;
	Charger.restart_fulldetect_act = 0;

	Charger_reset_use_energy();

	memset(Charger.current_card_num,0xFF,30);
	memset(Charger.charging_card_num,0xFF,30);

	_LIB_USERDELAY_set(&gDelay_chargserv_periodic_loop_time,10);
	_LIB_USERDELAY_set(&gTimeout_wd_fault,5000);
	_LIB_USERDELAY_set(&gTimeout_ac_oc_fault,1000000);
	_LIB_USERDELAY_set(&gTimeout_ac_oc_fast_fault,10000);
	_LIB_USERDELAY_set(&gTimeout_ac_ov_fault,5000);
	_LIB_USERDELAY_set(&gTimeout_ac_lv_fault,5000);
	_LIB_USERDELAY_set(&gTimeout_ac_uv_set_fault,30000);
	_LIB_USERDELAY_set(&gTimeout_ac_uv_clr_fault,5000);
	_LIB_USERDELAY_set(&gDelay_cp_fault,1000);
	_LIB_USERDELAY_set(&gTimeout_over_temp_set_fault,600000);
	_LIB_USERDELAY_set(&gTimeout_over_temp_clr_fault,5000);
	_LIB_USERDELAY_set(&gTimeout_auto_finish,CHARGSERV_AUTOFINISH_TIMEOUT);
	_LIB_USERDELAY_set(&gDelay_finish_mcoff,3000);

	_LIB_USERDELAY_set(&gTimeout_connect, 60000);
//220802 PES : charging_stable, charging_full detect
	_LIB_USERDELAY_set(&gTimeout_charging_stable,5400000);
	//_LIB_USERDELAY_set(&gTimeout_charging_stable,300000);
	_LIB_USERDELAY_set(&gTimeout_charging_full,30000);

	_LIB_USERDELAY_set(&gTimeout_csms_fault,1800000);

	_LIB_USERDELAY_set(&gTimeout_usercheck,25000);
	_LIB_USERDELAY_set(&gTimeout_userconfigmode_input,3000);
	_LIB_USERDELAY_set(&gDelay_autostartmode_input,10000);
	_LIB_USERDELAY_set(&gDelay_charging_ok_button_input, 500);

	_LIB_USERDELAY_set(&gTimeout_full_charge_minimum_ampe, 60000);

	_LIB_USERDELAY_set(&gDelay_forcestop_wake_up_seq_flag_set, 3000);

	//_LIB_USERDELAY_set(&gTimeout_forcestop_wake_up_seq_is_finish, 20000);
	//_LIB_USERDELAY_set(&gTimeout_full_wake_up_seq_is_finish, 15000);
	_LIB_USERDELAY_set(&gTimeout_forcestop_wake_up_seq_is_finish, 25000);
	_LIB_USERDELAY_set(&gTimeout_full_wake_up_seq_is_finish, 30000);

	_LIB_USERDELAY_set(&gTimout_Leakage_Relapse, 10000);

	_LIB_USERDELAY_set(&gTimout_oc_waring_Relapse, 1100000);

	_LIB_USERDELAY_set(&gTimout_oc_over_Relapse, 15000);
	//_MW_GPIO_set_gpo(KWH_LED, _OFF);
	//_MW_GPIO_set_gpo(LOAD_LED, _OFF);

	_LIB_USERDELAY_set(&gDelay_autostartmode_default_set,500);

	charger_set_modecfg();

	_APP_CHARGSERV_set_active_Ampe(0);

	_APP_CHARGSERV_wake_up_seq_init();

#if ((_EVERON_OCPP_SERVER_CONNECT_) == 2)
	printf("################## NON SERVER CONNECT ####################\r\n");
	_APP_CHARGSERV_set_connectiontimeout(60);
#endif

//#if ((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
//	_LIB_LPF_init(&Vrms_calc, _LPF_TIMEINTERVAL_, _LPF_TAU_);
//	_LIB_LPF_init(&Irms_calc, _LPF_TIMEINTERVAL_, _LPF_TAU_);
//#endif

}

void _APP_CHARGSERV_init()
{
	if(_TRUE == _APP_RFID_is_init_ok()) //카드 리더기 활성화
	{
		Charger.init_flag.rfid_init_ok = 1;
	}

	Charger.init_flag.systemctl_init_ok = 1;

	//230106 PES : non display. always on
	Charger.reg.display_cyclefinish = 1;

	_APP_CHARGSERV_init_ok(); //Charger.reg.init_complete = 1;
}

uint8_t _APP_CHARGSERV_deinit()
{
	charger_set_state(Init);
	Charger.reg.init_proceeding = 1;
	Charger.state_bk = Chargerstateend;

	charger_set_modecfg();

	_APP_CHARGSERV_set_active_Ampe(0);

	_APP_CHARGSERV_wake_up_seq_init();

	return 0;
}


void _APP_CHARGSERV_main()
{
	eCharger_State state = _APP_CHARGSERV_get_current_state();

	_APP_CHARGSERV_init();

	_MW_CP_main();

	if((state != Init) && (state != OTA) && (state != Powercut))
	{
		_APP_CHARGSERV_fault_loop();
	}
	_APP_CHARGSERV_state_machine();
	_APP_CHARGSERV_ctrl_loop();

#if((__CHARGSERV_DEBUG__)==1)
	_APP_CHARGSERV_print_state();
#endif
}

void _APP_CHARGSERV_process(void)
{
	eCharger_Mode mode = charger_get_mode();

	_LIB_USERDELAY_start(&gDelay_chargserv_periodic_loop_time, DELAY_RENEW_OFF);

	if(mode_config == mode) //설정 모드
	{
		return; //프로세스 종료
	}

    if(_LIB_USERDELAY_isfired(&gDelay_chargserv_periodic_loop_time))
    {
    	_APP_CHARGSERV_get_event();

		_APP_CHARGSERV_main();

		_APP_CHARGSERV_display_loop();

		_APP_CHARGSERV_set_event();

	  	_LIB_USERDELAY_start(&gDelay_chargserv_periodic_loop_time, DELAY_RENEW_ON);
    }
}




