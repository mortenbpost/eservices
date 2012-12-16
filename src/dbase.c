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
/* $Id: dbase.c,v 1.4 2003/03/01 16:52:08 mr Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mysql.h>
#include <string.h>

#include "setup.h"
#include "dbase.h"
#include "config.h"
#include "server.h"
#include "queue.h"
#include "log.h"
#include "errors.h"
#include "misc_func.h"
#include "chanserv.h"
#include "nickserv.h"


/*
 *****************************************************l*********************************************
 * dbase_load_persistant
 **************************************************************************************************
 *   Load persistant data from mySQL database into a dbase in memory
 **************************************************************************************************
 * Params:
 *   [OUT] int <result> - 0 if success, else error-nr
 **************************************************************************************************
 */
int dbase_load_persistant(void)
{

  MYSQL_RES *result;
  MYSQL_ROW row;
  MYSQL mysql, *conn;
  int res;

  mysql_init(&mysql);

  mysql_options(&mysql, MYSQL_OPT_COMPRESS, 0);

  conn = mysql_real_connect(&mysql, conf->mysql->host, conf->mysql->username, conf->mysql->password, conf->mysql->database, conf->mysql->port, conf->mysql->unixsock, 0);
  if (!conn)
  {
    log_command(LOG_SERVICES, NULL, "", "mySQL error: %s", mysql_error(&mysql));
    return ERROR_DBASE_MYSQL_ERROR;
  }

  /* Load Information about registered channels */
  res = mysql_query(conn, "SELECT name,owner,topic,keepmode,flags,lastlogin FROM chandata");
  if (res)
  {
    log_command(LOG_SERVICES, NULL, "", "mySQL error: %s", mysql_error(conn));
    mysql_close(conn);
    return ERROR_DBASE_MYSQL_ERROR;
  }

  result = mysql_use_result(conn);

  while ((row = mysql_fetch_row(result)))
  {
    chanserv_dbase_add(row[0], row[1], row[2], row[3], tr_atoi(row[4]), tr_atoi(row[5]), 0);
  }
  mysql_free_result(result);

  /* Load Information about registered nick */
  res = mysql_query(conn, "SELECT nick,email,password,userhost,lastlogin,info,flags,regdate,console FROM nickdata");
  if (res)
  {
    log_command(LOG_SERVICES, NULL, "", "mySQL error: %s", mysql_error(conn));
    mysql_close(conn);
    return ERROR_DBASE_MYSQL_ERROR;
  }

  result = mysql_use_result(conn);

  while ((row = mysql_fetch_row(result)))
  {
    nickserv_dbase_add(row[0], row[1], row[2], row[3], tr_atoi(row[4]), row[5], tr_atoi(row[6]), tr_atoi(row[7]), tr_atoi(row[8]), 0);
  }
  mysql_free_result(result);

  /* Load Information about chanserv access */
  res = mysql_query(conn, "SELECT channel,nick,level FROM access");
  if (res)
  {
    log_command(LOG_SERVICES, NULL, "", "mySQL error: %s", mysql_error(conn));
    mysql_close(conn);
    return ERROR_DBASE_MYSQL_ERROR;
  }

  result = mysql_use_result(conn);

  while ((row = mysql_fetch_row(result)))
  {
    chanserv_dbase_channel *chan = chanserv_dbase_find_chan(row[0]);
    nickserv_dbase_data *nick    = nickserv_dbase_find_nick(row[1]);
    unsigned long level           = tr_atoi(row[2]);

    if ((chan) && (nick))
      chanserv_dbase_access_add(chan, nick, level & 0xffff, (level >> 16), 0);
    else
      log_command(LOG_SERVICES, NULL, "", "mySQL load: Error giving %s access level of %s on %s", row[1], row[2], row[0]);
  }
  mysql_free_result(result);

  /* Load Information about nickserv notices */
  res = mysql_query(conn, "SELECT nick,sender,notice,com_date FROM notice");
  if (res)
  {
    log_command(LOG_SERVICES, NULL, "", "mySQL error: %s", mysql_error(conn));
    mysql_close(conn);
    return ERROR_DBASE_MYSQL_ERROR;
  }

  result = mysql_use_result(conn);

  while ((row = mysql_fetch_row(result)))
  {
    nickserv_dbase_data *who = nickserv_dbase_find_nick(row[0]);
 
    if (!who) continue;
    
    who->notices = (dbase_comment**)realloc(who->notices, (++who->notice_count) * SIZEOF_VOIDP);
    who->notices[who->notice_count-1] = (dbase_comment*)malloc(sizeof(dbase_comment));
  
    who->notices[who->notice_count-1]->date = tr_atoi(row[3]);
    who->notices[who->notice_count-1]->nick = (char*)malloc(strlen(row[1]) + SIZEOF_CHAR);
    who->notices[who->notice_count-1]->comment = (char*)malloc(strlen(row[2]) + SIZEOF_CHAR);

    strcpy(who->notices[who->notice_count-1]->nick, row[1]);
    strcpy(who->notices[who->notice_count-1]->comment, row[2]);
  }
  mysql_free_result(result);
  
  /* Load Information about chanserv/nickserv comments */
  res = mysql_query(conn, "SELECT subject,nick,comment,com_date FROM comment");
  if (res)
  {
    log_command(LOG_SERVICES, NULL, "", "mySQL error: %s", mysql_error(conn));
    mysql_close(conn);
    return ERROR_DBASE_MYSQL_ERROR;
  }

  result = mysql_use_result(conn);

  while ((row = mysql_fetch_row(result)))
  {
    nickserv_dbase_data *nick = nickserv_dbase_find_nick(row[1]);
    char *comment = row[2];
    unsigned long date = tr_atoi(row[3]);
    if (row[0][0] == '#')
    {
      /* chanserv comment */
      chanserv_dbase_channel *chan = chanserv_dbase_find_chan(row[0]);
      if ((!chan) || (!nick) || (!comment) || (!date))
        log_command(LOG_SERVICES, NULL, "", "mySQL load: Error setting comment on %s from %s", row[0], row[1]);
      else
        chanserv_dbase_comment_add(chan, nick, comment, date, 0);
    }
    else
    {
      /* nickserv command */
      nickserv_dbase_data *who = nickserv_dbase_find_nick(row[1]);
      if ((!who) || (!nick) || (!comment) || (!date))
        log_command(LOG_SERVICES, NULL, "", "mySQL load: Error setting comment on %s from %s", row[0], row[1]);
      else      
        nickserv_dbase_comment_add(who, nick, comment, date, 0);
    }
  }
  mysql_free_result(result);
  
  /* Load Information about chanserv bans */
  res = mysql_query(conn, "SELECT chan,mask,expire,nick FROM bans");
  if (res)
  {
    log_command(LOG_SERVICES, NULL, "", "mySQL error: %s", mysql_error(conn));
    mysql_close(conn);
    return ERROR_DBASE_MYSQL_ERROR;
  }

  result = mysql_use_result(conn);

  while ((row = mysql_fetch_row(result)))
  {
    chanserv_dbase_channel *ch = chanserv_dbase_find_chan(row[0]);
    long expire                = tr_atoi(row[2]);
    
    if (expire) expire -= time(0);
    if (expire < 0) expire = 0;

    if (ch)
      chanserv_dbase_add_enforce_ban(ch, row[1], expire, row[3], 0);
    else
      log_command(LOG_SERVICES, NULL, "", "mySQL load: Error setting ban on %s (%s)", row[0], row[1]);
  }
  mysql_free_result(result);
  
  mysql_close(conn);
  mysql_close(&mysql);
  return 0;
}
