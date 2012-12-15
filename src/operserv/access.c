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
/* $Id: access.c,v 1.2 2003/02/25 23:15:03 cure Exp $ */

#include <stdio.h>

#include "operserv.h"
#include "nickserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define OPERSERV_ACCESS_SET       "Access successfully changed, %s access is now %s"
#define OPERSERV_ACCESS_ERROR     "Error in access mode string."
#define OPERSERV_ACCESS_FLAGS     "Flags for %s: %s"
#define OPERSERV_ACCESS_NOT_OPER  "Can't give access to a non-oper."
#define OPERSERV_ACCESS_HIGHER    "You can't modify a user with higher access than yourself."

/**************************************************************************************************
 * operserv_access
 **************************************************************************************************
 *   ACCESS <nick> [+access | -access] 
 *   Modifies a user's oper access. 
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
FUNC_COMMAND(operserv_access)
{
  char buf[BUFFER_SIZE];
  unsigned long f = 0;
  char *who = getnext(params);
  char *flags = getrest(params);
  nickserv_dbase_data *data;

  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;

  if (!who) return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);
  if (!(data = nickserv_dbase_find_nick(who))) return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_USER_NOT_FOUND);
  if (!(data->flags & BITS_OPERSERV_OPER)) return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_ACCESS_NOT_OPER);

  if (!flags)
    return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_ACCESS_FLAGS, who, operserv_flags_to_str(data->flags, buf));

  if (data == from->nickserv) ; /* editing self - should something be done ? */
    
  f = operserv_str_to_flags(flags, data->flags, from->nickserv->flags);
    
  if (f == 0xffffffff)
    return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_ACCESS_HIGHER);
        
  data->flags = f;
  snprintf(buf, BUFFER_SIZE, "UPDATE nickdata SET flags=%lu WHERE nick='%s'", data->flags, data->nick);
  queue_add(buf);
        
  log_command(LOG_OPERSERV, from, "ACCESS", "%s %s", data->nick, queue_escape_string(flags));

  return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_ACCESS_SET, who, operserv_flags_to_str(data->flags, buf));
}
