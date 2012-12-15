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
/* $Id: errors.c,v 1.2 2003/01/12 15:30:26 cure Exp $ */

#include <stdlib.h>

#include "setup.h"
#include "errors.h"

struct err_struct err_messages[] =
{
  {ERROR_COM_QUIT,                    "Service has quit"},
  {ERROR_COM_CONNECTION_LOST,         "Lost connection to server"},
  {ERROR_LOG_NOT_DEFINED,             "Info about log-filenames not found in config"},
  {ERROR_LOG_ERROR_OPENING_FILE,      "Error opening log file"},
  {ERROR_DBASE_MYSQL_ERROR,           "mySQL error"},
  {ERROR_SERVER_COULD_NOT_CONNECT,    "Error connecting to UPLINK"},
  {ERROR_OPERSERV_DIE,                "OperServ DIE command"},
  {-1,                           NULL}
};


char *error_getstring(int errnr)
{
  struct err_struct *lst = err_messages;
  
  while (lst->errnr >= 0)
  {
    if (lst->errnr == errnr) return lst->errstr;
    lst++;
  }
  return NULL;
}
