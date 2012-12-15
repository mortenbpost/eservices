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
/* $Id: banlist.c,v 1.3 2003/03/01 16:47:04 cure Exp $ */

#include "chanserv.h"
#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define CHANSERV_BANLIST_EMPTY        "There are currently no active bans on %s."
#define CHANSERV_BANLIST_HEADER       "Banlist for %s:"

/**************************************************************************************************
 * chanserv_banlist
 **************************************************************************************************
 *   BANLIST <#channel>
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
FUNC_COMMAND(chanserv_banlist)
{
  int i;
  char *chan = getnext(params);
  chanserv_dbase_channel *ch;
  dbase_channels *info;
  
  if (!chan) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
  
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);

  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED);
    
  if (!chanserv_dbase_check_access(from->nickserv, ch, command_info->level) && !(operserv_have_access(from->nickserv->flags, BITS_OPERSERV_CS_OPER))) return ERROR_NO_ACCESS;
  
  if(!(info = channels_getinfo(-1, chan)))
    return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

  if (!ch->bancount && !info->bancount)
    return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_BANLIST_EMPTY, chan);
  
  com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_BANLIST_HEADER, chan);
  
  for (i = 0; i < ch->bancount; i++)
  {
    com_message(sock, conf->cs->numeric, from->numeric, format, "  %d) %s set by %s expiring: %s", i+1, ch->bans[i]->mask, ch->bans[i]->nick, gtime((time_t*)&ch->bans[i]->expire));
  }
  
  com_message(sock, conf->cs->numeric, from->numeric, format, "\nActive bans on %s:\n", chan);    
  
  for (i = 0; i < info->bancount; i++)
  {
    com_message(sock, conf->cs->numeric, from->numeric, format, "  %s", info->bans[i]);    
  }
  
  log_command(LOG_CHANSERV, from, "BANLIST", "%s", queue_escape_string(chan));
  return 0;
}

