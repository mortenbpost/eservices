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
/* $Id: die.c,v 1.3 2003/03/01 16:47:11 cure Exp $ */

#include <string.h>

#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

extern sock_info *irc;

#define OPERSERV_DIE_BAD_CONFIRM  "The specified confirm string did not match."

/**************************************************************************************************
 * operserv_die
 **************************************************************************************************
 *   DIE <confirm string> <reason> 
 *   Shutdown the services. 
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
FUNC_COMMAND(operserv_die)
{
  char *confirm = getnext(params);
  char *reason  = getrest(params);

  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;

  if ((!reason) || (!confirm)) return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);
    
  if (strcmp(confirm, conf->host)) return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_DIE_BAD_CONFIRM);

  log_command(LOG_SERVICES, NULL, "", "%s (%s@%s) told me to DIE with reason: %s", from->nick, from->username, from->host, reason);
    
  com_send(irc, "%s Q :Service shutdown\n", conf->cs->numeric);
  com_wallops(conf->os->numeric, "Service shutdown issued by %s: %s\n", from->nick, reason);
  com_send(irc, "%s SQ %s %lu :Service shutdown issued by %s\n", conf->os->numeric, conf->host, conf->starttime, from->nick);
    
  log_command(LOG_OPERSERV, from, "DIE", "[hidden] %s", queue_escape_string(reason));

  return ERROR_OPERSERV_DIE;
}
