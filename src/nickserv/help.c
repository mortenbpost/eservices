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
/* $Id: help.c,v 1.2 2003/02/25 22:41:17 cure Exp $ */

#include <string.h>

#include "help.h"
#include "misc_func.h"
#include "queue.h"
#include "config.h"
#include "parser.h"
#include "log.h"
#include "operserv.h"

extern struct parser_command_data parser_nickserv_commands[];
extern help_db *help_nickserv;

#define NICKSERV_HELP_HEADER    "This is "NETWORK_NAME"'s Nickname Service (%s). It allows\n"\
                                "you to register your nickname and guarantee that you can\n"\
                                "always use it. The following commands are for registration\n"\
                                "and maintenance of nicknames.\n"\
                                "\n"\
                                "To use them type /msg %s command\n"\
                                "\n"
                                
#define NICKSERV_HELP_FOOTER    "\n"\
                                "For more information on a specific command, type\n"\
                                "/msg %s HELP <command>\n"

/**************************************************************************************************
 * nickserv_help
 **************************************************************************************************
 *   HELP [command]
 *      Returns help text about the specified [command].
 *      If [command] is not specified, returns a list of all commands.
 *       [command] = getnext-string
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
FUNC_COMMAND(nickserv_help)
{
  help_db *help_db;
  char *help;
  help = getnext(params);

  /* is a parameter specified ? */
  if (!help)
  {
    char buf[BUFFER_SIZE];
    unsigned long flags = 0, lvl, acc, i;
    if (from->nickserv) flags = from->nickserv->flags;
    
    /* Init values */
    i = 0;
    acc = parser_nickserv_commands[0].flags;
    lvl = parser_nickserv_commands[0].level;
    strcpy(buf, "");
    
    /* write header */
    com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_HELP_HEADER, conf->ns->nick, conf->ns->nick);
    
    /* run through the entire list */
    while (parser_nickserv_commands[i].command)
    {
      /* is the line too long, did access or "level" change from last time ? */
      if ((parser_nickserv_commands[i].level != lvl) || (parser_nickserv_commands[i].flags != acc) || (strlen(buf) > 70))
      {
        /* is this a non-oper-command and is the user authed or is it a auted-not-required command ? */
        if ((!acc) && ((from->nickserv) || (!lvl)))
          com_message(sock, conf->ns->numeric, from->numeric, format, "           %s", buf);
        /* is it a oper-command, and do the user have access to it */
        else if (operserv_have_access(flags, acc))
        {
          char flagsbuf[10];
          /* Convert the numeric value to a string */
          operserv_flags_to_str(acc, flagsbuf);
          
          /* Is the command a helper-command, in that case, write is as such
             else write is as an OPER +[string] command */
          com_message(sock, conf->ns->numeric, from->numeric, format, "   OPER +%s:%s", flagsbuf, buf);
        }
        /* reset the buffer */
        strcpy(buf, "");
      }
      
      /* update the variables */
      lvl = parser_nickserv_commands[i].level;
      acc = parser_nickserv_commands[i].flags;
      
      /* concat the buffer with the next command */
      strcat(buf, " ");
      strcat(buf, parser_nickserv_commands[i].command);
      i++;
    }
    
    /* write the footer */
    com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_HELP_FOOTER, conf->ns->nick);
  }
  else
  {
    /* see if the command is in the help-database and write the help text if found */
    if(!(help_db = help_search(help_nickserv, help)))
      com_message(sock, conf->ns->numeric, from->numeric, format, "No help available on %s", help);
    else
      help_show(sock, from->numeric, conf->ns->numeric, format, help_db);
  }

  return 0;
}
