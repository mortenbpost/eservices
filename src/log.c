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
/* $Id: log.c,v 1.3 2003/02/21 23:12:19 mr Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "setup.h"
#include "errors.h"
#include "log.h"
#include "config.h"
#include "misc_func.h"
#include "dbase.h"
#include "queue.h"
#include "dcc.h"

static FILE *logfile = NULL;

/*
 **************************************************************************************************
 * log_open
 **************************************************************************************************
 *   Open log-files ready to append new entries
 **************************************************************************************************
 * Params:
 *   [OUT] int <result> - 0 if succes, else error-nr
 **************************************************************************************************
 */
int log_open(void)
{
  if (conf)
  {
    if (conf->logfile)
    {
      if (!(logfile = fopen(conf->logfile,  "a")))
      {
        log_close();
        return ERROR_LOG_ERROR_OPENING_FILE;
      }
    }
    else
      return ERROR_LOG_NOT_DEFINED;
  }
  return 0;
}

/*
 **************************************************************************************************
 * log_close
 **************************************************************************************************
 *   Flushes and closes log files
 **************************************************************************************************
 * Params:
 *   None
 **************************************************************************************************
 */
void log_close(void)
{
  if (!logfile) return;
  fclose(logfile);
}

/*
 **************************************************************************************************
 * log_write
 **************************************************************************************************
 *   write a raw line to the logfile.
 *   OBS OBS - use log_command(LOG_SERVICES, NULL, "", str, ...) instead.
 *             only use log_write if a line should no be prefixed with a timestamp,
 *             fx to make a delimiter-line.
 **************************************************************************************************
 * Params:
 *   [IN]  char *str     - String to write
 *   [IN]  ...           - variables to str
 **************************************************************************************************
 */
void log_write(const char *str, ...)
{
  va_list arglist;

  if (!logfile) logfile = stderr;
  va_start(arglist, str);
  vfprintf(logfile, str, arglist);
  va_end(arglist);
  fflush(logfile);
}

/*
 **************************************************************************************************
 * log_command
 **************************************************************************************************
 *   Write a log entry for a command in the mySQL database.
 *
 *   If serv == LOG_SERVICES, command is ignored, and the paramters are passed on to
 *   log_write, although prefixed with a timestamp, hence use of log_write is
 *   depreceted, and all logging should use log_command.
 *   
 **************************************************************************************************
 * Params:
 *   [IN]  log_type serv      - the services who the command was for (nickserv, chanserv etc)
 *   [IN]  dbase_nicks *from  - The user who issued the command
 *   [IN]  char *command      - The command used
 *   [IN]  char *param        - Parameters to command
 *   [IN]  ...                - variables to param
 **************************************************************************************************
 */
void log_command(log_type serv, dbase_nicks *from, const char *command, const char *param, ...)
{
  char tables[5][15] = {"-", "log_nickserv", "log_chanserv", "log_operserv", "log_multiserv"};
  char modes[5] = {'x', 'v', 'w', 'x', 'y'};
  char *nicks[5] = {conf->os->nick, conf->ns->nick, conf->cs->nick, conf->os->nick, conf->ms->nick};
  char buf[BUFFER_SIZE], buf2[2*BUFFER_SIZE];
  
  char rnick_buf[12] = "Not authed", nick_buf[12] = "Services";
  char *rnick = rnick_buf, *nick = nick_buf, *username = nick_buf, *host = NULL;
  
  va_list arglist;
  
  va_start(arglist, param);
  vsnprintf(buf, BUFFER_SIZE, param, arglist);
  va_end(arglist);
  
  if (conf) host = conf->host;
    
  if (from)
  {
    nick = from->nick;
    username = from->username;
    host = from->host;
    if (from->nickserv) rnick = from->nickserv->nick;
  }
  
  if (serv == LOG_SERVICES)
  {
    time_t t;
    struct tm *td;

    t = time(0);
    td = localtime(&t);

    log_write("[%2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d (%s)] %s\n", td->tm_mday, (td->tm_mon +1), (td->tm_year + 1900), td->tm_hour, td->tm_min, td->tm_sec, td->tm_zone, buf);
    return;
  }
  
  dcc_console_text(modes[serv], "[%s!%s!%s@%s] %s: %s %s", rnick, nick, username, host, nicks[serv], command, buf);

  snprintf(buf2, 2*BUFFER_SIZE, "INSERT INTO %s (cmd_date,nick,userhost,command,params) VALUES (%lu,'%s','%s!%s@%s','%s','%s')", tables[serv], (unsigned long)time(0), rnick, nick, username, host, command, buf);
  queue_add(buf2);
}
