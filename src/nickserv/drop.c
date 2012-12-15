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
/* $Id: drop.c,v 1.2 2003/02/25 22:41:17 cure Exp $ */

#include "nickserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define NICKSERV_DROP_WRONG                 "Incorrect password."
#define NICKSERV_DROP_OK                    "Nickname %s has been removed successfully."
#define NICKSERV_DROP_NO_SUCH_USER          "%s is not a registered nickname."

/**************************************************************************************************
 * nickserv_drop
 **************************************************************************************************
 *   DROP <password>
 *      +R
 *      Drops the account the user is currently identified as. 
 *      <password> MUST match the accounts password.
 *       <password> = getnext-string
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
FUNC_COMMAND(nickserv_drop)
{
  char *password = getnext(params);

  /* if user not authed, return error */
  if (!from->nickserv) return ERROR_NO_ACCESS;
    
  /* is password specified, else return syntax */
  if (!password) return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);

  /* is the password correct, if not return error-message */
  if (nickserv_dbase_validate_password(from->nickserv->nick, password, from))
    return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_DROP_WRONG);
  
  /* Inform the user that the nick is being dropped */
  com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_DROP_OK, from->nickserv->nick);

  /* log the command */
  log_command(LOG_NICKSERV, from, "DROP", "[hidden]");

  /* remove the nick from the database */
  nickserv_dbase_unreg(from->nickserv);
  
  /* update the from-struct so the nick is no longer authed */
  from->nickserv = NULL;
  return 0;
}
