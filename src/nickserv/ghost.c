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
/* $Id: ghost.c,v 1.3 2003/03/01 16:47:08 cure Exp $ */

#include <string.h>

#include "nickserv.h"
#include "misc_func.h"
#include "config.h"
#include "queue.h"
#include "log.h"

/* This is used for sending the WALLOPS */
extern sock_info *irc;

#define NICKSERV_GHOST_NO_SELF              "You can't use GHOST to kill yourself."
#define NICKSERV_GHOST_NOT_BLOCKED          "No-one is online or authenticated with that nickname."

/**************************************************************************************************
 * nickserv_ghost
 **************************************************************************************************
 *   GHOST <nick> <password>
 *      Makes services do a cross-kill to free <nick>, so the rightfull owner can use it.
 *      Also an efficient way to get rid of hanging clients (ghosts).
 *       <nick>     = getnext-string
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
FUNC_COMMAND(nickserv_ghost)
{
  nickserv_dbase_data *data;
  dbase_nicks *ns;
  const char *num;
  char *nick = getnext(params);
  char *pass = getnext(params);

  /* are there enough paramters */
  if (!pass) return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);
  /* is the specified nick regged */
  if (!(data = nickserv_dbase_find_nick(nick))) return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_NOT_REGISTERED, nick);
  /* is the specified password the correct password for the specified nick */
  if (strcmp(data->password, ircd_crypt(pass, data->password))) return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_WRONG_PASSWORD);

  /* anyone online with nick? */
  if ((ns = nicks_getinfo(NULL, nick, -1)))
  {
    num = ns->numeric;
    /* is the one online me? */
    if (ns == from)
    {
      /* select the user who is authed as nick, else if not found, return an error-message */
      if ((data->entry) && (data->entry != from))
        num = data->entry->numeric;
      else
        return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_GHOST_NO_SELF);
    }
  }
  else
  {
    /* anyone authed as nick */
    if (data->entry)
    {
      /* is the user issuing the command the one authed for the nick, if yes return error-mesage ? */
      if (data->entry == from)
        return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_GHOST_NO_SELF);
      num = data->entry->numeric;
    }
    /* noone online or authed as nick */
    else
      return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_GHOST_NOT_BLOCKED);
  }
  
  /* send the kill-string */
  com_send(irc, "%s D %s :%s Ghost requested by %s (%s@%s)\n", conf->ns->numeric, num, conf->ns->nick, from->nick, from->username, from->host);

  /* remove from internal databse */
  nicks_remove(num);
  
  /* log the command */
  log_command(LOG_NICKSERV, from, "GHOST", "%s [hidden]", queue_escape_string(nick));
  return 0;
}
