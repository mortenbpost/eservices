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
/* $Id: devoice.c,v 1.3 2003/03/01 16:47:04 cure Exp $ */

#include <stdio.h>
#include <string.h>

#include "chanserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

/* declare the external irc socket */
extern sock_info *irc;

#define CHANSERV_DEVOICE_OK           "%d users were devoiced on %s."

/**************************************************************************************************
 * chanserv_devoice
 **************************************************************************************************
 *   VOICE <#channel> {[nick],...}
 *      Set mode -v on the list of [nick]. If no [nick] set mode -v on self
 *      <#channel> = getnext-string
 *      [nick]     = getnext-string while != NULL
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
FUNC_COMMAND(chanserv_devoice)
{
  chanserv_dbase_channel *ch;
  int count = 0;
  char *chan = getnext(params);
  char *nick = getnext(params);
  char buf[BUFFER_SIZE], bufn[BUFFER_SIZE], bufc[BUFFER_SIZE];
  buf[0] = '\0'; bufn[0]= '\0'; bufc[0]= '\0';

  /* displays syntax if chan not given */
  if (!chan) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

  /* check if the channel is registered */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
  
  /* check to see if the channels is disabled */
  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED);
  
    /* Does the users have access to this command ? */
  if (!chanserv_dbase_check_access(from->nickserv, ch, command_info->level)) return ERROR_NO_ACCESS;

  /* If no nick(s) given, set to self */
  if (!nick) nick = from->nick;

  while (nick)
  {
    /* Translate nick to numeric */
    const char *num = nicks_getnum(nick);
    /* if a numeric was found for the user */
    if (num)
    {
      int mode;
      /* Hmm, is the user on the channel ? */
      if ((mode = channels_user_find(chan, num)) >= 0)
      {
        /* does the user actually have voice we can take from him/her ? */
        if (mode & 1)
        {
          snprintf(buf, BUFFER_SIZE, "%s %s", buf, num);
          snprintf(bufn, BUFFER_SIZE, "%s %s", bufn, nick);
          channels_usermode(-1, chan, "-v", num);
          bufc[count++] = 'v';
        }
      }
    }
    /* Get nick in list */
    nick = getnext(params);
  }

  bufc[count] = '\0';

  /* only log andsend the command if we actually voiced anyone */
  if (count)
  {
    /* send the command */
    char chanbuf[BUFFER_SIZE];
    com_send(irc, "%s M %s -%s %s\n", conf->cs->numeric, chan, bufc, buf);
    strcpy(chanbuf, queue_escape_string(chan));
    log_command(LOG_CHANSERV, from, "DEVOICE", "%s %s", chanbuf, queue_escape_string(bufn));
  }
  
  /* Inform user how many users was voiced */
  return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_DEVOICE_OK, count, chan);
}
