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
/* $Id: drop.c,v 1.3 2003/03/01 16:47:04 cure Exp $ */

#include <string.h>

#include "chanserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define CHANSERV_DROP_OK              "%s was succesfully dropped."

/**************************************************************************************************
 * chanserv_drop
 **************************************************************************************************
 *   DROP <#channel> <password>
 *     Drops the registred channel <#channel>.
 *     Must be validated, ie. <password> MUST be equal to owner's NickServ-password.
 *     <#channel> = getnext-string
 *     <password> = getnext-string
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
FUNC_COMMAND(chanserv_drop)
{
  chanserv_dbase_channel *ch;
  chanserv_dbase_access *tmp;
  char *chan = getnext(params);
  char *pass = getnext(params);

  /* enough parameters? */
  if ((!chan) | (!pass)) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
  
  /* check if the channel is registered */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
  
  /* check to see if the channels is disabled */
  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED);
  
  /* Is the channel regged */
  if ((tmp = chanserv_dbase_find_access(from->nickserv, ch)))
  {
    /* Does the user have sufficient access to drop the channel */
    if (tmp->level >= CHANSERV_LEVEL_OWNER)
    {
      /* Did the user specify the correct password */
      if (!strcmp(from->nickserv->password, ircd_crypt(pass, from->nickserv->password)))
      {
        /* unregister the channel */
        chanserv_dbase_delete(tmp->channel);
                            
        /* log the command */
        log_command(LOG_CHANSERV, from, "DROP", "%s [hidden]", queue_escape_string(chan));
        
        /* return message that channel was succesfully dropped */
        return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_DROP_OK, chan);
      }
      /* Error about wrong password */
      return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_WRONG_PASSWORD);
    }
  }
  /* Error about insuficient level */
  return ERROR_NO_ACCESS;
}
