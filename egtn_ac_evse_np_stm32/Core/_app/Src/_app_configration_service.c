/*
 * _app_configration_service.c
 *
 *  Created on: 2023. 3. 16.
 *      Author: MSI
 */

#include <_app_configration_service.h>
#include <_app_charging_service.h>
#include <_app_system_control.h>
#include <_app_rfid_comm.h>
#include <_config.h>

static sConfig Config;
static gUserDelay gDelay_configserv_periodic_loop_time;
static gUserDelay gDelay_finish;
static gUserDelay gDelay_fault;

uint8_t _APP_CONFIGSERV_flag(uint8_t value, uint8_t is_userconfig)
{
	uint8_t ret_value = _TRUE;

	if((_TRUE == value) && (0 == Config.reg.enable))
	{
		Config.reg.enable = 1;
		Config.state = C_Init;
		Config.reg.init_proceeding = 1;
		Config.state_bk = C_Init;
		memset(Config.read_card_num,0xFF,30);
		printf("%%%%%%$ CONFIGRATION FLAG ON Enable set %%%%%%$\r\n");
		if(_TRUE == is_userconfig)
		{
			printf("%%%%%%$ CONFIGRATION _ USERCONFIGMODE ON %%%%%%$\r\n");
			Config.reg.is_userconfigmode = 1;
		}
	}
	else if((_FALSE == value) && (1 == Config.reg.enable))
	{
		Config.reg.enable = 0;
		printf("%%%%%%$ CONFIGRATION FLAG OFF Enable reset %%%%%%$\r\n");
	}
	else
	{
		ret_value = _FALSE;
	}

	return ret_value;
}

void _APP_CONFIGSERV_startup()
{
	Config.reg.Raw = 0;

	if(CONFIGSERV_READ_MODE_ENABLE)
	{
		printf("%%%%%%$ CONFIGRATION MODE %%%%%%$\r\n");

		Config.reg.enable = 1;
		Config.state = C_Init;
		Config.reg.init_proceeding = 1;
		Config.state_bk = C_Init;

		memset(Config.read_card_num,0xFF,30);
	}
	else
	{
		Config.reg.enable = 0;
		Config.state = Configstateend;

		printf("is not CONFIGRATION MODE \r\n");
	}

	_LIB_USERDELAY_set(&gDelay_configserv_periodic_loop_time,10);
	_LIB_USERDELAY_set(&gDelay_finish,2000);
	_LIB_USERDELAY_set(&gDelay_fault,2000);
}

void _APP_CONFIGSERV_init()
{
	static uint8_t init_stack = 0xFF;

	if(_TRUE == _APP_RFID_is_init_ok())
	{
		init_stack = 0x00;
	}

	if(0 == init_stack)
	{
		Config.reg.init_complete = 1;
	}
}

uint8_t config_state_changed_check()
{
	if(Config.state != Config.state_bk)
	{
		switch(Config.state)
		{
			case C_Init :
				_LIB_LOGGING_printf("%%%% CONFIGSERV State : Init #### \r\n");
			break;

			case C_Ready :
				_LIB_LOGGING_printf("#### CHARGSERV State : Ready #### \r\n");
			break;

			case C_Setting :
				_LIB_LOGGING_printf("#### CHARGSERV State : Setting #### \r\n");
			break;

			case C_Finish :
				_LIB_LOGGING_printf("#### CHARGSERV State : Finish #### \r\n");
			break;

			case C_Fault :
				_LIB_LOGGING_printf("#### CHARGSERV State : Fault #### \r\n");
			break;

			default :
				_LIB_LOGGING_printf("#### CHARGSERV State : default #### \r\n");
			break;
		}

		if(((C_Finish == Config.state_bk) || (C_Fault == Config.state_bk)) && (C_Ready == Config.state))
		{
			if(1 == Config.reg.is_userconfigmode)
			{
				Config.reg.is_userconfigmode = 0;
				Config.state = C_Init;
				printf("userconfigmode is only one try. go to chargermode \r\n");
				_APP_CONFIGSERV_flag(_FALSE,_FALSE);
				_APP_CHARGSERV_deinit();
			}
		}

		Config.state_bk = Config.state;
		Config.reg.state_changed = 1;
	}
	return _TRUE;
}

void _APP_CONFIGSERV_state_machine()
{
	switch(Config.state)
	{
		case C_Init :
			if((1 == Config.reg.init_proceeding) &&
			   (1 == Config.reg.init_complete))
			{
				Config.reg.init_proceeding = 0;
				Config.reg.init_complete = 0;

				Config.state = C_Ready;
				Config.reg.ready_standby = 1;
			}
		break;

		case C_Ready :
			if(1 == Config.reg.ready_cardtag)
			{
				Config.reg.ready_standby = 0;
				Config.reg.ready_cardtag = 0;

				Config.state = C_Setting;
				Config.reg.setting_proceeding = 1;
			}
			else if(1 == Config.reg.ready_cardtagfail)
			{
				Config.reg.ready_standby = 0;
				Config.reg.ready_cardtagfail = 0;

				Config.state = C_Fault;
				Config.reg.fault_set = 1;
			}
		break;

		case C_Setting :
			if(1 == Config.reg.setting_proceeding)
			{
				if(1 == Config.reg.setting_ok)
				{
					Config.reg.setting_proceeding = 0;
					Config.reg.setting_ok = 0;

					Config.state = C_Finish;
					Config.reg.finish_proceeding = 1;
				}
				else if(1 == Config.reg.setting_fail)
				{
					Config.reg.setting_proceeding = 0;
					Config.reg.setting_fail = 0;

					Config.state = C_Fault;
					Config.reg.fault_set = 1;
				}
			}
		break;

		case C_Finish :
			if((1 == Config.reg.finish_proceeding) &&
			   (1 == Config.reg.finish_ok))
			{
				Config.reg.finish_proceeding = 0;
				Config.reg.finish_ok = 0;

				Config.state = C_Ready;
				Config.reg.ready_standby = 1;
			}
		break;

		case C_Fault :
			if(1 == Config.reg.fault_reset)
			{
				Config.reg.fault_set = 0;
				Config.reg.fault_reset = 0;

				Config.state = C_Ready;
				Config.reg.ready_standby = 1;
			}
		break;

		default :

		break;

	}

	config_state_changed_check();
}

uint8_t config_is_state_change()
{
	uint8_t ret_value = _FALSE;

	if(1 == Config.reg.state_changed)
	{
		Config.reg.state_changed = 0;
		ret_value = _TRUE;
	}

	return ret_value;
}

void _APP_CONFIGSERV_ctrl_loop()
{
	uint8_t ret_value = 0x00;
	static int8_t step = 0;
	int i = 0;
	int j = 0;
	uint8_t rxtemp[30] = {0,};
	uint8_t txtemp[30] = {0,};

	switch(Config.state)
	{
		case C_Init :
			if(_TRUE == config_is_state_change())
			{

			}


		break;

		case C_Ready :
			if(_TRUE == config_is_state_change())
			{
				_APP_RFID_set_start_tagging(0);
			}

			ret_value = _APP_RFID_get_tagging_result();
			if(_TRUE == ret_value)
			{
				_APP_RFID_get_card_number(Config.read_card_num);
				_LIB_LOGGING_printf("%%%% CONFIGSERV Ready : Read Card Number #### \r\n");
				Config.reg.ready_cardtag = 1;
			}
			else if(_FALSE == ret_value)
			{
				_LIB_LOGGING_printf("%%%% CONFIGSERV Ready : fail Card Number. go to fault #### \r\n");
				Config.reg.ready_cardtagfail = 1;
			}

		break;

		case C_Setting :
			if(_TRUE == config_is_state_change())
			{
				step = 0;
				i = 0;
			}
			switch(step)
			{
				case 0 :	// step 1. check is collect id card number (ex : 16digit)
					while(i<30)
					{
						if(Config.read_card_num[i] == 0xFF)
						{
							break;
						}
						else
						{
							i++;
						}
					}

					if(i == 8)
					{
						_LIB_LOGGING_printf("%%%% CONFIGSERV Setting : Step 1 _ OK #### \r\n");
						step = 1;
					}
					else
					{
						_LIB_LOGGING_printf("%%%% CONFIGSERV Setting : Step 1 _ Fail : %d #### \r\n",i);
						step = -1;
					}
				break;

				case 1 :	// step 2. check reduplication
					for(i = 0; i<MAX_CARDNUM_MEMORY ; i++)
					{
						if(true == ee24_read(MEM_ADDR_CARDNUM_INDEX_MSB(i), rxtemp, 16, 100))
						{
							for(j = 0; j<8;j++)
							{
								if(Config.read_card_num[j] != rxtemp[8+j])
								{
									break;
								}
							}

							if(8 == j)
							{
								_LIB_LOGGING_printf("%%%% CONFIGSERV Setting : Step 2 _ is reduplication. go to finish #### \r\n");
								step = 3;
								break;
							}
						}
					}

					if(step != 3)
					{
						if(MAX_CARDNUM_MEMORY == i)
						{
							_LIB_LOGGING_printf("%%%% CONFIGSERV Setting : Step 2 _ is new. #### \r\n");
							step = 2;
						}
						else
						{
							_LIB_LOGGING_printf("%%%% CONFIGSERV Setting : Step 2 _ Fail : #### \r\n");
							step = -1;
						}
					}
				break;

				case 2 :	// step 3. save eeprom id card number
					for(i = 0; i<MAX_CARDNUM_MEMORY ; i++)
					{
						if(true == ee24_read(MEM_ADDR_CARDNUM_INDEX_LSB(i), rxtemp, 1, 100))
						{
							if(0xFF == rxtemp[0])
							{
								txtemp[0] = 0x00;
								txtemp[1] = 0x00;
								txtemp[2] = 0x00;
								txtemp[3] = 0x00;
								txtemp[4] = 0x00;
								txtemp[5] = 0x00;
								txtemp[6] = 0x00;
								txtemp[7] = (i+1);
								txtemp[8] = Config.read_card_num[0];
								txtemp[9] = Config.read_card_num[1];
								txtemp[10] = Config.read_card_num[2];
								txtemp[11] = Config.read_card_num[3];
								txtemp[12] = Config.read_card_num[4];
								txtemp[13] = Config.read_card_num[5];
								txtemp[14] = Config.read_card_num[6];
								txtemp[15] = Config.read_card_num[7];

								ee24_write(MEM_ADDR_CARDNUM_INDEX_MSB(i), txtemp, 16, 100);

/*								config_app.usecardnum[i][0] = txtemp[8];
								config_app.usecardnum[i][1] = txtemp[9];
								config_app.usecardnum[i][2] = txtemp[10];
								config_app.usecardnum[i][3] = txtemp[11];
								config_app.usecardnum[i][4] = txtemp[12];
								config_app.usecardnum[i][5] = txtemp[13];
								config_app.usecardnum[i][6] = txtemp[14];
								config_app.usecardnum[i][7] = txtemp[15];*/

								break;
							}
						}
					}

					if(MAX_CARDNUM_MEMORY == i)
					{
						_LIB_LOGGING_printf("%%%% CONFIGSERV Setting : Step 3 _ Fail : #### \r\n");
						step = -1;
					}
					else
					{
						_LIB_LOGGING_printf("%%%% CONFIGSERV Setting : Step 3 _ CARDNUM_save ok : %d : #### \r\n", i);
						step = 3;
					}
				break;

				case 3 :	// step 4. go to finish
					if(0 == Config.reg.setting_ok)
					{
						Config.reg.setting_ok = 1;
						_LIB_LOGGING_printf("%%%% CONFIGSERV Setting : Step 4 _ finish : #### \r\n");
					}

				break;

				case -1 :	// exception. go to fault
					if(0 == Config.reg.setting_fail)
					{
						Config.reg.setting_fail = 1;
						_LIB_LOGGING_printf("%%%% CONFIGSERV Setting : exception _ fault : #### \r\n");
					}

				break;

				default :

				break;
			}
		break;

		case C_Finish :
			if(_TRUE == config_is_state_change())
			{
				_LIB_USERDELAY_start(&gDelay_finish,DELAY_RENEW_OFF);
				_LIB_LOGGING_printf("%%%% CONFIGSERV Finish : delay_start : #### \r\n");
			}

			if(_TRUE == _LIB_USERDELAY_isfired(&gDelay_finish))
			{
				_LIB_LOGGING_printf("%%%% CONFIGSERV Finish : delay_OK : #### \r\n");
				_LIB_USERDELAY_stop(&gDelay_finish);
				Config.reg.finish_ok = 1;
			}

		break;

		case C_Fault :
			if(_TRUE == config_is_state_change())
			{
				_LIB_USERDELAY_start(&gDelay_fault,DELAY_RENEW_OFF);
				_LIB_LOGGING_printf("%%%% CONFIGSERV Fault : delay_start : #### \r\n");
			}

			if(_TRUE == _LIB_USERDELAY_isfired(&gDelay_fault))
			{
				_LIB_LOGGING_printf("%%%% CONFIGSERV Fault : delay_OK : #### \r\n");
				_LIB_USERDELAY_stop(&gDelay_fault);
				Config.reg.fault_reset = 1;
			}
		break;

		default :

		break;
	}
}

void _APP_CONFIGSERV_display_loop()
{
	switch(Config.state)
	{
		case C_Init :
			_MW_INDILED_sled_ctl(YELLOW);
		break;

		case C_Ready :
			_MW_INDILED_sled_ctl(YELLOW);
		break;

		case C_Setting :
			_MW_INDILED_sled_ctl(BLUE);
		break;

		case C_Finish :
			_MW_INDILED_sled_ctl(GREEN);
		break;

		case C_Fault :
			_MW_INDILED_sled_ctl(RED);
		break;

		default :

		break;
	}
}

void _APP_CONFIGSERV_main()
{
	_APP_CONFIGSERV_init();

	_APP_CONFIGSERV_state_machine();
	_APP_CONFIGSERV_ctrl_loop();
}

void _APP_CONFIGSERV_process()
{
	_LIB_USERDELAY_start(&gDelay_configserv_periodic_loop_time, DELAY_RENEW_OFF);

	if(0 == Config.reg.enable)
	{
		return;
	}

    if(_LIB_USERDELAY_isfired(&gDelay_configserv_periodic_loop_time))
    {
		_APP_CONFIGSERV_main();

		_APP_CONFIGSERV_display_loop();

	  	_LIB_USERDELAY_start(&gDelay_configserv_periodic_loop_time, DELAY_RENEW_ON);
    }
}
