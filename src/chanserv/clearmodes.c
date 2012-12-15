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
/* $Id: clearmodes.c,v 1.2 2003/02/25 22:41:36 cure Exp $ */

#include <stdlib.h>

#include "chanserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

/* declare the external irc socket */
extern sock_info *irc;

#define CHANSERV_CLEARMODES_OK        "All modes were cleared on %s."

/**************************************************************************************************
 * chanserv_clearmodes
 **************************************************************************************************
 *   CLEARMODES <#channel>
 *     Resets channelmodes to +nt, ie. removing limit, invite only, key, secret, private
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
FUNC_COMMAND(chanserv_clearmodes)
{
  chanserv_dbase_channel *ch;
  char *chan = getnext(params);
  if (!chan) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
  
  /* check if the channel is registered */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
  
  /* check to see if the channels is disabled */
  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED);
  
  if (chanserv_dbase_check_access(from->nickserv, ch, command_info->level))
  {
    dbase_channels *data = channels_getinfo(-1, chan);
    if (data)
    {
      while (data->bancount > 0)
      {
        com_send(irc, "%s M %s -b %s\n", conf->cs->numeric, chan, data->bans[--data->bancount]);
        xfree(data->bans[data->bancount]);
      }
      data->bans = (char **)realloc(data->bans, 0);
      if (data->key)
        com_send(irc, "%s M %s +nt-ispmlk %s\n", conf->cs->numeric, chan, data->key);
      else
        com_send(irc, "%s M %s +nt-ispml\n", conf->cs->numeric, chan);
      
      channels_setmode(-1, data->name, "+nt-ispmlk");
      
      log_command(LOG_CHANSERV, from, "CLEARMODES", queue_escape_string(chan));
            
      return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CLEARMODES_OK, chan);
    }
  }
  return ERROR_NO_ACCESS;
}

