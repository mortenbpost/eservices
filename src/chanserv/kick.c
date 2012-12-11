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
/* $Id: kick.c,v 1.3 2003/03/01 16:47:04 cure Exp $ */

#include <string.h>

#include "chanserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

/* declare the external irc socket */
extern sock_info *irc;

#define CHANSERV_KICK_NOT_IN_CHAN     "%s is not in %s."                                      

/**************************************************************************************************
 * chanserv_kick
 **************************************************************************************************
 *   KICK <#channel> <nick> <reason>
 *      kicks <nick> from <#channel>
 *      <#channel> = getnext-string
 *      <nick>     = getnext-string
 *      <reason>   = getrest-string
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
FUNC_COMMAND(chanserv_kick)
{
  chanserv_dbase_channel *ch;
  /* Initialise variables with the parameters */
  char *chan = getnext(params);
  char *victim = getnext(params);
  char *reason = getrest(params);
  char vbuf[BUFFER_SIZE], rbuf[BUFFER_SIZE];
  dbase_nicks *info;
  int level;
  
  /* Is reason specified, ie is all the parameters present, if not, return syntax */
  if (!reason) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
  
  /* Is the user we are trying to kick in out database, ie. online */
  if (!(info = nicks_getinfo(NULL, victim, -1))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_KICK_NOT_IN_CHAN, victim, chan);
  
  /* check if the channel is registered */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
  
  /* check to see if the channels is disabled */
  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED);

  /* Does the user have access to this command */
  if ((level = chanserv_dbase_check_access(from->nickserv, ch, command_info->level)))
  {
    /* Is the victim actually on the channel ? */
    if (channels_user_find(chan, info->numeric) < 0) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_KICK_NOT_IN_CHAN, victim, chan);
    /* Does the victim have higher access then the user on that channel,
       if yes, tell user, and abort */
    if (info->nickserv)
      if (chanserv_dbase_check_access(info->nickserv, ch, level)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_VICTIM_HIGHER_LEVEL);
      
    strcpy(vbuf, queue_escape_string(victim));
    strcpy(rbuf, queue_escape_string(reason));
    log_command(LOG_CHANSERV, from, "KICK", "%s %s %s", queue_escape_string(chan), vbuf, rbuf);
    /* Do the actually kicking */
    com_send(irc, "%s K %s %s :%s\n", conf->cs->numeric, chan, info->numeric, reason);
    /* Keep the internal database synced */
    channels_userpart(-1, chan, info->numeric);
      
    chanserv_dbase_update_lastlogin(ch);

    return 0;
  }
  /* User does not have enough level to this command, tell user */
  return ERROR_NO_ACCESS;
}
