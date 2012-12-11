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
/* $Id: register.c,v 1.2 2003/02/25 22:41:36 cure Exp $ */


#include "chanserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define CHANSERV_REGISTER_ALREADY     "%s is an already registered channel."
#define CHANSERV_REGISTER_MUST_BE_OP  "You must have op on the channel you want to register."

/**************************************************************************************************
 * chanserv_register
 **************************************************************************************************
 *   REGISTER <#channel>
 *     Register <#channel> in services, setting user as owner and giving user level 500
 *     <#channel> = getnext-string
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
FUNC_COMMAND(chanserv_register)
{
  char *chan = getnext(params);
  long nr;
  chanserv_dbase_channel *data;
  dbase_channels *channel;


  if (!chan) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

  data = chanserv_dbase_find_chan(chan);
  if (data)
    return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_REGISTER_ALREADY, chan);

  if (!(channel = channels_getinfo(-1, chan)))
    return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_REGISTER_MUST_BE_OP);

  if ((nr = channels_user_search(channel->users, 0, channel->usercount -1, from->numeric)) < 0)
    return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_REGISTER_MUST_BE_OP);

  if (!(channel->users[nr]->mode & 2))
    return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_REGISTER_MUST_BE_OP);

  data = chanserv_dbase_create(chan, from);

  if (!data) return com_message(sock, conf->cs->numeric, from->numeric, format, "OH MY FUCKING GOD. PLEASE REPORT THIS TO AN OPER!");
  
  log_command(LOG_CHANSERV, from, "REGISTER", queue_escape_string(chan));

  chanserv_dbase_join(data, 0, "");
  
  return 0;
}
