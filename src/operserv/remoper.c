/****************************************************************************
* Exiled.net IRC Services                                                   *
* Copyright (C) 2002-2003  Michael Rasmussen <the_real@nerdheaven.dk>       *
*                          Morten Post <cure@nerdheaven.dk>                 *
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
/* $Id: remoper.c,v 1.3 2003/03/01 16:47:11 cure Exp $ */

#include <stdio.h>
#include <string.h>

#include "operserv.h"
#include "nickserv.h"
#include "misc_func.h"
#include "config.h"
#include "log.h"
#include "errors.h"
#include "queue.h"

#define OPERSERV_REMOPER_OK       "Oper access succesfully removed from user: %s"

/**************************************************************************************************
 * operserv_remoper
 **************************************************************************************************
 *   REMOPER <registered nickname> FORCE
 *   Removes a user's oper access. 
 **************************************************************************************************
 * Params:
 *   [IN] sock_info *sock    : The socket from which the data was recieved
 *   [IN] dbase_nicks *from  : Pointer to the user who issued this command
 *   [IN] char **params      : The parameters to the command (to be used with getnext and getrest)
 *   [IN] char *format       : The format the message should be returned (privmsg or notice)
 *   [IN] parser_command_data: *command_info : struct containing syntax, access-level etc.
 * Return:
 *  [OUT] int return         : return 0 if success, anything else will cause the services to stop
 **************************************************************************************************/
FUNC_COMMAND(operserv_remoper)
{
  char *nick = getnext(params);
  char *forcestr = getrest(params);
  char buf[BUFFER_SIZE];
  nickserv_dbase_data *ns;
  
  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;
  if (!forcestr) return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);
  if (strcmp(forcestr, "FORCE")) return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);
    
  if (!(ns = nickserv_dbase_find_nick(nick))) 
    return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_USER_NOT_FOUND);
  
  ns->flags &= 0x0000fffe;
  
  snprintf(buf, BUFFER_SIZE, "UPDATE nickdata set flags='%lu' WHERE nick='%s'", ns->flags, queue_escape_string(ns->nick));
  queue_add(buf);
  
  log_command(LOG_OPERSERV, from, "REMOPER", "%s FORCE", queue_escape_string(nick));
  
  return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_REMOPER_OK, nick);
}
