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
/* $Id: db_server.h,v 1.1.1.1 2002/08/20 15:20:02 mr Exp $ */

#ifndef INCL_DB_SERVER_H
#define INCL_DB_SERVER_H

typedef struct dbase_server
{
  char numeric[3];
  char *name;
  char *desc;
  int children_count;
  unsigned long linktime;
  struct dbase_server **children;
  struct dbase_server *parent;
} dbase_server;

void server_add(char *from, char *numeric, char *name, unsigned long linktime, char *desc);
dbase_server *server_search(char *numeric);
dbase_server *server_find(char *numeric, dbase_server *root);
dbase_server *server_find_name(char *name, dbase_server *root);
void server_remove(char *name);
void server_free(dbase_server *server, int top);
void server_dump(dbase_server *root, int level);
#endif /* INCL_DB_SERVER_H */
