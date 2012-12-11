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
/* $Id: trace.c,v 1.2 2003/02/25 23:15:04 cure Exp $ */

#include <stdio.h>

#include "operserv.h"
#include "misc_func.h"
#include "errors.h"
#include "config.h"
#include "queue.h"
#include "log.h"

#define OPERSERV_TRACE_TRUNCATED  "More than 100 matches, output truncated!"

/**************************************************************************************************
 * operserv_trace
 **************************************************************************************************
 *   TRACE <user@host>
 *   Lists all connected users who matches <user@host>.
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
FUNC_COMMAND(operserv_trace)
{
  int i, cnt, no = 0;
  char *mask = getnext(params);
  char buffer[BUFFER_SIZE*2]; /* Saving the userhost into this bitch */
  dbase_nicks *n;

  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;

  if (!mask)
    return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);

  cnt = nicks_getcount();

  com_message(sock, conf->os->numeric, from->numeric, format, "Tracing network for %s:", mask);

  for (i = 0; i < cnt; i++)
  {
    n = nicks_getinfo(NULL, NULL, i);
    /* copying nick!username@host into buffer */
    snprintf(buffer, BUFFER_SIZE * 2, "%s!%s@%s", n->nick, n->username, n->host);

    if (wildcard_compare(buffer, mask))
    {
      no++;
      com_message(sock, conf->os->numeric, from->numeric, format, "  %c%c %-10s (%s)", 
                 ((isbiton(n->modes, 'o'-'a')))?'@':' ',(n->nickserv)?'*':' ', n->nick, buffer);
    } 
    if (no >= 100)
    {
      com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_TRACE_TRUNCATED); 
      break;
    }
  } 
  log_command(LOG_OPERSERV, from, "TRACE", "%s", queue_escape_string(mask));
  return com_message(sock, conf->os->numeric, from->numeric, format, "%d matches found for %s.", no, mask); 
}
