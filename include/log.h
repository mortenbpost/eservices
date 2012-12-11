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
/* $Id: log.h,v 1.1.1.1 2002/08/20 15:20:02 mr Exp $ */

#ifndef INC_LOG_H
#define INC_LOG_H

#include "dbase.h"

typedef enum log_type
{
  LOG_SERVICES,
  LOG_NICKSERV,
  LOG_CHANSERV,
  LOG_OPERSERV,
  LOG_MULTISERV
} log_type;

int log_open(void);
void log_close(void);
void log_write(const char *str, ...);
void log_command(log_type serv, dbase_nicks *from, const char *command, const char *param, ...);

#endif /* INC_LOG_H */
