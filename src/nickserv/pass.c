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
/* $Id: pass.c,v 1.3 2003/03/01 16:47:08 cure Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nickserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define NICKSERV_PASS_OK                    "Your password has been changed to '%s'. Please remember it."
#define NICKSERV_PASS_WRONG                 "Old password incorrect."

/**************************************************************************************************
 * nickserv_pass
 **************************************************************************************************
 *   PASS <old password> <new password>
 *      +R
 *      Changes the password for the users account.
 *      <old password> is the current password, and msut match to succesfully change.
 *      <new password> is the password the account password should be changed to.
 *       <old password> = getnext-string
 *       <new password> = getnext-string
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
FUNC_COMMAND(nickserv_pass)
{
  char *oldpass = getnext(params);
  char *newpass = getnext(params);

  /* is the user authed */
  if (!from->nickserv) return ERROR_NO_ACCESS;

  /* enough parameters ? */
  if ((!oldpass) || (!newpass)) return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);

  /* is the old password the correct password */
  if (nickserv_dbase_validate_password(from->nickserv->nick, oldpass, from))
    return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_PASS_WRONG);
  else
  {
    /* crypt the new password, save it and inform the user that the password is changed */
    char buf[BUFFER_SIZE];
    const char *pwd = ircd_crypt(newpass, from->host);
    from->nickserv->password = (char *) realloc(from->nickserv->password, strlen(pwd) +1);
    strcpy(from->nickserv->password, pwd);
    snprintf(buf, BUFFER_SIZE, "UPDATE nickdata SET password='%s' WHERE nick='%s'", pwd, queue_escape_string(from->nickserv->nick));
    queue_add(buf);
    com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_PASS_OK, newpass);
  }
  
  /* log the command */
  log_command(LOG_NICKSERV, from, "PASS", "[hidden] [hidden]");
  return 0;
}
