/****************************************************************************
* Exiled.net IRC Services                                                   *
* Copyright (C) 2002  Michael Rasmussen <the_real@nerdheaven.dk>            *
*                     Morten Post <cure@nerdheaven.dk>                      *
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
/* $Id: operserv.h,v 1.8 2003/02/25 23:14:11 cure Exp $ */

#ifndef INC_OPERSERV_H
#define INC_OPERSERV_H

#include "parser.h"

/* Help function defines 
   dcc.c uses this part so I could't put it in src/operserv/help.c 
 */
#define OPERSERV_HELP_HEADER    "This is "NETWORK_NAME"'s Operator Service (%s). It's used for\n"\
                                "maintenance of the network. Please use this service with\n"\
                                "caution due to the power it consists!\n"\
                                "\n"\
                                "To use them type /msg %s command\n"\
                                "\n"
#define OPERSERV_HELP_FOOTER    "\n"\
                                "For more information on a specific command, type\n"\
                                "/msg %s HELP <command>"
/* Syntax function defines */
#define OPERSERV_SYNTAX_HELP      "Syntax: HELP [command]"
#define OPERSERV_SYNTAX_BROADCAST "Syntax: BROADCAST <USERS|OPERS> <message>"
#define OPERSERV_SYNTAX_OPERLIST  "Syntax: OPERLIST"
#define OPERSERV_SYNTAX_OP        "Syntax: OP <#channel>"
#define OPERSERV_SYNTAX_MODE      "Syntax: MODE <#channel> <modes [mode parameters]>"
#define OPERSERV_SYNTAX_TRACE     "Syntax: TRACE <user@host>"
#define OPERSERV_SYNTAX_GLINE     "Syntax: GLINE <user@host> <durance> <reason>"
#define OPERSERV_SYNTAX_UNGLINE   "Syntax: UNGLINE <user@host>"
#define OPERSERV_SYNTAX_DIE       "Syntax: DIE <confirm string> <reason>"
#define OPERSERV_SYNTAX_REMOPER   "Syntax: REMOPER <nick> FORCE"
#define OPERSERV_SYNTAX_ACCESS    "Syntax: ACCESS <nick> [+access | -access]"

/* GENERIC OPERSERV_ defines that are used in common. They should
   all be placed here.
 */
#define OPERSERV_USER_NOT_FOUND   "User not found."
#define OPERSERV_INVALID_GLINE    "Invalid gline user@host, must contain at least 5 non-wildcard characters!"

/* 
   FUNC_COMMAND prototypes.
   FUNC_COMMAND is #defined in parser.h
 */
FUNC_COMMAND(operserv_help);
FUNC_COMMAND(operserv_broadcast);
FUNC_COMMAND(operserv_operlist);
FUNC_COMMAND(operserv_mode);
FUNC_COMMAND(operserv_op);
FUNC_COMMAND(operserv_trace);
FUNC_COMMAND(operserv_gline);
FUNC_COMMAND(operserv_ungline);
FUNC_COMMAND(operserv_die);
FUNC_COMMAND(operserv_access);
FUNC_COMMAND(operserv_remoper);

/*
   Function prototypes for operserv helper functions.
 */
unsigned long operserv_flag(unsigned long flags, unsigned long required);
unsigned long operserv_str_to_flags(char *str, unsigned long flags, unsigned long perm);

char *operserv_flags_to_title(unsigned long flags, char *buf);
char *operserv_flags_to_str(unsigned long flags, char *buf);

int operserv_valid_gline(char *userhost);
int operserv_have_access(unsigned long user, unsigned long flags);

#endif /* INC_OPERSERV_H */
