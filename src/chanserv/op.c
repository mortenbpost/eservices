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
/* $Id: op.c,v 1.3 2003/03/01 16:47:04 cure Exp $ */

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

#define CHANSERV_OP_OK                "%d users were opped in %s."

/**************************************************************************************************
 * chanserv_op
 **************************************************************************************************
 *   OP <#channel> [nick {[nick},...]
 *     Set mode +o on [nick] in <#channel>. If [nick] not specified, the user is opped.
 *     <#channel> = getnext-string
 *     [nick]     = getnext-string
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
FUNC_COMMAND(chanserv_op)
{
  int count = 0;
  char *chan = getnext(params);
  char *nick = getnext(params);
  chanserv_dbase_channel *ch;
  char buf[BUFFER_SIZE], bufn[BUFFER_SIZE], bufc[BUFFER_SIZE];
  buf[0] = '\0'; bufn[0]= '\0'; bufc[0]= '\0';

  if (!chan) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

  /* check if the channel is registered */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
  
  /* check to see if the channels is disabled */
  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED);

  if (!chanserv_dbase_check_access(from->nickserv, ch, command_info->level)) return ERROR_NO_ACCESS;

  if (!nick) nick = from->nick;

  while (nick)
  {
    const char *num = nicks_getnum(nick);
    if (num)
    {
      int mode;
      if ((mode = channels_user_find(chan, num)) >= 0)
      {
        if (!(mode & 2))
        {
          snprintf(buf, BUFFER_SIZE, "%s %s", buf, num);
          snprintf(bufn, BUFFER_SIZE, "%s %s", bufn, nick);
          channels_usermode(-1, chan, "+o", num);
          bufc[count++] = 'o';
        }
      }
    }
    nick = getnext(params);
  }

  bufc[count] = '\0';
  if (count)
  {
    char chanbuf[BUFFER_SIZE];
    com_send(irc, "%s M %s +%s %s\n", conf->cs->numeric, chan, bufc, buf);
    strcpy(chanbuf, queue_escape_string(chan));
    log_command(LOG_CHANSERV, from, "OP", "%s %s", chanbuf, queue_escape_string(bufn));
  }
  
  chanserv_dbase_update_lastlogin(ch);

  return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_OP_OK, count, chan);
}
