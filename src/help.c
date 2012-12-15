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
/* $Id: help.c,v 1.3 2003/01/17 18:32:40 mr Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "help.h"
#include "config.h"
#include "misc_func.h"

help_db *help_nickserv  = NULL;
help_db *help_chanserv  = NULL;
help_db *help_operserv  = NULL;
help_db *help_multiserv = NULL;

int help_load(void)
{
  help_nickserv = help_load_db(HELP_DIR_NICKSERV);
  help_chanserv = help_load_db(HELP_DIR_CHANSERV);
  help_operserv = help_load_db(HELP_DIR_OPERSERV);
  help_multiserv = help_load_db(HELP_DIR_MULTISERV);
  return 0;
}

help_db *help_load_db(char *dir)
{
  int i = 0;
  char buffer[BUFFER_SIZE];
  char filename[BUFFER_SIZE];
  char help_dir[BUFFER_SIZE];
  help_db *new    = NULL;
  help_db *db     = NULL;
  struct dirent *dp;
  DIR *h_dir;
  FILE *fp;

  snprintf(help_dir, BUFFER_SIZE, "%s/%s/", getenv("PWD"), dir);

  if ((h_dir = opendir(help_dir)) == NULL)
  {
    debug_out("- Could not read help files (%s)\n", help_dir);
    return NULL;
  }

  while ((dp = readdir(h_dir)) != NULL)
  {
    if (dp->d_type & DT_DIR) continue;

    filename[0] = '\0';
    strcpy(filename, help_dir);
    strcat(filename, dp->d_name);

    if ((fp = fopen(filename, "r")))
    {
      new = (help_db *) malloc(sizeof(help_db));
      new->next = db;
      db = new;

      new->cmd_name  = (char *) malloc(strlen(dp->d_name)+1);
      strcpy(new->cmd_name, dp->d_name);

      for (i = 0; fgets(buffer, BUFFER_SIZE, fp); i++)
      {
        new->help_desc[i] = (char *) malloc(strlen(buffer)+1);
        strcpy(new->help_desc[i], buffer);
      }
      new->help_desc[i] = NULL;
      fclose(fp);
    }
  }
  closedir(h_dir);
  return db;
}


help_db *help_search(help_db *db, char *entry)
{
  help_db *curr = db;

  while (curr)
  {
    if (strcasecmp(curr->cmd_name, entry) == 0) return curr;
    curr = curr->next;
  }

  return NULL;
}

int help_show(sock_info *sock, char *from, char *to, char *format, help_db *db)
{
  int i = 0;

  while (db->help_desc[i] != NULL)
  {
    com_message(sock, to, from, format, "%s", db->help_desc[i]);
    i++;
  }

  return 1;
}


int help_print(help_db *db)
{
  int i = 0;
  help_db *curr = db;

  while (curr)
  {
    debug_out("name: %s\n", curr->cmd_name);
    i = 0;
    while(curr->help_desc[i])
      debug_out(">%s\n", curr->help_desc[i++]);

    curr = curr->next;
  }

  return 1;

}

int help_free(void)
{
  help_free_internal(help_nickserv);
  help_free_internal(help_chanserv);
  help_free_internal(help_operserv);
  debug_out(" | |==> Cleaning Help database...\n");

  return 0;
}

int help_free_internal(help_db *db)
{
  if (!db) return 0;

  if (db->next)
    help_free_internal(db->next);
  xfree(db);

  return 0;
}
