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
/* $Id: unban.c,v 1.3 2003/03/01 16:47:05 cure Exp $ */

#include <string.h>

#include "chanserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

/* declare the external irc socket */
extern sock_info *irc;

#define CHANSERV_BAN_REMOVED          "Ban on %s for %s expiring on %s was removed."
#define CHANSERV_UNBAN_NO_SUCH_BAN_ID "%d is not a valid ban id."

/**************************************************************************************************
 * chanserv_unban
 **************************************************************************************************
 *   UNBAN <#channel> <id>
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
FUNC_COMMAND(chanserv_unban)
{
  char mask[BUFFER_SIZE];
  char *chan = getnext(params);
  int id = tr_atoi(getnext(params));
  chanserv_dbase_channel *ch;
 
  /* checking syntax */
  if (!chan) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
  if (!id && id <= 0) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

  /* checking if channel exists */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);

  /* checking if the channel is disabled */
  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED); 

  /* checking if the user has enough access */
  if (!chanserv_dbase_check_access(from->nickserv, ch, command_info->level))
    return ERROR_NO_ACCESS;

  id -= 1;

  /* checks if it's a valid ban id */
  if (id >= ch->bancount) 
    return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_UNBAN_NO_SUCH_BAN_ID, id);  

  com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_BAN_REMOVED, chan, ch->bans[id]->mask, gmtime((time_t*)&ch->bans[id]->expire)); 

  /* we better log this */
  strcpy(mask, queue_escape_string(ch->bans[id]->mask));
  log_command(LOG_CHANSERV, from, "UNBAN", "%s %s", queue_escape_string(chan), mask);

  /* removing ban from mysql */
  chanserv_dbase_remove_enforce_ban(ch, id, 0, COMMIT_TO_MYSQL);   

  /* removing ban from internal database and from channel */
  com_send(irc, "%s M %s -b %s\n", conf->cs->numeric, chan, ch->bans[id]->mask);
  channels_remban(-1, chan, ch->bans[id]->mask);

  return 0;
}
