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
/* $Id: auth.c,v 1.4 2003/03/01 16:47:08 cure Exp $ */

#include "nickserv.h"
#include "misc_func.h"
#include "config.h"
#include "queue.h"
#include "log.h"

#define NICKSERV_AUTH_OK                    "Authentication successful for %s.\n"\
                                            "Last login was from: %s on %s"

#define NICKSERV_AUTH_ALREADY_AUTH          "Someone has already authenticated to this nickname. To release the nickname use the GHOST command."

/**************************************************************************************************
 * nickserv_auth
 **************************************************************************************************
 *   AUTH [username] <password>
 *
 *   Authorizes to nickserv for [username] or the user's current nick if [username] is omitted.
 *       [username] = getnext-string
 *       <password> = getnext-string
 *   
 *   Alias: IDENTIFY
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
FUNC_COMMAND(nickserv_auth)
{
  char *userna = getnext(params);
  char *passwd = getnext(params);
  nickserv_dbase_data *ns;

  /* enough parameters */
  if (!userna)
    return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);

  /* if only 1 parameter, get username from nick in the from struct */
  if (!passwd)
  {
    passwd = userna;
    userna = from->nick;
  }
  
  /* Already authed */
  if (from->nickserv) return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_ALREADY_AUTHED, from->nickserv->nick);
  
  /* Is this a regged nick ? */
  if (!(ns = nickserv_dbase_find_nick(userna)))
    return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_NOT_REGISTERED, userna);
  
  /* Has someone already authed for this nick ? */
  if (ns->entry)
     return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_AUTH_ALREADY_AUTH, ns->nick);

  /* Is the password correct */
  if (nickserv_dbase_validate_password(userna, passwd, from))
  {
    /* Inform user about incorrect password, and log the auth-failure */
    com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_WRONG_PASSWORD);
    log_command(LOG_NICKSERV, from, "AUTH", "%s [password] - FAILED", queue_escape_string(userna));
  }
  else
  {
    /* Inform user about correct login, update last_login date in the database,
       op the user in alle the channels it is currently in and has autoop in
       and log the command */
    com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_AUTH_OK, userna, ns->userhost, gtime((time_t*)&ns->lastlogin));
    nickserv_dbase_update(from);
    nickserv_dbase_op_on_auth(from);
    nickserv_dbase_notice_check(from);
    log_command(LOG_NICKSERV, from, "AUTH", "%s [password] - SUCCESS", queue_escape_string(userna));
  }
  return 0;
}

