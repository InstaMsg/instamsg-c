/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/


#include <sys/time.h>
#include <stdlib.h>

#include "include/time.h"


struct TimerObj {
        struct timeval end_time;
};
#define GET_IMPLEMENTATION_SPECIFIC_TIMER_OBJ(timer) ((struct TimerObj*)(timer->obj))

char expired(Timer* timer)
{
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&GET_IMPLEMENTATION_SPECIFIC_TIMER_OBJ(timer)->end_time, &now, &res);
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}


void countdown_ms(Timer* timer, unsigned int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
	timeradd(&now, &interval, &GET_IMPLEMENTATION_SPECIFIC_TIMER_OBJ(timer)->end_time);
}


void countdown(Timer* timer, unsigned int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout, 0};
	timeradd(&now, &interval, &GET_IMPLEMENTATION_SPECIFIC_TIMER_OBJ(timer)->end_time);
}


int left_ms(Timer* timer)
{
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&GET_IMPLEMENTATION_SPECIFIC_TIMER_OBJ(timer)->end_time, &now, &res);
	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}


void init_timer(Timer* timer)
{
	GET_IMPLEMENTATION_SPECIFIC_TIMER_OBJ(timer)->end_time = (struct timeval){0, 0};
}


Timer* get_new_timer()
{
	Timer *timer = (Timer*) malloc(sizeof(Timer));
	timer->obj = malloc(sizeof(struct TimerObj));

	timer->init_timer = init_timer;
	timer->expired = expired;
	timer->countdown_ms = countdown_ms;
	timer->countdown = countdown;
	timer->left_ms = left_ms;

}

void release_timer(Timer* timer)
{
	free(timer->obj);
	free(timer);
}
