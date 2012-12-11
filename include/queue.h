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
/* $Id: queue.h,v 1.2 2003/02/25 23:45:58 mr Exp $ */

#ifndef INC_QUEUE_H
#define INC_QUEUE_H

#define COMMIT_TO_MYSQL 1
#define DONT_COMMIT 0

struct queue_list
{
  struct queue_list *next;
  char *command;
};

void *queue_write(void *arg);
int queue_init(void);
int queue_add(const char *str);
void queue_wait_until_empty(void);
char *queue_escape_string_buf(const char *str, char *buf);
char *queue_escape_string(const char *str);

#endif /* INC_QUEUE_H */
