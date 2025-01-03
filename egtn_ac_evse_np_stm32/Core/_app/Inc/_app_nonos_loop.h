/*
 * _app_nonos_loop.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef APP_INC__APP_NONOS_LOOP_H_
#define APP_INC__APP_NONOS_LOOP_H_

#include "main.h"

#include <_app_charging_service.h>
#include <_app_system_control.h>
#include <_app_configration_service.h>
#include <_app_rfid_comm.h>
#include <_app_metering_service.h>

void _system_go_to_process_delay();

extern void acevsesystem();

#endif /* APP_INC__APP_NONOS_LOOP_H_ */
