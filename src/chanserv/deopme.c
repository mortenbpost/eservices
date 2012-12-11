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
/* $Id: deopme.c,v 1.2 2003/02/25 22:41:36 cure Exp $ */

#include "chanserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

/* declare the external irc socket */
extern sock_info *irc;

#define CHANSERV_DEOPME_OK            "You were deopped in %d channels."

/**************************************************************************************************
 * chanserv_deopme
 **************************************************************************************************
 *   DEOPME
 *     Set user -o on all channel, in which the uses has sufficient access
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
FUNC_COMMAND(chanserv_deopme)
{
  int i, nr, count = 0;
  dbase_channels *chan;

  for (i = 0; i < from->nickserv->access_count; i++)
  {
    if (chanserv_dbase_disabled(from->nickserv->access[i]->channel)) continue;
    if (from->nickserv->access[i]->level >= command_info->level)
    {
      chan = channels_getinfo(-1, from->nickserv->access[i]->channel->name);
      if (chan)
      {
        nr = channels_user_search(chan->users, 0, chan->usercount-1, from->numeric);
        /* If nr >= 0, id so, the user is on the channel */
        if (nr >= 0)
        {
          if (!(chan->users[nr]->mode & 2)) continue; /* not opped */
          com_send(irc, "%s M %s -o %s\n", conf->cs->numeric, from->nickserv->access[i]->channel->name, from->numeric);
          channels_usermode(-1, from->nickserv->access[i]->channel->name, "-o", from->numeric);
          count++;
        }
      }
    }
  }
  
  log_command(LOG_CHANSERV, from, "DEOPME", "");
  
  com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_DEOPME_OK, count);

  return 0;
}
