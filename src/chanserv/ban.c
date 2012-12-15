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
/* $Id: ban.c,v 1.2 2003/02/25 22:41:36 cure Exp $ */

#include <stdio.h>
#include <string.h>

#include "chanserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

/* declare the external irc socket */
extern sock_info *irc;

#define CHANSERV_BAN_INVALID_PERIOD   "Invalid period. See HELP BAN for more info."
#define CHANSERV_BAN_COVERED          "There is already a ban matching that mask."
#define CHANSERV_BAN_SET              "Ban set on %s for %s"
#define CHANSERV_BAN_REMOVED          "Ban on %s for %s expiring on %s was removed."

/**************************************************************************************************
 * chanserv_ban
 **************************************************************************************************
 *   BAN <#channel> <nick|userhost>
 *   BAN #chan nick   (=> find user@host for nick, and ban *!user@host)
 *   BAN #chan nick!user@host
 *   BAN #chan nick time ( lav en str-to-sec
 *      Changes the topic of <#channel>, sets it to <topic>
 *      <#channel> = getnext-string
 *      <topic>    = getrest-string
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
FUNC_COMMAND(chanserv_ban)
{
  char *chan = getnext(params);
  char *mask = getnext(params);
  char *per  = getnext(params);
  char banmask[BUFFER_SIZE], buf[BUFFER_SIZE];
  long period = 0;
  chanserv_dbase_channel *ch;
  
  if (!mask) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
    
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);

  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED);
    
  if (!chanserv_dbase_check_access(from->nickserv, ch, command_info->level)) return ERROR_NO_ACCESS;

  if (per) period = time_string_to_int(per);

  if (period == -1) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_BAN_INVALID_PERIOD);
  
  if ((strchr(mask, '!')) && (strchr(mask, '@'))) /* valid mask - hopefully */
  {
    strcpy(banmask, mask);
  }
  else if ((!strchr(mask, '!')) && (!strchr(mask, '@'))) 
  {
    /* nick */
    dbase_nicks *nick = nicks_getinfo(NULL, mask, -1);
    if (!nick) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
      
    snprintf(banmask, BUFFER_SIZE, "*!*%s@%s", nick->username, nick->host);
  }
  else
    return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
  
  if (channels_check_ban_covered(-1, ch->name, banmask))
    return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_BAN_COVERED);
  
  if (period)
  {
    chanserv_dbase_add_enforce_ban(ch, banmask, period, from->nickserv->nick, COMMIT_TO_MYSQL);
  }

  com_send(irc, "%s M %s +b %s\n", conf->cs->numeric, ch->name, banmask);
  channels_addban(-1, ch->name, banmask);

  /* Log the chanserv command */
  strcpy(buf, queue_escape_string(banmask));
  log_command(LOG_CHANSERV, from, "BAN", "%s %s %lu", queue_escape_string(ch->name), buf, period);

  /* inform the user that topic has been changed */
  return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_BAN_SET, chan, mask);
}
