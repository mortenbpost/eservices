/****************************************************************************
* Exiled.net IRC Services                                                   *
* Copyright (C) 2002  Michael Rasmussen <the_real@nerdheaven.dk>            *
*                     Morten Post <cure@nerdheaven.dk>                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License for more details.                              *
*                                                                           *
* You should have received a copy of the GNU General Public License         *
* along with this program; if not, write to the Free Software               *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
*****************************************************************************/
/* $Id: timer.h,v 1.1.1.1 2002/08/20 15:20:02 mr Exp $ */

#ifndef INC_TIMER_H
#define INC_TIMER_H

typedef void (*timer_function)(void *ptr);

typedef enum timer_type
{
  TIMER_PRINTF
} timer_type;

typedef struct timer_event timer_event;

struct timer_event
{
  timer_event *next;
  timer_event *prev;
  
  void *data;
  timer_function func;
  timer_function free;

  unsigned long when;
};

int  timer_add(timer_event *event, unsigned long seconds);
int  timer_cancel(unsigned long when);

void timer_free(timer_event *event);
void timer_freeall(void);

void timer_check(void);

#endif /* INC_TIMER_H */
