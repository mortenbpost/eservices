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
/* $Id: queue.c,v 1.5 2003/03/01 16:47:02 cure Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <mysql.h>

#include "setup.h"
#include "queue.h"
#include "dbase.h"
#include "misc_func.h"
#include "log.h"
#include "config.h"
#include "errors.h"
#include "main.h"

pthread_mutex_t mutex;

struct queue_list *queue_next;
struct queue_list *queue_last;

int queue_run = 1;

void *queue_write(void *arg)
{
  struct timeval timeout;
  int res;
  MYSQL queue_mysql, *queue_conn;

  mysql_init(&queue_mysql);
  mysql_options(&queue_mysql, MYSQL_OPT_COMPRESS, 0);

  queue_conn = mysql_real_connect(&queue_mysql, conf->mysql->host, conf->mysql->username, conf->mysql->password, conf->mysql->database, conf->mysql->port, conf->mysql->unixsock, 0);
  if (!queue_conn)
  {
    log_command(LOG_SERVICES, NULL, "", "mySQL error: %s", mysql_error(&queue_mysql));
    quit_service(ERROR_DBASE_MYSQL_ERROR);
    return NULL;
  }

  while (queue_run)
  {
    if (queue_next)
    {
      struct queue_list *current;
      current = queue_next;

      pthread_mutex_lock(&mutex);

      if (queue_next == queue_last) queue_last = NULL;
      queue_next = queue_next->next;

      pthread_mutex_unlock(&mutex);

      res = mysql_query(queue_conn, current->command);
      debug_out("[MYSQL] %s\n", current->command);
      if (res)
      {
        log_command(LOG_SERVICES, NULL, "", "mySQL error: \"%s\"", current->command);
        log_command(LOG_SERVICES, NULL, "", "mySQL error: (%d) %s", res, mysql_error(queue_conn));
      }

      xfree(current->command);
      xfree(current);
    }
    else
    {
      timeout.tv_sec  = 1;
      timeout.tv_usec = 0;
      select(0, NULL, NULL, NULL, &timeout);
    }
  }
  pthread_mutex_destroy(&mutex);
  mysql_close(queue_conn);
  mysql_close(&queue_mysql);
  return NULL;
}

int queue_add(const char *str)
{
  struct queue_list *entry;
  entry = (struct queue_list *)malloc(sizeof(struct queue_list));
  entry->next = NULL;
  entry->command = (char *)malloc(strlen(str)+1);
  strcpy(entry->command, str);

  pthread_mutex_lock(&mutex);

  if (!queue_next) queue_next = entry;
  if (queue_last) queue_last->next = entry;
  queue_last = entry;

  pthread_mutex_unlock(&mutex);
  return 0;
}

int queue_init(void)
{
  pthread_t tread;
  pthread_mutex_init(&mutex, NULL);
	pthread_create(&tread, NULL, queue_write, NULL);


	return 0;
}

void queue_wait_until_empty(void)
{
  struct timeval timeout;
  while (queue_next)
  {
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;
    select(0, NULL, NULL, NULL, &timeout);
  }
  queue_run = 0;
}

char *queue_escape_string_buf(const char *str, char *buf)
{ 
  char *p = buf;
  
  if (str)
  {
    while (*str)
    {
      if ((*str == '\'') || (*str == '\\')) *p++ = '\\';
      *p++ = *str++;
    }
  }
  *p = '\0';
  return buf;
}

char queue_escaped[BUFFER_SIZE];
char *queue_escape_string(const char *str)
{
  return queue_escape_string_buf(str, queue_escaped);
}
