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
/* $Id: ungline.c,v 1.2 2003/02/25 23:15:04 cure Exp $ */


#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

extern sock_info *irc;

#define OPERSERV_UNGLINE_OK       "Trying to ungline %s"

/**************************************************************************************************
 * operserv_ungline
 **************************************************************************************************
 *   UNGLINE <user@host> 
 *   Removes a network wide ban. 
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
FUNC_COMMAND(operserv_ungline)
{
  char *userhost = getnext(params);

  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;

  if (!userhost) return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);
  if (!operserv_valid_gline(userhost)) return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_INVALID_GLINE);

  com_send(irc, "%s GL * -%s\n", conf->numeric, userhost);
  com_wallops(conf->os->numeric, "%s has asked to remove a G:Line on %s\n", from->nick, userhost);
     
  log_command(LOG_OPERSERV, from, "UNGLINE", queue_escape_string(userhost));
      
  return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_UNGLINE_OK, userhost);
}
