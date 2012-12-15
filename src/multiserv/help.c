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
/* $Id: help.c,v 1.2 2003/02/25 22:42:23 cure Exp $ */

#include <string.h>

#include "multiserv.h"
#include "misc_func.h"
#include "operserv.h"
#include "config.h"
#include "parser.h"
#include "help.h"

extern struct parser_command_data parser_multiserv_commands[];
extern help_db *help_multiserv;

#define MULTISERV_HELP_HEADER   "This is "NETWORK_NAME"'s MultiServ (%s). It is used for different\n"\
                                "purposes.\n"\
                                "\n"

#define MULTISERV_HELP_FOOTER   "For more information on a specific command, type:\n"\
                                "/msg %s help <command>"

/**************************************************************************************************
 * multiserv_help
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
FUNC_COMMAND(multiserv_help)
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
    acc = parser_multiserv_commands[0].flags;
    lvl = parser_multiserv_commands[0].level;
    strcpy(buf, "");
    
    /* write header */
    com_message(sock, conf->ns->numeric, from->numeric, format, MULTISERV_HELP_HEADER, conf->ms->nick);
    
    /* run through the entire list */
    while (parser_multiserv_commands[i].command)
    {
      /* is the line too long, did access or "level" change from last time ? */
      if ((parser_multiserv_commands[i].level != lvl) || (parser_multiserv_commands[i].flags != acc) || (strlen(buf) > 70))
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
      lvl = parser_multiserv_commands[i].level;
      acc = parser_multiserv_commands[i].flags;
      
      /* concat the buffer with the next command */
      strcat(buf, " ");
      strcat(buf, parser_multiserv_commands[i].command);
      i++;
    }
    
    /* write the footer */
    com_message(sock, conf->ns->numeric, from->numeric, format, MULTISERV_HELP_FOOTER, conf->ms->nick);
  }
  else
  {
    /* see if the command is in the help-database and write the help text if found */
    if(!(help_db = help_search(help_multiserv, help)))
      com_message(sock, conf->ns->numeric, from->numeric, format, "No help available on %s", help);
    else
      help_show(sock, from->numeric, conf->ns->numeric, format, help_db);
  }

  return 0;
}
