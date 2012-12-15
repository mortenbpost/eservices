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
/* $Id: help.c,v 1.2 2003/02/25 22:41:36 cure Exp $ */

#include <string.h>

#include "chanserv.h"
#include "misc_func.h"
#include "config.h"
#include "server.h"
#include "queue.h"
#include "log.h"
#include "help.h"
#include "operserv.h"

extern help_db *help_chanserv;
extern struct parser_command_data parser_chanserv_commands[];

#define CHANSERV_HELP_HEADER    "This is "NETWORK_NAME"'s Channel Service (%s). It allows you\n"\
                                "to register your channel, to prevent takeovers and allows you\n"\
                                "to be opped in your own channel.\n"\
                                "\n"\
                                "To use them type /msg %s command\n"\
                                "\n"
                                
#define CHANSERV_HELP_FOOTER    "\n"\
                                "To see commands available on a specific channel, type:\n"\
                                "/msg %1$s showcommands <#channel>\n"\
                                "\n"\
                                "For more information on a specific command, type:\n"\
                                "/msg %1$s help <command>"

/**************************************************************************************************
 * chanserv_help
 **************************************************************************************************
 *   HELP [command]
 *      Display's list of functions, of specific information about [topic]
 *      [command] = getnext-string
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
FUNC_COMMAND(chanserv_help)
{
  help_db *help_db;
  char *help;
  int i, lvl, acc;
  char buf[BUFFER_SIZE];

  help = getnext(params);

  /* Did the user specify a command to retrieve help for */
  if (!help)
  {
    /* No parameters given to HELP, show list of commands */ 
    com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_HELP_HEADER, conf->cs->nick, conf->cs->nick);
    
    lvl = 0;
    acc = 0;
    i = 0;
    strcpy(buf, "");
    
    while (parser_chanserv_commands[i].command)
    {
      if ((parser_chanserv_commands[i].level != lvl) || (parser_chanserv_commands[i].flags != acc) || (strlen(buf) > 70))
      {
        if (!acc)
          com_message(sock, conf->cs->numeric, from->numeric, format, "       %3d:%s", lvl, buf);
        else if (operserv_have_access(from->nickserv->flags, acc))
        {
          char flagsbuf[10];
          operserv_flags_to_str(acc, flagsbuf);
          com_message(sock, conf->cs->numeric, from->numeric, format, "   OPER +%s:%s", flagsbuf, buf);
        }
        strcpy(buf, "");
      }
      
      lvl = parser_chanserv_commands[i].level;
      acc = parser_chanserv_commands[i].flags;
      
      strcat(buf, " ");
      strcat(buf, parser_chanserv_commands[i].command);
      i++;
    }

    com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_HELP_FOOTER, conf->cs->nick);

  }
  else
  {
    /* Is there any help in the database for the specified command ? */
    if((help_db = help_search(help_chanserv, help)))
      help_show(sock, from->numeric, conf->cs->numeric, format, help_db);
    else
      com_message(sock, conf->cs->numeric, from->numeric, format, "No help available on %s", help);
  }

  return 0;
}
