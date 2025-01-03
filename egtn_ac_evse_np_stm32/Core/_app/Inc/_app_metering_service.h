/*
 * _app_metering_service.h
 *
 *  Created on: 2024. 5. 14.
 *      Author: USER
 */

#ifndef APP_INC__APP_METERING_SERVICE_H_
#define APP_INC__APP_METERING_SERVICE_H_

#include "main.h"
#include "math.h"

#include <_mw_it.h>

#include <_lib_logging.h>
#include <_lib_userdelay.h>
#include <_lib_LPF.h>

#include <_app_charging_service.h>

#define _VRMS_IRMS_CALC_LPF_FILTER_		1
#if	((_VRMS_IRMS_CALC_LPF_FILTER_) == 1)
#define _LPF_TIMEINTERVAL_VOL_ (2)
#define _LPF_TIMEINTERVAL_CUR_ (1)
#define _LPF_TAU_VOL_ (800)
#define _LPF_TAU_CUR_ (200)
#endif
#define _IRMS_INPUT_LPF_FILTER_		0
#if	((_IRMS_INPUT_LPF_FILTER_) == 1)
#define _IRMS_INPUT_LPF_TIMEINTERVAL_ (2)
#define _IRMS_INPUT_LPF_TAU_ (10)
#endif

void _APP_METERING_startup();
void _APP_METERING_process();


#endif /* APP_INC__APP_METERING_SERVICE_H_ */
