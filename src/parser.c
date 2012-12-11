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
/* $Id: parser.c,v 1.9 2003/03/01 16:47:02 cure Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#include "setup.h"
#include "misc_func.h"
#include "dbase.h"
#include "config.h"
#include "server.h"
#include "queue.h"
#include "log.h"
#include "parser.h"
#include "nickserv.h"
#include "p10.h"
#include "msg.h"
#include "errors.h"
#include "operserv.h"
#include "timer.h"

extern char *build_date;
extern char *os_name;
extern int connected;
extern sock_info *irc;

/*
 **************************************************************************************************
 * parser_servr
 **************************************************************************************************
 *   Parser for server data, ie. P10 code
 **************************************************************************************************
 * Params:
 *   [IN]  sock_info *sock - the sock_info struct containing information about the individual socket
 *   [IN]  char *str - the string from the server to parse
 *   [OUT] int <result> - 0 if success, else error-nr
 **************************************************************************************************
 */
int parser_p10(sock_info *sock, char *str)
{
  char **params, *from, *cmd;
  struct parser_p10_data *cmds = parser_p10_commands;
  
  if (*str == ':') str++;

  params = &str;
  
  from = getnext(params);

  if (!connected)
  {
    if (!strcasecmp(from, "SERVER")) return p10_server(sock, NULL, params);
    else if (!strcasecmp(from, "PASS"))  return p10_pass(sock, NULL, params);
    else if (!strcasecmp(from, "ERROR"))  return p10_error(sock, NULL, params);
  }

  cmd = getnext(params);

  cmd = uppercase(cmd);

  while (cmds->cmd)
  {
    if (!strcmp(cmd, cmds->cmd))
    {
      return cmds->func(sock, from, params);
    }
    cmds++;
  }

  if (!cmds->cmd)
  {
    log_command(LOG_SERVICES, NULL, "", "Don't know how to parse: %s %s %s", from, cmd, getrest(params));
  }

  return 0;
}

/*
 **************************************************************************************************
 * parser_commands
 **************************************************************************************************
 *   Parser for the commands issued to nickserv, operserv, chanserv or multiserv
 **************************************************************************************************
 * Params:
 *   [IN]  sock_info *sock - the sock_info struct containing information about the individual socket
 *   [IN]  char *str    - the string from the server to parse
 *   [IN]  char *format - the format in which the data to the user should be send
 *   [IN]  char *str - the string from the server to parse
 *   [OUT] int <result> - 0 if success, else error-nr
 **************************************************************************************************
 */
int parser_commands(sock_info *sock, char *to, char **params, char *from, char *format)
{
  char notice[2] = "O", privmsg[2] = "P";
  char *cmd, num[4], nick[20] = "", *informat = format;
  struct parser_command_data *cmds = NULL;
  dbase_nicks *info = nicks_getinfo(from, NULL, -1);
  if (!info) return 0;

  /* is this user ignored */
  if (info->ignored) return 0;

  /* set the correct return-format */
  format = notice;
  if (info->nickserv)
    if (info->nickserv->flags & BITS_NICKSERV_PRIVMSG)
      format = privmsg;
    
  /* test if we should ignore this user */
  if (parser_check_ignore(info, to, format)) return 0;

  /* remove leading colon and spaces */
  skipcolon(params);
  /* any parametes at all? */
  if (!strlen(*params)) return 0;
    
  /* is this a CTCP command */
  if ((!strcmp(informat, MODE_PRIVMSG)) && (**params == 1))
  {
    char *p = *params;
    
    /* "remove" all leading CTCP chars (\001) */
    while (*p == '\001')
      p++;
    
    /* remove all trailing CTCP chars (\001) */
    while (p[strlen(p) - 1] == '\001') 
      p[strlen(p) - 1] = '\0';

    *params = p;
    return parser_ctcp(sock, to, params, info);
  }

  /* Juped nicks have numeric xxPyy, where xx is the server numeric, and yy the number */
  strcpy(num, conf->numeric);
  strcat(num, "J");
  
  /* test if the user is writing to a juped nick */
  if ((!strncmp(to, num, 3)) && (strlen(to) == 5))
  {
    nickserv_dbase_data *ns = nickserv_dbase_get_juped((to[3] - 'A') * 26 + (to[4] - 'A'));
    if (!ns) return 0;
    return com_message(sock, to, info->numeric, format, "This is a forbidden nick, reason: %s", ns->info);
  }
  
  /* Find the command, and uppercase it */
  cmd = getnext(params);

  cmd = uppercase(cmd);
  
  /* is this for ChanServ ? */
  if (!strcmp(to, conf->cs->numeric))
  {
    /* is this user authed ? */
    if (!info->nickserv)
      return com_message(sock, to, info->numeric, format, "You must be authenticated to Nickserv (%s) to use services.", conf->ns->nick);
    
    cmds = parser_chanserv_commands;
    strcpy(nick, conf->cs->nick);
  }
  else if (!strcmp(to, conf->ns->numeric))
  {
    cmds = parser_nickserv_commands;
    strcpy(nick, conf->ns->nick);
  }
  else if (!strcmp(to, conf->ms->numeric))
  {
    if (!info->nickserv)
      return com_message(sock, to, info->numeric, format, "You must be authenticated to Nickserv (%s) to use services.", conf->ns->nick);
    cmds = parser_multiserv_commands;
    strcpy(nick, conf->ms->nick);
  }
  else if (!strcmp(to, conf->os->numeric))
  {
    if (!info->nickserv) return 0;
    if (!operserv_have_access(info->nickserv->flags, BITS_OPERSERV_OPER)) return 0;
    cmds = parser_operserv_commands;
    strcpy(nick, conf->os->nick);
  }
  else return 0;
  
  if (!cmds) return 0;

  while (cmds->command)
  {
    if (!strcmp(cmd, cmds->command))
    {
      int res = cmds->func(sock, info, params, format, cmds);
      if (res == ERROR_NO_ACCESS)
        break;
      return res;
    }
    cmds++;
  }
  
  if ((*params) && (*nick))
    dcc_console_text('s', "[%s!%s@%s] %s: unknown command: %s %s", info->nick, info->username, info->host, nick, cmd, *params);
  else
    dcc_console_text('s', "[%s!%s@%s] %s: unknown command: %s", info->nick, info->username, info->host, nick, cmd);


  return com_message(sock, to, info->numeric, format, "Unknown command.");
}

/*
 **************************************************************************************************
 * parser_commands
 **************************************************************************************************
 *   Parser for the commands issued to nickserv, operserv, chanserv or multiserv
 **************************************************************************************************
 * Params:
 *   [IN]  sock_info *sock - the sock_info struct containing information about the individual socket
 *   [IN]  char *str    - the string from the server to parse
 *   [OUT] int <result> - 0 if success, else error-nr
 **************************************************************************************************
 */
int parser_dcc(sock_info *sock, char *str)
{
  char **params, *cmd;
  struct parser_command_data *cmds = parser_dcc_commands;
  
  if (*str == ':') str++;  
  while (*str == ' ') str++;

  params = &str;
  
  cmd = getnext(params);
  
  if (!strcmp(cmd, "")) return 0;
  
  cmd = uppercase(cmd);

  while (cmds->command)
  {
    if (!strcmp(cmd, cmds->command))
      return cmds->func(sock, sock->from, params, MODE_NOTICE, cmds);
    cmds++;
  }
  
  if (*params)
    dcc_console_text('s', "[%s!%s!%s@%s] [DCC] unknown command: %s %s", sock->from->nickserv->nick, sock->from->nick, sock->from->username, sock->from->host, cmd, *params);
  else
    dcc_console_text('s', "[%s!%s!%s@%s] [DCC] unknown command: %s", sock->from->nickserv->nick, sock->from->nick, sock->from->username, sock->from->host, cmd);
  return com_message(sock, NULL, NULL, NULL, "Unknown command.");
}

/*
 **************************************************************************************************
 * parser_commands
 **************************************************************************************************
 *   Parser for the commands issued to nickserv, operserv, chanserv or multiserv
 **************************************************************************************************
 * Params:
 *   [IN]  sock_info *sock - the sock_info struct containing information about the individual socket
 *   [IN]  char *str    - the string from the server to parse
 *   [IN]  char *format - the format in which the data to the user should be send
 *   [IN]  char *str - the string from the server to parse
 *   [OUT] int <result> - 0 if success, else error-nr
 **************************************************************************************************
 */
int parser_ctcp(sock_info *sock, char *to, char **params, dbase_nicks *from)
{
  char *cmd = getnext(params);
  
  cmd = uppercase(cmd);
  
  if (!strcmp(cmd, "PING"))
    return com_message(sock, to, from->numeric, MODE_NOTICE, "\001PING %s\001", getrest(params));
  if (!strcmp(cmd, "TIME"))
    return com_message(sock, to, from->numeric, MODE_NOTICE, "\001TIME %lu\001", time(0));
  if (!strcmp(cmd, "VERSION"))
  {
    com_message(sock, to, from->numeric, MODE_NOTICE, "\001VERSION %s IRC Services\001", NETWORK_NAME);
    com_message(sock, to, from->numeric, MODE_NOTICE, "\001VERSION   Compiled %s on %s\001", build_date, os_name);
  }
  if (!strcmp(cmd, "DCC"))
  {
    char buf[BUFFER_SIZE];
    char *chat1 = getnext(params);
    char *chat2 = getnext(params);
    
    if (strcmp(to, conf->os->numeric)) return 0;
    if (!from->nickserv) return 0;

    if (!(from->nickserv->flags & BITS_OPERSERV_DCC)) return 0;

    if ((!chat1) || (!chat2)) return 0;
    
    strcpy(buf, chat1);
    strcat(buf, chat2);
    
    uppercase(buf);
    
    if (!strcmp(buf, "CHATCHAT"))
    {
      char *ip = getnext(params);
      char *port_s = getnext(params);
      int port = tr_atoi(port_s);
      unsigned long lip = tr_atoi(ip);
      dcc_init_arg *args;
      pthread_t tread;
      
      if ((!ip) || (!port) || (!lip)) return 0;
      
      lip = ((lip & 0xff) << 24) | (((lip >> 8) & 0xff) << 16) | (((lip >> 16) & 0xff) << 8) | ((lip >> 24) & 0xff);
        
      args = (dcc_init_arg*)malloc(sizeof(dcc_init_arg));
      args->port = port;
      args->num = (char*)malloc(strlen(from->numeric) + 1);
      strcpy(args->num, from->numeric);
      args->ip = lip;
      pthread_create(&tread, NULL, dcc_init, args);
    }
  }
  return 0;
}

int parser_check_ignore(dbase_nicks *nick, char *to, char *format)
{
  if (nick->nickserv)
  {
    /* if CS admin, NS admin or above, don't ignore */
    if (operserv_have_access(nick->nickserv->flags, BITS_OPERSERV_NS_ADMIN)) return 0;
    if (operserv_have_access(nick->nickserv->flags, BITS_OPERSERV_CS_ADMIN)) return 0;
  }
  nick->ignore_lines++;
  if (!nick->ignore_ts[0])
    nick->ignore_ts[0] = (unsigned long)time(0);
  else
  {
    nick->ignore_ts[1] = (unsigned long)time(0);
    if (nick->ignore_lines >= IGNORE_LINES)
    {
      nick->ignore_lines = 0;

      if ((nick->ignore_ts[1] - nick->ignore_ts[0]) <= IGNORE_TIME)
      {
        timer_event *te = (timer_event*)malloc(sizeof(timer_event));

        nick->ignored = 1;
        
        com_message(irc, to, nick->numeric, format, "Received %d messages within %d seconds, you are being ignored for %d seconds!", IGNORE_LINES, IGNORE_TIME, IGNORE_LENGTH);
        
        te->data = malloc(strlen(nick->numeric) + 1);
        strcpy(te->data, nick->numeric);
        te->func = parser_remove_ignore;
        te->free = xfree;

        timer_add(te, IGNORE_LENGTH);
        return 1;
      }
      else
      { 
        nick->ignore_lines = 1;
        nick->ignore_ts[0] = (unsigned long)time(0);
      }
    }
    else
    { 
      if ((nick->ignore_ts[1] - nick->ignore_ts[0]) > IGNORE_TIME)
      {
        nick->ignore_lines = 1;
        nick->ignore_ts[0] = (unsigned long)time(0);
      }
    }
  }
  return 0; 
}

void parser_remove_ignore(void *ptr)
{
  char *num = (char*)ptr;
  dbase_nicks *nick = nicks_getinfo(num, NULL, -1);
  
  if (!nick) return;
  
  nick->ignored = 0;
  nick->ignore_lines = 0;
  nick->ignore_ts[0] = 0;
  nick->ignore_ts[1] = 0;
}
