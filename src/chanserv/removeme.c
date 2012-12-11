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
/* $Id: removeme.c,v 1.3 2003/03/01 16:47:04 cure Exp $ */

#include <string.h>

#include "chanserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define CHANSERV_REMOVEME_OK          "Your access in %s has been removed."
#define CHANSERV_REMOVEME_OWNER       "You cannot remove your access from a channel you own."

/**************************************************************************************************
 * chanserv_removeme
 **************************************************************************************************
 *   REMOVEME <#channel> <password>
 *      Removes yourself from the channel's access list
 *      <#channel> = getnext-string
 *      <password> = getrest-string
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
FUNC_COMMAND(chanserv_removeme)
{
  chanserv_dbase_channel *ch;
  char *chan  = getnext(params);
  char *pwd   = getrest(params);

  /* displays syntax if chan or topic is not given */
  if (!pwd)
    return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

  /* check if the channel is registered */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
  
  /* verify that the user has sufficient access in that channel to set topic */
  if (!chanserv_dbase_check_access(from->nickserv, ch, command_info->level)) return ERROR_NO_ACCESS;
  
  /* Is the user the owner of the channel, if yes, refuse to remove */
  if (chanserv_dbase_check_access(from->nickserv, ch, CHANSERV_LEVEL_OWNER)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_REMOVEME_OWNER);
  
  /* Was the correct password specified */
  if (strcmp(from->nickserv->password, ircd_crypt(pwd, from->nickserv->password))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_WRONG_PASSWORD);
  
  /* Remove the acces */
  chanserv_dbase_access_delete(ch, from->nickserv, COMMIT_TO_MYSQL);
  
  /* Log the chanserv command */
  log_command(LOG_CHANSERV, from, "REMOVEME", "%s [hidden]", queue_escape_string(chan));

  return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_REMOVEME_OK, chan);
}
