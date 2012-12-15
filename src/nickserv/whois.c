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
/* $Id: whois.c,v 1.3 2003/02/26 12:29:48 mr Exp $ */

#include "nickserv.h"
#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "queue.h"
#include "log.h"

/**************************************************************************************************
 * nickserv_whois
 **************************************************************************************************
 *   WHOIS <nick> 
 *      Returns information aboit <nick>
 *       <nick> = getnext-string
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
FUNC_COMMAND(nickserv_whois)
{
  int i;
  dbase_nicks *info;
  char *nick = getnext(params);

  /* if no nick specified, do a whois on the user */
  if (!nick)
    nick = from->nick;

  /* Get info for the nick, if not found, return an error-message to the user */
  if (!(info = nicks_getinfo(NULL, nick, -1)))
    return com_message(sock, conf->ns->numeric, from->numeric, format, "User (%s) NOT found.");

  /* write the user@host for the requested nick */
  com_message(sock, conf->ns->numeric, from->numeric, format, "%s is %s!%s@%s :%s", info->nick, info->nick, info->username, info->host, info->userinfo);

  /* write weather the user is authed or not */
  if (!info->nickserv)
    com_message(sock, conf->ns->numeric, from->numeric, format, "%s is NOT authenticated.", info->nick);
  else
    com_message(sock, conf->ns->numeric, from->numeric, format, "%s is authenticated as %s.", info->nick, info->nickserv->nick);
  
  /* log the command */
  log_command(LOG_NICKSERV, from, "WHOIS", queue_escape_string(nick));

  if (!from->nickserv) return 0; 
  if (!operserv_have_access(from->nickserv->flags, BITS_OPERSERV_NS_OPER)) return 0;
  
  /* list all channel the user is in, even secret/private ones */
  com_message(sock, conf->ns->numeric, from->numeric, format, "%s is on the following channels:", info->nick);
  for (i = 0; i < info->channels_count; i++)
    com_message(sock, conf->ns->numeric, from->numeric, format, "  %s", info->channels[i]->channel->name);        

  return 0;
}
