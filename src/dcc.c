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
/* $Id: dcc.c,v 1.8 2003/02/18 14:47:30 cure Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "setup.h"
#include "server.h"
#include "config.h"
#include "misc_func.h"
#include "dbase.h"
#include "parser.h"
#include "errors.h"
#include "timer.h"
#include "chanserv.h"
#include "nickserv.h"
#include "operserv.h"
#include "multiserv.h"
#include "dcc.h"
#include "help.h"
#include "queue.h"
#include "log.h"

extern char *build_date;
extern char *os_name;


extern sock_info *irc;
extern int com_sock_array_count;
extern sock_info **com_sock_array;

extern struct parser_command_data parser_dcc_commands[];
  
extern help_db *help_operserv;
extern help_db *help_chanserv;
extern help_db *help_nickserv;

/**************************************************************************************************
 * dcc_help
 **************************************************************************************************
 *   HELP [command]
 *     Displays indepts help for [command] or genneral help, and a short list of commands 
 *     available, if [command] is not specified.
 *       [command] = getnext-string
 **************************************************************************************************
 * Params:
 *   [IN] sock_info *sock    : The socket from which the data was recieved
 *   [IN] dbase_nicks *from  : Pointer to the user who issued this command
 *   [IN] char **params      : The parameters to the command (to be used with getnext and getrest)
 *   [IN] char *format       : The format the message should be returned (privmsg or notice)
 * Return:
 *  [OUT] int return         : return 0 if success, anything else will cause the services to stop
 **************************************************************************************************/ 
FUNC_COMMAND(dcc_help)
{
  help_db *help_db;
  char *help;
  help = getnext(params);
  
  if(!help)
  {
    int i, acc;
    char buf[BUFFER_SIZE];
    com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_HELP_HEADER, conf->os->nick, conf->os->nick);
    
    acc = parser_dcc_commands[0].flags;
    i = 0;
    strcpy(buf, "");
    
    while (parser_dcc_commands[i].command)
    {
      if ((parser_dcc_commands[i].flags != acc) || (strlen(buf) > 70))
      {
        if (operserv_have_access(from->nickserv->flags, acc))
        {
          char flagsbuf[10];
          operserv_flags_to_str(acc, flagsbuf);
          com_message(sock, conf->os->numeric, from->numeric, format, "   OPER +%s:%s", flagsbuf, buf);
        }
        strcpy(buf, "");
      }
      acc = parser_dcc_commands[i].flags;
      
      strcat(buf, " ");
      strcat(buf, parser_dcc_commands[i].command);
      i++;
    }
    
    com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_HELP_FOOTER, conf->os->nick);
    log_command(LOG_OPERSERV, sock->from, "[DCC] HELP", "");
    
  }
  else
  {
    if ((help_db = help_search(help_operserv, help)))
      help_show(sock, from->numeric, conf->os->numeric, format, help_db);
    else if ((help_db = help_search(help_chanserv, help)))
      help_show(sock, from->numeric, conf->os->numeric, format, help_db);
    else if ((help_db = help_search(help_nickserv, help)))
      help_show(sock, from->numeric, conf->os->numeric, format, help_db);
    else
      return com_message(sock, conf->os->numeric, from->numeric, format, "No help available on %s", help);
      log_command(LOG_OPERSERV, sock->from, "[DCC] SYNTAX", queue_escape_string(help));
  }

  return 0;
}


/**************************************************************************************************
 * dcc_close
 **************************************************************************************************
 *   CLOSE
 *     Closes the DCC-connection
 **************************************************************************************************
 * Params:
 *   [IN] sock_info *sock    : The socket from which the data was recieved
 *   [IN] dbase_nicks *from  : Pointer to the user who issued this command
 *   [IN] char **params      : The parameters to the command (to be used with getnext and getrest)
 *   [IN] char *format       : The format the message should be returned (privmsg or notice)
 * Return:
 *  [OUT] int return         : return 0 if success, anything else will cause the services to stop
 **************************************************************************************************/ 
FUNC_COMMAND(dcc_close)
{
  log_command(LOG_OPERSERV, sock->from, "[DCC] CLOSE", "");
  com_message(sock, NULL, NULL, NULL, "Goodbye!");
  com_free(sock);
  return 0;
}

/**************************************************************************************************
 * dcc_say
 **************************************************************************************************
 *   SAY <message>
 *     Sends a message to the partyline, for everyone currently DCC-connected to see.
 *       <message> = getrest-string
 **************************************************************************************************
 * Params:
 *   [IN] sock_info *sock    : The socket from which the data was recieved
 *   [IN] dbase_nicks *from  : Pointer to the user who issued this command
 *   [IN] char **params      : The parameters to the command (to be used with getnext and getrest)
 *   [IN] char *format       : The format the message should be returned (privmsg or notice)
 * Return:
 *  [OUT] int return         : return 0 if success, anything else will cause the services to stop
 **************************************************************************************************/ 
FUNC_COMMAND(dcc_say)
{
  char *msg = getrest(params);
  if (!msg) return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);

  dcc_to_all(sock, "[%s] %s", sock->from->nickserv->nick, msg);

  log_command(LOG_OPERSERV, sock->from, "[DCC] SAY", queue_escape_string(msg));

  return 0;
}

/**************************************************************************************************
 * dcc_who
 **************************************************************************************************
 *   WHO
 *     Displays a list of user currently logged into the DCC interface.
 **************************************************************************************************
 * Params:
 *   [IN] sock_info *sock    : The socket from which the data was recieved
 *   [IN] dbase_nicks *from  : Pointer to the user who issued this command
 *   [IN] char **params      : The parameters to the command (to be used with getnext and getrest)
 *   [IN] char *format       : The format the message should be returned (privmsg or notice)
 * Return:
 *  [OUT] int return         : return 0 if success, anything else will cause the services to stop
 **************************************************************************************************/ 
FUNC_COMMAND(dcc_who)
{
  int i;
  
  com_message(sock, NULL, NULL, NULL, "Users currently logged into the DCC interface:");
  
  for (i = 0; i < com_sock_array_count; i++)
  {
    if (com_sock_array[i]->type == SOCK_DCC)
    {
      char access[BUFFER_SIZE];
      com_message(sock, NULL, NULL, NULL, "   %-9s (%s)", com_sock_array[i]->from->nickserv->nick, operserv_flags_to_title(com_sock_array[i]->from->nickserv->flags, access));
    }
  }

  log_command(LOG_OPERSERV, sock->from, "[DCC] WHO", "");

  return 0;
}

/**************************************************************************************************
 * dcc_console
 **************************************************************************************************
 *   CONSOLE [[+|-]modes]
 *     Set or displays the console modes for the DCC-interface
 **************************************************************************************************
 * Params:
 *   [IN] sock_info *sock    : The socket from which the data was recieved
 *   [IN] dbase_nicks *from  : Pointer to the user who issued this command
 *   [IN] char **params      : The parameters to the command (to be used with getnext and getrest)
 *   [IN] char *format       : The format the message should be returned (privmsg or notice)
 * Return:
 *  [OUT] int return         : return 0 if success, anything else will cause the services to stop
 **************************************************************************************************/ 
FUNC_COMMAND(dcc_console)
{
  char buf[BUFFER_SIZE];
  char *p, *modes = getnext(params);
  unsigned long flags = sock->from->nickserv->console;
  int add = 1;
  
  if (modes)
  {
    p = modes;
    
    while (*p)
    {
      if (*p == '+')
        add = 1;
      else if (*p == '-')
        add = 0;
      else if ((*p >= 'a') && (*p <= 'z'))
      {
        switch (*p)
        {
          case 'c': case 'n': case 'o': case 's': case 'y':
            if (add) bitadd(flags, *p - 'a');
            else bitdel(flags, *p - 'a');
            break;
          case 'b': case 'x': case 'z':
            if (operserv_have_access(sock->from->nickserv->flags, BITS_OPERSERV_SERVICES_SUB_ADMIN))
            {
              if (add) bitadd(flags, *p - 'a');
              else bitdel(flags, *p - 'a');
            }
            break;
          case 'j': case 'm': case 't': case 'w':
            if (operserv_have_access(sock->from->nickserv->flags, BITS_OPERSERV_CS_OPER))
            {
              if (add) bitadd(flags, *p - 'a');
              else bitdel(flags, *p - 'a');
            }
            break;
          case 'u': case 'v':
            if (operserv_have_access(sock->from->nickserv->flags, BITS_OPERSERV_NS_OPER))
            {
              if (add) bitadd(flags, *p - 'a');
              else bitdel(flags, *p - 'a');
            }
            break;
          case 'd':
            if (operserv_have_access(sock->from->nickserv->flags, BITS_OPERSERV_DEVELOPER))
            {
              if (add) bitadd(flags, *p - 'a');
              else bitdel(flags, *p - 'a');
            }
            break;
        }        
      }
      p++;
    }
  }
  
  sock->from->nickserv->console = flags;
  
  snprintf(buf, BUFFER_SIZE, "UPDATE nickdata SET console='%lu' WHERE nick='%s'", flags, queue_escape_string(sock->from->nickserv->nick));
  queue_add(buf);
  
  p = buf;
  
  for (add = 'a'; add <= 'z'; add++)
  {
    if (isbiton(sock->from->nickserv->console, add - 'a'))
      *p++ = add;
  }
  *p = '\0';
  com_message(sock, NULL, NULL, NULL, "Your current DCC console settings: %s", buf);
  
  
  if (modes)
    log_command(LOG_OPERSERV, sock->from, "[DCC] CONSOLE", modes);
  else
    log_command(LOG_OPERSERV, sock->from, "[DCC] CONSOLE", "");

  return 0;
}


/**************************************************************************************
 *                            DCC helper functions                                    *
 **************************************************************************************/ 

sock_info *dcc_init_connect(sock_info *sock, unsigned long ip, int port)
{
  struct sockaddr_in dest;

  dest.sin_family = AF_INET;
  dest.sin_port = htons(port);

  dest.sin_addr.s_addr = ip;
    
  memset(&(dest.sin_zero), 0, 8);

  if ((sock->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
  {
    com_free(sock);
    return NULL;
  }

  if (connect(sock->sockfd, (struct sockaddr *)&dest, sizeof(struct sockaddr)))
  {
    com_free(sock);
    return NULL;
  }

  return sock;
}


void *dcc_init(void *arg)
{
  dcc_init_arg *args = (dcc_init_arg*)arg;
  sock_info *sock;
  dbase_nicks *from;
  
  from = nicks_getinfo(args->num, NULL, -1);
  
  if (!from)
  {
    xfree(args->num);
    xfree(args);
    return 0;
  }
  
  if (!(sock = com_sock_create(SOCK_DCC)))
  {
    xfree(args->num);
    xfree(args);
    return 0;
  }
  
  if (!dcc_init_connect(sock, args->ip, args->port))
  {
    com_free(sock);
    xfree(args->num);
    xfree(args);
    return 0;
  }
  
  sock->from = from;
    
  com_message(sock, conf->os->numeric, from->numeric, MODE_NOTICE, "Welcome to %s DCC interface.", conf->host);
  com_message(sock, conf->os->numeric, from->numeric, MODE_NOTICE, "Compiled %s on %s", build_date, os_name);
  
  dcc_console_text('c', "[%s] connected to the DCC interface...", sock->from->nickserv->nick);

  xfree(args->num);
  xfree(args);
  return 0;
}

void dcc_free(dbase_nicks *nick)
{
  int i;
  for (i = 0; i < com_sock_array_count; i++)
  {
    if ((com_sock_array[i]->type == SOCK_DCC) && (com_sock_array[i]->from == nick))
    {
      com_free(com_sock_array[i]);
      i--;
    }
  }  
}

void dcc_to_all(sock_info *from, char *str, ...)
{
  int i;
  
  char buf[2*BUFFER_SIZE];
  va_list ag;

  if (!str) return;

  va_start(ag, str);
  vsnprintf(buf, 2*BUFFER_SIZE, str, ag);
  va_end(ag);
  
  for (i = 0; i < com_sock_array_count; i++)
  {
    if ((com_sock_array[i]->type == SOCK_DCC) && (com_sock_array[i] != from))
    {
      com_message(com_sock_array[i], NULL, NULL, NULL, buf);
    }
  }
}


void dcc_on_free(sock_info *sock)
{
  if (!sock) return;
  if (!sock->from) return;
  dcc_console_text('c', "[%s] disconnected from the DCC interface...", sock->from->nickserv->nick);
}

void dcc_console_text(char mode, char *str, ...)
{
  int i;
  
  char buf[2*BUFFER_SIZE];
  va_list ag;

  if (!str) return;

  va_start(ag, str);
  vsnprintf(buf, 2*BUFFER_SIZE, str, ag);
  va_end(ag);
  
  for (i = 0; i < com_sock_array_count; i++)
  {
    if ((com_sock_array[i]->type == SOCK_DCC) && (com_sock_array[i]->from))
    {
      if (isbiton(com_sock_array[i]->from->nickserv->console, mode - 'a'))
        com_message(com_sock_array[i], NULL, NULL, NULL, "[+%c] %s", mode, buf);
    }
  }
}
