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
/* $Id: info.c,v 1.3 2003/03/01 16:47:08 cure Exp $ */

#include "nickserv.h"
#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "queue.h"
#include "log.h"

/**************************************************************************************************
 * nickserv_info
 **************************************************************************************************
 *   INFO [nick]
 *      Returns information aboit [nick]
 *      If [nick] is not specified, returns information about the user
 *       [nick] = getnext-string
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
FUNC_COMMAND(nickserv_info)
{
  int from_ns_oper = 0;
  nickserv_dbase_data *info;
  char *nick = getnext(params);
  
  /* is the user an NS_OPER ? */
  if (from->nickserv) from_ns_oper = operserv_have_access(from->nickserv->flags, BITS_OPERSERV_NS_OPER);

  /* is nick specified, else show info for the user issuing the command */
  if (!nick)
    nick = from->nick;

  /* is the nick registered */
  if (!(info = nickserv_dbase_find_nick(nick)))
    return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_NOT_REGISTERED, nick);

  /* Write common data about the user */
  com_message(sock, conf->ns->numeric, from->numeric, format, "Information about %s:", info->nick);

  com_message(sock, conf->ns->numeric, from->numeric, format, "  Registered at: %s", gtime((time_t*)&info->regdate));
  com_message(sock, conf->ns->numeric, from->numeric, format, "  Last login...: %s from %s", gtime((time_t*)&info->lastlogin), info->userhost);
  
  /* write the user's info-line, if set */
  if (info->info)
    if (*info->info) com_message(sock, conf->ns->numeric, from->numeric, format, "           Info: %s", info->info);

  /* Is the user issuing the command an NS_OPER */
  if (from_ns_oper || from->nickserv == info)
  {
    /* is the nick a juped nick */
    if (info->flags & BITS_NICKSERV_JUPED)
      com_message(sock, conf->ns->numeric, from->numeric, format, "  This nickname is currently forbidden.");
    else
    {
   
      /* Write the users email-address and settings */
      com_message(sock, conf->ns->numeric, from->numeric, format, "  Email........: %s", info->email);
      com_message(sock, conf->ns->numeric, from->numeric, format, "  Oper.........: %s", (info->flags & BITS_NICKSERV_OPER)?"On":"Off");
      com_message(sock, conf->ns->numeric, from->numeric, format, "  Secret.......: %s", (info->flags & BITS_NICKSERV_SECRET)?"On":"Off");
      com_message(sock, conf->ns->numeric, from->numeric, format, "  PrivMsg......: %s", (info->flags & BITS_NICKSERV_PRIVMSG)?"On":"Off");
      com_message(sock, conf->ns->numeric, from->numeric, format, "  No Expire....: %s", (info->flags & BITS_NICKSERV_NOEXPIRE)?"On":"Off");
      com_message(sock, conf->ns->numeric, from->numeric, format, "  Comments.....: %lu", info->comment_count);
      /* Listing all channel where user got access */
      if (info->access) 
      {
        int i = 0; /* using this to print the first and then the rest in the for loop */
        com_message(sock, conf->ns->numeric, from->numeric, format, "  Channels.....: %c%3d - %s", (info->access[i]->autoop?'@':' '), info->access[i]->level, info->access[i]->channel->name);

        for (i = 1; i < info->access_count; i++)
          com_message(sock, conf->ns->numeric, from->numeric, format, "                 %c%3d - %s", (info->access[i]->autoop?'@':' '), info->access[i]->level, info->access[i]->channel->name);
      }
      else
        com_message(sock, conf->ns->numeric, from->numeric, format, "  Channels.....: 0");
    }
  }
  else
  {
    /* write the uses email-address, unless secret is set */
    if (info->flags & BITS_NICKSERV_SECRET)
      com_message(sock, conf->ns->numeric, from->numeric, format, "          Email: (hidden)");
    else
      com_message(sock, conf->ns->numeric, from->numeric, format, "          Email: %s", info->email);
  }
        
  /* log the command */
  log_command(LOG_NICKSERV, from, "INFO", queue_escape_string(nick));
  
  return 0;
}
