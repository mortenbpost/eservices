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
/* $Id: chpass.c,v 1.3 2003/03/01 16:47:08 cure Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nickserv.h"
#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define NICKSERV_CHPASS_OK                  "Password was successfully changed for: %s to \"%s\".\n"\
                                            "You MUST notify the user immediately."

/**************************************************************************************************
 * nickserv_chpass
 **************************************************************************************************
 *   CHPASS <registered nickname> <password>
 *   +R +O +N
 *     Force-changes the password for <rnick>, setting password to <password>
 *      <rnick>    = getnext-string
 *      <password> = getnext-string
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
FUNC_COMMAND(nickserv_chpass)
{
  nickserv_dbase_data *data;
  char buf[BUFFER_SIZE];
  const char *p;
  char *who = getnext(params);
  char *pass = getrest(params);

  /* does the user have access to this command */
  if (!from->nickserv) return ERROR_NO_ACCESS;
  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;

  /* enough paramters */
  if (!pass) return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);
  /* is the nick regged */
  if (!(data = nickserv_dbase_find_nick(who))) return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_NOT_REGISTERED, who);

  /* crypt the new password */
  p = ircd_crypt(pass, from->host);
  data->password = (char*)realloc(data->password, strlen(p)+1);
  strcpy(data->password, p);

  /* save it to the database */
  snprintf(buf, BUFFER_SIZE, "UPDATE nickdata SET password='%s' WHERE nick='%s'", data->password, queue_escape_string(data->nick));
  queue_add(buf);

  /* log the command */
  strcpy(buf, queue_escape_string(who));
  log_command(LOG_NICKSERV, from, "CHPASS", "%s %s", buf, queue_escape_string(pass));
  /* return a confirmation-string */
  return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_CHPASS_OK, who, pass);
}
