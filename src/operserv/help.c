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
/* $Id: help.c,v 1.2 2003/02/25 23:15:03 cure Exp $ */

#include <string.h>

#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "help.h"
#include "log.h"

extern help_db *help_operserv;
extern struct parser_command_data parser_operserv_commands[];

/**************************************************************************************************
 * operserv_help
 **************************************************************************************************
 *   HELP [subject]
 *   Displays help 
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
FUNC_COMMAND(operserv_help)
{
  help_db *help_db;
  char *help;
  help = getnext(params);
  
  if(!help)
  {
    int i, acc;
    char buf[BUFFER_SIZE];
    com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_HELP_HEADER, conf->os->nick, conf->os->nick);
    
    acc = parser_operserv_commands[0].flags;
    i = 0;
    strcpy(buf, "");
    
    while (parser_operserv_commands[i].command)
    {
      if ((parser_operserv_commands[i].flags != acc) || (strlen(buf) > 70))
      {
        if (operserv_have_access(from->nickserv->flags, acc))
        {
          char flagsbuf[10];
          operserv_flags_to_str(acc, flagsbuf);
          com_message(sock, conf->os->numeric, from->numeric, format, "   OPER +%s:%s", flagsbuf, buf);
        }
        strcpy(buf, "");
      }
      acc = parser_operserv_commands[i].flags;
      
      strcat(buf, " ");
      strcat(buf, parser_operserv_commands[i].command);
      i++;
    }
    
    com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_HELP_FOOTER, conf->os->nick);
  }
  else
  {
    if(!(help_db = help_search(help_operserv, help)))
      com_message(sock, conf->os->numeric, from->numeric, format, "No help available on %s", help);
    else
      help_show(sock, from->numeric, conf->os->numeric, format, help_db);
  }

  return 0;
}
