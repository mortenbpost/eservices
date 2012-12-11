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
/* $Id: showcommands.c,v 1.2 2003/02/25 22:41:36 cure Exp $ */

#include "chanserv.h"
#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

extern struct parser_command_data parser_chanserv_commands[];

/**************************************************************************************************
 * chanserv_showcommands
 **************************************************************************************************
 *   SHOWCOMMANDS <#channel>
 *     Returns a list of commands the user has access to in <#channel>
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
FUNC_COMMAND(chanserv_showcommands)
{
  int level, i;
  char *chan = getnext(params);
  chanserv_dbase_channel *ch;

  /* If no channel given, display syntax */
  if (!chan) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

  /* check if the channel is registered */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
  
  /* check to see if the channels is disabled */
  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED);
  
  /*if (!(level = chanserv_dbase_check_access(from->nickserv, ch, CHANSERV_LEVEL_INVITE))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_NO_ACCESS_ON_CHAN);*/
  level = chanserv_dbase_check_access(from->nickserv, ch, 0);

  com_message(sock, conf->cs->numeric, from->numeric, format, "Commands available on %s at level %d:", chan, level);
  
  i = 0;
  while (parser_chanserv_commands[i].command)
  {
    if (level >= parser_chanserv_commands[i].level)
      com_message(sock, conf->cs->numeric, from->numeric, format, "       %3d: %-15s %s", parser_chanserv_commands[i].level, parser_chanserv_commands[i].command, parser_chanserv_commands[i].syntax);
    else if ((operserv_have_access(from->nickserv->flags, parser_chanserv_commands[i].flags)) && (parser_chanserv_commands[i].flags))
    {
      char flagsbuf[10];
      operserv_flags_to_str(parser_chanserv_commands[i].flags, flagsbuf);
      com_message(sock, conf->cs->numeric, from->numeric, format, "   OPER +%s: %-15s %s", flagsbuf, parser_chanserv_commands[i].command, parser_chanserv_commands[i].syntax);
    }
    i++;
  }

  log_command(LOG_CHANSERV, from, "SHOWCOMMANDS", queue_escape_string(chan));

  return 0;
}
