/*
 * _lib_userdelay.c
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */




#include <_lib_userdelay.h>
#include <_lib_logging.h>

static volatile uint32_t gUserDelayTick = 0;


void _LIB_USERDELAY_tickcount(void)
{
	gUserDelayTick++;
}

uint32_t _LIB_USERDLEAY_gettick(void)
{
	return gUserDelayTick;
}

void _LIB_USERDELAY_set(gUserDelay *Delay, unsigned long delayvalue)
{
	unsigned long delay_to_tick = delayvalue * GUSERDELAY_PERIOD;

	if(delay_to_tick == 0)
	{
		Delay->nDelay = 0;
		return;
	}
	else
	{
		Delay->nDelay = delay_to_tick;
		//printf("delay : %ld\r\n", delay_to_tick);
	}

	Delay->state = DELAY_INACTIVE;
	Delay->uStartTick = delay_to_tick;
}

unsigned int _LIB_USERDELAY_start(gUserDelay *Delay, tUSERDELAYRENEWSTAT Renew)
{
	if(Renew == DELAY_RENEW_OFF)
	{
		if(Delay->state == DELAY_ACTIVE)		return 0;
	}

	Delay->state = DELAY_ACTIVE;
	Delay->uStartTick = gUserDelayTick;
	//printf("StartTick = %ld\r\n", gUserDelayTick);
	return 1;
}

void _LIB_USERDELAY_stop(gUserDelay *Delay)
{
	Delay->state = DELAY_INACTIVE;
}

unsigned long CheckDelayValue(unsigned long nStartTime)
{
	unsigned long uCurrentTick;
	//unsigned long starttime_to_tick = nStartTime;


	uCurrentTick = gUserDelayTick;

	if(uCurrentTick >= nStartTime)
	{
		return (uCurrentTick - nStartTime);
	}
	else
	{
		return (0xFFFFFFFF - nStartTime + 1 + uCurrentTick);
	}
}

unsigned int _LIB_USERDELAY_isfired(gUserDelay *Delay)
{
	if(Delay->state == DELAY_INACTIVE)
	{
		return 0;
	}
	if(CheckDelayValue(Delay->uStartTick) >= Delay->nDelay)
	{

		return 1;
	}
	else
	{
		return 0;
	}
}


