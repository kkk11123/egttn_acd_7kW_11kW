/*
 * _lib_userdelay.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef LIB_INC__LIB_USERDELAY_H_
#define LIB_INC__LIB_USERDELAY_H_


#include "main.h"

#define GUSERDELAY_PERIOD		10		//unit : ms

// Delay status
typedef enum {DELAY_INACTIVE,DELAY_ACTIVE,USERDELAYSTAT_END}			tUSERDELAYSTAT;
typedef enum {DELAY_RENEW_OFF,DELAY_RENEW_ON,USERDELAYRENEWSTAT_END}	tUSERDELAYRENEWSTAT;

typedef struct SETDELAY{
	tUSERDELAYSTAT state;
	unsigned long uStartTick;
	unsigned long nDelay;
}gUserDelay;

void _LIB_USERDELAY_tickcount(void);
uint32_t _LIB_USERDLEAY_gettick(void);
void _LIB_USERDELAY_set(gUserDelay *Delay, unsigned long delayvalue);
unsigned int _LIB_USERDELAY_start(gUserDelay *Delay, tUSERDELAYRENEWSTAT Renew);
void _LIB_USERDELAY_stop(gUserDelay *Delay);
unsigned long CheckDelayValue(unsigned long nStartTime);
unsigned int _LIB_USERDELAY_isfired(gUserDelay *Delay);

#endif /* LIB_INC__LIB_USERDELAY_H_ */
