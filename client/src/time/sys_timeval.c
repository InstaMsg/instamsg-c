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
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/


#include <sys/time.h>
#include <stdlib.h>

#include "include/time.h"


struct TimerObj {
        struct timeval end_time;
};
#define GET_IMPLEMENTATION_SPECIFIC_TIMER_OBJ(timer) ((struct TimerObj*)(timer->obj))


Timer* get_new_timer()
{
	Timer *timer = (Timer*) malloc(sizeof(Timer));
	timer->obj = malloc(sizeof(struct TimerObj));
}

void release_timer(Timer* timer)
{
    if(timer != NULL)
    {
        if(timer->obj != NULL)
        {
	        free(timer->obj);
        }

	    free(timer);
    }
}
