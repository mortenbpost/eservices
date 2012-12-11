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
/* $Id: help.h,v 1.1.1.1 2002/08/20 15:20:02 mr Exp $ */

#ifndef INC_HELP_H
#define INC_HELP_H

#include "server.h"

#define HELP_DIR_NICKSERV "help/nickserv/"
#define HELP_DIR_CHANSERV "help/chanserv/"
#define HELP_DIR_OPERSERV "help/operserv/"
#define HELP_DIR_MULTISERV "help/multiserv/"


typedef struct help_db {
  char *cmd_name;
  char *help_desc[100];
  struct help_db *next;
} help_db;

int help_load(void);
int help_free(void);
int help_free_internal(help_db *db);
int help_print(help_db *db);
help_db *help_load_db(char *dir);
help_db *help_search(help_db *db, char *entry);
int help_show(sock_info *sock, char *from, char *to, char *format, help_db *db);

#endif /* INC_HELP_H */

