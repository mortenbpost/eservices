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
/* $Id: timer.c,v 1.1.1.1 2002/08/20 15:20:04 mr Exp $ */

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "misc_func.h"
#include "timer.h"
#include "config.h"
#include "server.h"

extern sock_info *irc;

timer_event *timer_root = NULL;

void timer_check(void)
{
  timer_event *tmp = timer_root;
  unsigned long now = (unsigned long)time(0);
  
  while (tmp)
  {
    if (now < tmp->when) return;

    if (tmp->func) tmp->func(tmp->data);

    timer_root = tmp->next;

    timer_free(tmp);

    tmp = timer_root;
  }
}

int timer_add(timer_event *event, unsigned long seconds)
{
  unsigned long when = time(0) + seconds;
  
  if (seconds == 0) when = 0;
  
  if (!event) return 0;
  if (seconds < 0) return 0;
  
  event->prev = NULL;
  event->next = NULL;

  if (!timer_root)
  {
    timer_root = event;
  }
  else if (timer_root->when <= when)
  {
    timer_event *tmp = timer_root;

    while (tmp->when <= when)
    {
      if (tmp->when == when) when++;

      if (!tmp->next) break;
      if (tmp->next->when > when) break;
      
      tmp = tmp->next;
    }
    
    event->prev = tmp;
    event->next = tmp->next;
    tmp->next = event;    
  }
  else
  {
    event->next = timer_root;
    timer_root->prev = event;
    timer_root = event;
  }
  
  event->when = when;
  
  return when;
}

int timer_cancel(unsigned long when)
{
  timer_event *tmp = timer_root;

  if (when == 0) return 1;

  while (tmp)
  {
    if (tmp->when == when)
    {
      if (!tmp->prev) timer_root = tmp->next;
      else tmp->prev->next = tmp->next;

      if (tmp->next) tmp->next->prev = tmp->prev;

      timer_free(tmp);

      return 0;      
    }
    tmp = tmp->next;
  }

  return 1;
}

void timer_free(timer_event *event)
{
  if (event)
  {
    if (event->free) event->free(event->data);
    xfree(event);
  }
}

void timer_freeall(void)
{
  timer_event *tmp = timer_root;
  while (tmp)
  {
    timer_root = tmp;
    tmp = tmp->next;
    
    timer_free(timer_root);
  }
  timer_root = NULL;
}
