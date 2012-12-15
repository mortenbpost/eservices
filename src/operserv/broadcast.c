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
/* $Id: broadcast.c,v 1.3 2003/03/01 16:47:11 cure Exp $ */

#include <ctype.h>
#include <string.h>

#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

extern sock_info *irc;

#define OPERSERV_BROADCAST_USERS  "[Global Announcement by %s]: %s"
#define OPERSERV_BROADCAST_OPERS  "[Oper Announcement by %s]: %s"
#define OPERSERV_BROADCAST_SENT   "Message sent to %d users."

/**************************************************************************************************
 * operserv_broadcast
 **************************************************************************************************
 *   BROADCAST
 *   Broadcasts a message on the network.
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
FUNC_COMMAND(operserv_broadcast)
{
  int i, cnt, no = 0; 
  char *type    = getnext(params);
  char *message = getrest(params);
  dbase_nicks *n;

  /* the mandatory access check */
  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;

  /* checks syntax */
  if (!message)
    return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);

  cnt = nicks_getcount(); /* saves the number of connected clients. */

  /* upper cases type */
  type = uppercase(type);

  /* use only the accepted types of broadcasts! */
  if (!strcmp(type, "OPERS"))  
  {
    /* looping through all connected clients */ 
    for (i = 0; i < cnt; i++)
    {
      if (!(n = nicks_getinfo(NULL, NULL, i))) continue;
      
      if ((n->numeric[0] == conf->numeric[0]) && (n->numeric[1] == conf->numeric[1]))
        continue;

      /* checks if user is opered up or is authenticated in nickserv and has
         oper access there. 
         Hmm, this might be fucked up... dunno... it's 2am and I've been working
         12 hours today... I'm exhausted... there's probably a better way but this
         works :P
       */
      if (isbiton(n->modes, 'o'-'a')) 
      {
        no++;
        /* announcements will be send from multiserv as a privat message */
        com_message(irc, conf->ms->numeric, n->numeric, MODE_PRIVMSG, OPERSERV_BROADCAST_OPERS, from->nick, message); 
      }
      else if (n->nickserv)
        if (n->nickserv->flags & BITS_NICKSERV_OPER)
        {
          no++;
          /* announcements will be send from multiserv as a privat message */
          com_message(irc, conf->ms->numeric, n->numeric, MODE_PRIVMSG, OPERSERV_BROADCAST_OPERS, from->nick, message); 
        }
    } 
  } /* same deal just with all users including opers */
  else if (!strcmp(type, "USERS")) 
  {
    for (i = 0; i < cnt; i++)
    {
      if (!(n = nicks_getinfo(NULL, NULL, i))) continue;
      
      if ((n->numeric[0] == conf->numeric[0]) && (n->numeric[1] == conf->numeric[1]))
        continue;

      no++;
      com_message(irc, conf->ms->numeric, n->numeric, MODE_PRIVMSG, OPERSERV_BROADCAST_USERS, from->nick, message); 
    } 
  }
  else
    return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);

  log_command(LOG_OPERSERV, from, "BROADCAST", "%s %s", type, queue_escape_string(message));
  return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_BROADCAST_SENT, no); 
}
