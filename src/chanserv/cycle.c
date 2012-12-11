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
/* $Id: cycle.c,v 1.2 2003/02/25 22:41:36 cure Exp $ */

#include "chanserv.h"
#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

/* declare the external irc socket */
extern sock_info *irc;

/**************************************************************************************************
 * chanserv_cycle
 **************************************************************************************************
 *   CYCLE <#channel>
 *      Make E part <#channel> an rejoin
 *      <#channel> = getnext-string
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
FUNC_COMMAND(chanserv_cycle)
{
  chanserv_dbase_channel *ch;
  /* Get first parameter */
  char *chan = getnext(params);
  
  /* If parameter not present, return syntax to user */
  if (!chan) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
  
  /* Is the specified channel regged ? */
  if (!chanserv_dbase_find_chan(chan)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);

  /* check if the channel is registered */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
  
  /* check to see if the channels is disabled */
  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED);
  
  /* Does the user have enough level on that channel to use this command */
  if ((chanserv_dbase_check_access(from->nickserv, ch, command_info->level)) || (operserv_have_access(from->nickserv->flags, BITS_OPERSERV_CS_OPER)))
  {
    /* Send the PART, JOIN, MODE command */
    com_send(irc, "%s L %s\n", conf->cs->numeric, chan);
    com_send(irc, "%s J %s\n", conf->cs->numeric, chan);
    com_send(irc, "%s M %s +o %s\n", conf->numeric, chan, conf->cs->numeric);
    
    log_command(LOG_CHANSERV, from, "CYCLE", queue_escape_string(chan));
    /* Don't bother sending user a reply, should be easy to spot that the
       command completed sucessfully */
    return 0;
  }
  /* User does not have enough level to this command, tell user */
  return ERROR_NO_ACCESS;
}
