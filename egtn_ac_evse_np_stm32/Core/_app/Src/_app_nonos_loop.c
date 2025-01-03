/*
 * _app_nonos_loop.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */


#include <_app_nonos_loop.h>

static gUserDelay gDelay_go_to_process;

void acevsesystem()
{
	_LIB_LOGGING_printf("|-------- AC EVSE NP-----------|\r\n");
	_LIB_LOGGING_printf("|-------- 7_11kW ----------|\r\n");
	_LIB_LOGGING_printf("|-------- EGTRONIC------------|\r\n");
	_LIB_LOGGING_printf("|_FW_VERSION_ : %d ----------|\r\n",_FW_VERSION_);
	_LIB_LOGGING_print_date_time("|--- Compile :");

#if	((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
	_LIB_LOGGING_printf("|-LPF_FILTER_ENABLE----------|\r\n");
#endif

	_MW_GPIO_init();
	_MW_IT_init();
	_MW_CP_init();
	_MW_PWM_init();
	_MW_INDILED_init();

	_APP_SYSTEMCTL_startup();
	_APP_METERING_startup();
	_APP_CHARGSERV_startup();
	_APP_CONFIGSERV_startup();
	_APP_RFID_startup();

	_system_go_to_process_delay();

	while(1)
	{
		_APP_SYSTEMCTL_process();

		_APP_METERING_process();

		_APP_CHARGSERV_process();

		_APP_CONFIGSERV_process();

		_APP_RFID_comm_process();
	}
}

void _system_go_to_process_delay()
{
	_LIB_USERDELAY_set(&gDelay_go_to_process, 500);

	_LIB_USERDELAY_start(&gDelay_go_to_process, DELAY_RENEW_OFF);

	while(0 == _LIB_USERDELAY_isfired(&gDelay_go_to_process));

	_LIB_USERDELAY_stop(&gDelay_go_to_process);

	_LIB_LOGGING_printf("go to process...\r\n");
}
