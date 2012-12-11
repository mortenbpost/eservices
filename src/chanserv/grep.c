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
/* $Id: grep.c,v 1.3 2003/03/01 16:47:04 cure Exp $ */

#include <string.h>

#include "chanserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

extern int nickserv_list_count;
extern nickserv_dbase_data **nickserv_list;

extern int chanserv_list_count;
extern chanserv_dbase_channel **chanserv_list;

/**************************************************************************************************
 * chanserv_grep
 **************************************************************************************************
 *   GREP <chan|user> <mask>
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
FUNC_COMMAND(chanserv_grep)
{
  char *type = getnext(params);
  char *mask = getnext(params);
  int no = 0;

  if (!type || !mask)
    return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

  type = uppercase(type);

  if (!strcmp(type, "CHAN"))
  {
    int i;

    com_message(sock, conf->cs->numeric, from->numeric, format, "Searching %s for %s.", type, mask);

    for (i = 0; i < chanserv_list_count; i++)
    {
      if (wildcard_compare(chanserv_list[i]->name, mask))
      {
        no++;
        com_message(sock, conf->cs->numeric, from->numeric, format, "  %-9s %-2lu %-2lu %c%c %s", chanserv_list[i]->owner, chanserv_list[i]->comment_count,
                    chanserv_list[i]->access_count, (chanserv_list[i]->flags & BITS_CHANSERV_EXPIRED)?'E':'.', 
                    (chanserv_list[i]->flags & BITS_CHANSERV_DISABLED)?'D':'.', chanserv_list[i]->name);
      }
    }
  }
  else if (!strcmp(type, "USER"))
  {
    int i;

    com_message(sock, conf->cs->numeric, from->numeric, format, "Searching %s for %s.", type, mask);

    for (i = 0; i < nickserv_list_count; i++)
    {
      if (strchr(mask, '@')  && wildcard_compare(nickserv_list[i]->email, mask))
      {
        no++;
        com_message(sock, conf->cs->numeric, from->numeric, format, "  %-9s %-2lu %-2lu %c (%s)", nickserv_list[i]->nick, nickserv_list[i]->comment_count, 
                    nickserv_list[i]->access_count, (nickserv_list[i]->flags & BITS_NICKSERV_OPER)?'@':' ', nickserv_list[i]->email);
        continue;
      }  

      if (wildcard_compare(nickserv_list[i]->nick, mask))
      {
        no++;
        com_message(sock, conf->cs->numeric, from->numeric, format, "  %-9s %-2lu %-2lu %c (%s)", nickserv_list[i]->nick, nickserv_list[i]->comment_count, 
                    nickserv_list[i]->access_count, (nickserv_list[i]->flags & BITS_NICKSERV_OPER)?'@':' ', nickserv_list[i]->email);
      }
    }
  }
  else
    return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
   
  log_command(LOG_CHANSERV, from, "GREP", "%s %s", type, queue_escape_string(mask));
  return com_message(sock, conf->cs->numeric, from->numeric, format, "%d matches found for %s.", no, mask);
  
}
