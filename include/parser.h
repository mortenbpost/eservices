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
/* $Id: parser.h,v 1.3 2003/01/16 21:49:26 mr Exp $ */

#ifndef INC_PARSERS_H
#define INC_PARSERS_H

#include "dbase.h"
#include "server.h"

typedef struct parser_command_data parser_command_data;
typedef int (*parser_p10_function)(sock_info *sock, char *from, char **params);
typedef int (*parser_command_function)(sock_info *sock, dbase_nicks *from, char **params, char *format, parser_command_data *command_info);

struct parser_p10_data
{
  char *cmd;
  parser_p10_function func;
};

struct parser_command_data
{
  char                    *command;
  parser_command_function  func;
  unsigned long            level;
  unsigned long            flags;
  char                    *syntax;
};

#define FUNC_COMMAND(func) int func(sock_info *sock, dbase_nicks *from, char **params, char *format, parser_command_data *command_info)

int parser_p10(sock_info *sock, char *str);
int parser_commands(sock_info *sock, char *to, char **params, char *from, char *format);
int parser_ctcp(sock_info *sock, char *to, char **params, dbase_nicks *from);
int parser_dcc(sock_info *sock, char *str);
int parser_check_ignore(dbase_nicks *nick, char *to, char *format);
void parser_remove_ignore(void *ptr);
#endif /* INC_PARSERS_H */
