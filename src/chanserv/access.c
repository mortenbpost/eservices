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
/* $Id: access.c,v 1.4 2003/03/01 16:47:04 cure Exp $ */

#include <stdio.h>
#include <string.h>

#include "chanserv.h"
#include "nickserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define CHANSERV_ACCESS_LIST_HEAD      "Access list for %s:\n"
#define CHANSERV_ACCESS_LIST_ENTRY     "  %c%3d %s"
#define CHANSERV_ACCESS_OVER_OWN       "%s has either equal or higher access than yourself, and cannot be modified."
#define CHANSERV_ACCESS_CHANGED        "%s's access in %s is now set to %d."
#define CHANSERV_ACCESS_USER_DEL       "%s's access was removed from %s."
#define CHANSERV_ACCESS_USER_NOT_FOUND "%s is not a registered nickname and cannot be given access."
#define CHANSERV_ACCESS_USER_ADDED     "%s successfully added to access list on %s with level %d."

/**************************************************************************************************
 * chanserv_access
 **************************************************************************************************
 *   ACCESS <#channel> ADD <rnick> <level>
 *     Gives <rnick> access in <#channel> (sets level to <level> (1 to 500))
 *
 *   ACCESS <#channel> REM <rnick>
 *     Removes <rnick>'s access in <#channel>
 *
 *   ACCESS <#channel> LIST
 *     Returns a list of all users who has access in <#channel>
 *
 *     <#channel> = getnext-string
 *     <rnick>    = getnext-string
 *     <level>    = getnext-string
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
FUNC_COMMAND(chanserv_access)
{
  chanserv_dbase_channel *ch;
  chanserv_dbase_access *data;
  char *chan = getnext(params);
  char *cmd = getnext(params);
  char *nick = getnext(params);
  int level = tr_atoi(getnext(params));
  char safe[BUFFER_SIZE];

  if (!(chan)) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

  /* check if the channel is registered */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
  
  /* check to see if the channels is disabled */
  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED);
  
  if (!(data = chanserv_dbase_find_access(from->nickserv, ch))) return ERROR_NO_ACCESS;

  if (data->level < command_info->level) return ERROR_NO_ACCESS;
  
  cmd = uppercase(cmd);

  strcpy(safe, queue_escape_string(chan));

  if ((!cmd) || (!strcmp(cmd, "LIST")))
  {
    int i;
    com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_ACCESS_LIST_HEAD, chan);
    for (i = 0; i < ch->access_count; i++)
      com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_ACCESS_LIST_ENTRY, ch->access[i]->autoop?'@':' ', ch->access[i]->level, ch->access[i]->nick->nick);
    log_command(LOG_CHANSERV, from, "ACCESS", "%s LIST", safe);
    return 0;
  }
  else if (!strcmp(cmd, "REM"))
  {
    chanserv_dbase_access *acc;
    if (!nick) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
    if ((acc = chanserv_dbase_has_access(nick, ch)))
    {
      nickserv_dbase_data *nick_info = acc->nick;
      if (acc->level >= data->level)
        return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_ACCESS_OVER_OWN, nick);
      if (!chanserv_dbase_access_delete(data->channel, acc->nick, COMMIT_TO_MYSQL))
      {
        log_command(LOG_CHANSERV, from, "ACCESS", "%s REM %s", safe, queue_escape_string(nick));
        nickserv_dbase_notice(nick_info, "Your access on %s has been removed by %s.", chan, from->nickserv->nick);
        return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_ACCESS_USER_DEL, nick, chan);
      }
    }
    return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_ACCESS_USER_NOT_FOUND, nick);
  }
  else if ((!strcmp(cmd, "SET")) || (!strcmp(cmd, "ADD")))
  {
    chanserv_dbase_access *acc;
    nickserv_dbase_data *nick_info;
    if (!nick) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
    if (level <= 0) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
    if (level >= data->level) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_ACCESS_OVER_OWN, nick);
    if ((acc = chanserv_dbase_has_access(nick, ch)))
    {
      if (acc->level >= data->level)
        return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_ACCESS_OVER_OWN, nick);
      else
      {
        int lvl;
        char buf[BUFFER_SIZE], nbuf[BUFFER_SIZE];
        acc->level = level;
        lvl = level | (acc->autoop << 16);
        strcpy(nbuf, queue_escape_string(acc->nick->nick));
        snprintf(buf, BUFFER_SIZE, "UPDATE access SET level='%d' WHERE nick='%s' AND channel='%s'", lvl, nbuf, queue_escape_string(acc->channel->name));
        queue_add(buf);

        log_command(LOG_CHANSERV, from, "ACCESS", "%s SET %s %d", safe, queue_escape_string(nick), level);
        nickserv_dbase_notice(acc->nick, "Your access on %s has been changed to %d by %s.", chan, lvl, from->nickserv->nick);
        return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_ACCESS_CHANGED, nick, chan, lvl);
      }
    }
    if ((nick_info = nickserv_dbase_find_nick(nick)))
    {
      if (chanserv_dbase_access_add(data->channel, nick_info, level, 0, COMMIT_TO_MYSQL))
        return com_message(sock, conf->cs->numeric, from->numeric, format, "Bug! Please report the following code to an oper: %s:%d", __FILE__, __LINE__);
      log_command(LOG_CHANSERV, from, "ACCESS", "%s ADD %s %d", safe, queue_escape_string(nick), level);
      nickserv_dbase_notice(nick_info, "You have been granted access on %s with level %d by %s.", chan, level, from->nickserv->nick);
      return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_ACCESS_USER_ADDED, nick_info->nick, chan, level);
    }
    return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_ACCESS_USER_NOT_FOUND, nick);
  }
  return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
}
