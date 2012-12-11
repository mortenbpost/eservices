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
/* $Id: autoop.c,v 1.3 2003/03/01 16:47:04 cure Exp $ */

#include <stdio.h>
#include <string.h>

#include "chanserv.h"
#include "nickserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define CHANSERV_AUTOOP_OK            "%s autoop for %s in %s."
#define CHANSERV_AUTOOP_MUST_HAVE_OP  "%s must have access to OP command to enable autoop."

/**************************************************************************************************
 * chanserv_autoop
 **************************************************************************************************
 *   AUTOOP <#channel> <nick>
 *      toggle autoop for <nick> in <#channel>
 *      <nick> MUST have op-access in <#channel>
 *      <#channel> = getnext-string
 *      <nick>     = getnext-string
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
 FUNC_COMMAND(chanserv_autoop)
{
  chanserv_dbase_channel *ch;
  char *chan = getnext(params);
  char *nick = getnext(params);  
  char buf[BUFFER_SIZE], nbuf[BUFFER_SIZE];
  int lvl;
  nickserv_dbase_data *ns;
  chanserv_dbase_access *ac;
    /* If parameter not present, return syntax to user */
  if (!nick) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

  /* Is the specified channel regged ? */
  if (!chanserv_dbase_find_chan(chan)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);

  /* check if the channel is registered */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
  
  /* check to see if the channels is disabled */
  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED);

  /* Does the user have enough level on that channel to use this command */
  if (!chanserv_dbase_check_access(from->nickserv, ch, command_info->level)) return ERROR_NO_ACCESS;

  if (!(ns = nickserv_dbase_find_nick(nick))) return com_message(sock, conf->cs->numeric, from->numeric, format, NICKSERV_NOT_REGISTERED, nick);
  
  if (!chanserv_dbase_check_access(ns, ch, CHANSERV_LEVEL_OP)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_AUTOOP_MUST_HAVE_OP, nick);
  
  if (!(ac = chanserv_dbase_find_access(ns, ch))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_AUTOOP_MUST_HAVE_OP, nick);
  
  ac->autoop = !ac->autoop;

  lvl = ac->level | (ac->autoop << 16);
  strcpy(nbuf, queue_escape_string(ac->nick->nick));
  snprintf(buf, BUFFER_SIZE, "UPDATE access SET level='%d' WHERE nick='%s' AND channel='%s'", lvl, nbuf, queue_escape_string(ac->channel->name));
  queue_add(buf);
  
  strcpy(buf, queue_escape_string(chan));
  log_command(LOG_CHANSERV, from, "AUTOOP", "%s %s [%s]", buf, queue_escape_string(nick), (ac->autoop?"SET":"REMOVED"));
  
  chanserv_dbase_update_lastlogin(ch);

  return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_AUTOOP_OK, (ac->autoop?"Activated":"Deactivated"), nick, chan);
}
