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
/* $Id: op.c,v 1.2 2003/02/25 23:15:04 cure Exp $ */


#include "errors.h"
#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "queue.h"
#include "log.h"

extern sock_info *irc;

#define OPERSERV_MODE_USER_NOT_ON "Can't set mode for %s, user is not on the channel."

/**************************************************************************************************
 * operserv_op
 **************************************************************************************************
 *   OP <#channel>
 *   Ops you on a channel.
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
FUNC_COMMAND(operserv_op)
{
  dbase_channels *info;
  char *chan = getnext(params);

  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;
    
  if (!chan) return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);
  if (!(info = channels_getinfo(-1, chan)))
    return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);

  if (channels_usermode(-1, chan, "+o", from->numeric) < 0)
    return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_MODE_USER_NOT_ON, from->nick);
  
  com_wallops(conf->os->numeric, "%s requested mode: %s +o %s\n", from->nick, chan, from->nick);
  log_command(LOG_OPERSERV, from, "OP", queue_escape_string(chan));
  if (info->chanserv)
    return com_send(irc, "%s M %s +o %s\n", conf->cs->numeric, chan, from->numeric);
  return com_send(irc, "%s M %s +o %s\n", conf->numeric, chan, from->numeric);     
}
