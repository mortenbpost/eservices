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
/* $Id: main.c,v 1.3 2003/03/01 00:32:20 mr Exp $ */

#include <setup.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "main.h"
#include "config.h"
#include "dbase.h"
#include "queue.h"
#include "log.h"
#include "server.h"
#include "errors.h"
#include "help.h"
#include "nickserv.h"
#include "chanserv.h"
#include "timer.h"

sock_info *irc = NULL;
extern pthread_mutex_t sock_mutex;

void quit_do_cleanup(void)
{
  log_write("#############################################################################\n");
  log_command(LOG_SERVICES, NULL, "", "Services shutdown\n\n");

  debug_out("==> Starting onexit cleanup...\n");
  debug_out(" |==> Closing all sockets and freeing them...\n");
  com_free_all();           /* Close ALL connections and free memory */
  debug_out(" |==> Executing pending mySQL queries...\n");
  queue_wait_until_empty(); /* Make the queue write all the pending data to mySQL before quitting */
  debug_out(" |==> Freeing memory used by databases...\n");
  help_free();
  dbase_clear();            /* Clear the dbase and free memory */
  debug_out(" |==> Flushing and closing log files...\n");
  log_close();              /* flush and close log files */
  debug_out(" |==> Freeing memory used by settings...\n");
  conf_unload();            /* free memory taken by config */
  debug_out(" |==> Freeing memory used by timers...\n");
  timer_freeall();          /* remove all timers */
  debug_out(" \\==> Cleanup done, quitting...\n");
}

void quit_got_signal(int sig)
{
  char si[8] = "SIGINT", st[9] = "SIGTERM", *p = si;
  if (sig == SIGTERM) p = st;
    
  debug_out("%s captured... exiting gracefully...\n", p);
  log_command(LOG_SERVICES, NULL, "", "Service shutdown (caught %s)", p);
  
  com_send(irc, "%s Q :Service shutdown\n", conf->cs->numeric);
  com_send(irc, "%s WA :caught %s - service shutdown\n", conf->os->numeric, p);
  com_send(irc, "%s SQ %s %lu :caught %s - service shutdown\n", conf->os->numeric, conf->host, conf->starttime, p);
  
  exit(1);
}

void sighup_rehash(int sig)
{
  com_send(irc, "%s WA :caught SIGHUP - ignored it\n", conf->os->numeric);
  signal(SIGHUP, sighup_rehash);
}

/*
 **************************************************************************************************
 * quit_service
 **************************************************************************************************
 *   General function which call misc. clean-up-functions
 **************************************************************************************************
 * Params:
 *   [IN]  int errno     - error number to write to log-file - see errors.h for more info.
 **************************************************************************************************
 */
void quit_service(int errnr)
{
  log_command(LOG_SERVICES, NULL, "", "Service quit with errno %d (%s)", errnr, error_getstring(errnr));
  if (errnr == ERROR_OPERSERV_DIE) exit(0);
  else exit(errnr);
}

/*
 **************************************************************************************************
 * main
 **************************************************************************************************
 *   Main function
 **************************************************************************************************
 * Params:
 *   std main params
 **************************************************************************************************
 */
int main(int argc, char **argv)
{
  int res;
  srand((unsigned long)time(NULL));

  /* if (fork()) exit(0); */

  atexit(quit_do_cleanup);
  signal(SIGINT, quit_got_signal);
  signal(SIGTERM, quit_got_signal);
  signal(SIGHUP, sighup_rehash);

  if ((res = conf_load())) quit_service(res);
  if ((res = log_open())) quit_service(res);

  log_command(LOG_SERVICES, NULL, "", "Services startup");
  log_write("#############################################################################\n");
  
  if ((res = dbase_load_persistant())) quit_service(res);
  if ((res = queue_init())) quit_service(res);
  help_load();
  

  nickserv_dbase_checkold(NULL);
  chanserv_dbase_check_expire(NULL);
  
  pthread_mutex_init(&sock_mutex, NULL);

  irc = com_sock_create(SOCK_SERVER);
  if (!com_connect(irc)) quit_service(ERROR_SERVER_COULD_NOT_CONNECT);

  irc->buffer[0] = '\0';
  
  res = com_mainloop();

  pthread_mutex_destroy(&sock_mutex);
  
  quit_service(res);

  return res;
}
