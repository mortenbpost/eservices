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
/* $Id: chowner.c,v 1.4 2003/03/01 16:47:04 cure Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chanserv.h"
#include "operserv.h"
#include "nickserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define CHANSERV_CHOWNER_OK           "Owner of %s sucessfully changed to %s.\n"\
                                      "Your access has been decreased to 499."
#define CHANSERV_CHOWNER_NO_ACCESS    "You cannot to give away %s to %s when he/she isn't on the access list."

/**************************************************************************************************
 * chanserv_chowner
 **************************************************************************************************
 *   CHOWNER <#channel> <rnick> <password>
 *      Changes the owner of <#channel> to <rnick>
 *      <password> MUST be equal to the current owner's NickServ password
 *
 *   CHOWNER <#channel> <rnick> FORCE
 *      +C
 *      Forcefully changes the owner of <#channel> to <rnick>
 *      <#channel> = getnext-string
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
FUNC_COMMAND(chanserv_chowner)
{
  chanserv_dbase_channel *ch;
  nickserv_dbase_data *info;
  chanserv_dbase_access *acc, *own;
  char *chan = getnext(params);
  char *nick = getnext(params);
  char *pass = getnext(params);
  char buf[BUFFER_SIZE], buf2[BUFFER_SIZE];
  int levelok = 0, passok = 0, userok = 0, oper = 0;
  
  if (!pass) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

  /* check if the channel is registered */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
  
  /* check to see if the channels is disabled */
  if (chanserv_dbase_disabled(ch)) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_DISABLED);
    
  if (!(info = nickserv_dbase_find_nick(nick))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_USER_NOT_FOUND, nick);
  
  if (chanserv_dbase_check_access(from->nickserv, ch, CHANSERV_LEVEL_OWNER)) levelok = 1;
  if ((acc = chanserv_dbase_has_access(nick, ch))) userok = 1;
  if (!strcmp(from->nickserv->password, ircd_crypt(pass, from->nickserv->password))) passok = 1;
  
  if (operserv_have_access(from->nickserv->flags, BITS_OPERSERV_CS_ADMIN))
  {
    if (!passok)
    {
      if (strcmp(pass, "FORCE"))
        return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
      oper = 1;
    }
    levelok = 1;
    passok = 1;
    if (!userok) userok = 1;
  }
  if (!levelok) return ERROR_NO_ACCESS;
  if (!userok) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHOWNER_NO_ACCESS, chan, nick);
  if (!passok) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_WRONG_PASSWORD);
  
  strcpy(buf2, queue_escape_string(ch->name));
  snprintf(buf, BUFFER_SIZE, "UPDATE access SET level='%d' WHERE nick='%s' AND channel='%s'", CHANSERV_LEVEL_OWNER -1, queue_escape_string(ch->owner), buf2);
  queue_add(buf);
  
  own = chanserv_dbase_has_access(ch->owner, ch);
  if (own) own->level--;
  
  snprintf(buf, BUFFER_SIZE, "UPDATE chandata SET owner='%s' WHERE name='%s'", queue_escape_string(info->nick), buf2);
  queue_add(buf);
  
  if (acc)
  {
    snprintf(buf, BUFFER_SIZE, "UPDATE access SET level='%d' WHERE nick='%s' AND channel='%s'", CHANSERV_LEVEL_OWNER, queue_escape_string(info->nick), buf2);
    queue_add(buf);
    acc->level = CHANSERV_LEVEL_OWNER;
  }
  else
    chanserv_dbase_access_add(ch, info, CHANSERV_LEVEL_OWNER, 1, COMMIT_TO_MYSQL);
    
  ch->owner = (char *)realloc(ch->owner, strlen(info->nick) + SIZEOF_CHAR);
  strcpy(ch->owner, info->nick);

  if (oper)
  {
    log_command(LOG_CHANSERV, from, "CHOWNER", "%s %s FORCE", ch->name, info->nick);
    nickserv_dbase_notice(info, "You have been given ownership of %s by %s.", ch->name, from->nickserv->nick);
    if (own)
      nickserv_dbase_notice(own->nick, "%s has changed the ownership of %s to %s.", from->nickserv->nick, ch->name, info->nick);
  }
  else
  {
    log_command(LOG_CHANSERV, from, "CHOWNER", "%s %s [hidden]", ch->name, info->nick);
    nickserv_dbase_notice(info, "You have been given ownership of %s by %s.", ch->name, from->nickserv->nick);
  }
  
  return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHOWNER_OK, chan, nick);
}
