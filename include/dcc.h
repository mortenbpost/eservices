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
/* $Id: dcc.h,v 1.4 2003/02/14 18:56:04 mr Exp $ */

#ifndef INC_DCC_H
#define INC_DCC_H

#include "parser.h"
#include "server.h"

#define DCC_SYNTAX_CLOSE                    "Syntax: CLOSE"
#define DCC_SYNTAX_HELP                     "Syntax: HELP [command]"
#define DCC_SYNTAX_SAY                      "Syntax: SAY <message>"
#define DCC_SYNTAX_WHO                      "Syntax: WHO"
#define DCC_SYNTAX_CONSOLE                  "Syntax: CONSOLE [[+|-]modes]"

FUNC_COMMAND(dcc_help);
FUNC_COMMAND(dcc_close);
FUNC_COMMAND(dcc_say);
FUNC_COMMAND(dcc_who);
FUNC_COMMAND(dcc_console);

typedef struct
{
  char *num;
  unsigned long ip;
  int port;
} dcc_init_arg;

void *dcc_init(void *arg);
sock_info *dcc_init_connect(sock_info *sock, unsigned long ip, int port);
void dcc_free(dbase_nicks *nick);
void dcc_to_all(sock_info *from, char *str, ...);
void dcc_on_free(sock_info *sock);
void dcc_console_text(char mode, char *str, ...);

#endif /* INC_DCC_H */
