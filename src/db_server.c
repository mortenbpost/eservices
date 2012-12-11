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
/* $Id: db_server.c,v 1.3 2003/02/14 18:24:12 mr Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "setup.h"
#include "misc_func.h"
#include "dbase.h"
#include "config.h"
#include "server.h"
#include "queue.h"
#include "log.h"
#include "parser.h"
#include "chanserv.h"
#include "operserv.h"
#include "nickserv.h"
#include "errors.h"
#include "help.h"
#include "db_server.h"

extern dbase_nicks **nicks_num;
extern long          nicks_count;

dbase_server *servers = NULL;

void server_add(char *from, char *numeric, char *name, unsigned long linktime, char *desc)
{
  dbase_server *ny;
  dbase_server *parent = NULL;

  numeric[2] = '\0';

  ny = (dbase_server*) malloc(sizeof(dbase_server));

  if ((parent = server_find(from, servers)))
  {
    parent->children = (dbase_server**)realloc(parent->children, sizeof(dbase_server*)*(++parent->children_count));
    parent->children[parent->children_count -1] = ny;
  }
  else if (!from)
  {
    servers = ny;
  }
  else
  {
    /* BIG ERROR - got a server, but havent got it's "parent" */
  }

  ny->parent = parent;

  strcpy(ny->numeric, numeric);

  ny->name = (char *)malloc(strlen(name)+1);
  strcpy(ny->name, name);

  ny->desc = (char *)malloc(strlen(desc)+1);
  strcpy(ny->desc, desc);

  ny->linktime = linktime;

  ny->children = NULL;
  ny->children_count = 0;
}

dbase_server *server_search(char *numeric)
{
  return server_find(numeric, servers);
}

dbase_server *server_find(char *numeric, dbase_server *root)
{
  if (root)
  {
    int i;
    dbase_server *tmp;

    if (!strcmp(numeric, root->numeric)) return root;

    for (i = 0; i < root->children_count; i++)
      if ((tmp = server_find(numeric, root->children[i]))) return tmp;
  }
  return NULL;
}

dbase_server *server_find_name(char *name, dbase_server *root)
{
  if (root)
  {
    int i;
    dbase_server *tmp;

    if (!strcmp(name, root->name)) return root;

    for (i = 0; i < root->children_count; i++)
      if ((tmp = server_find_name(name, root->children[i]))) return tmp;
  }
  return NULL;
}


void server_remove(char *name)
{
  dbase_server *server = servers;

  if (name)
  {
    server = server_find_name(name, servers);
  }

  if (server)
  {
    if (server == servers) servers = NULL;
    server_free(server, 1);
  }
}

void server_free(dbase_server *server, int top)
{
  int i;
  if (!server) return;
  for (i = 0; i < server->children_count; i++)
    server_free(server->children[i], 0);

  i = 0;
  while (i < nicks_count)
  {
    if ((server->numeric[0] == nicks_num[i]->numeric[0]) && (server->numeric[1] == nicks_num[i]->numeric[1]))
    {
      if (nicks_remove(nicks_num[i]->numeric) < 0) i++;
    }
    else i++;
  }
  if (top)
  {
    if (server->parent)
    {
      for (i = 0; i < server->parent->children_count; i++)
      {
        if (server->parent->children[i] == server)
        {
          server->parent->children[i] = server->parent->children[--server->parent->children_count];
          server->parent->children = (dbase_server **) realloc(server->parent->children, sizeof(dbase_server *)*server->parent->children_count);
          break;
        }
      }
    }
  }

  xfree(server->children);
  xfree(server->desc);
  xfree(server->name);
  xfree(server);
}

void server_dump(dbase_server *root, int level)
{
  int i;
  if ((!root) && (!level)) root = servers;
  if (root)
  {
    for (i = 0; i < level; i++) debug_out("| ");
    debug_out("%s (%s) - %s\n", root->name, root->numeric, root->desc);
    for (i = 0; i < root->children_count; i++)
      server_dump(root->children[i], level+1);
  }
}
