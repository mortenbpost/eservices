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
/* $Id: server.c,v 1.6 2003/03/01 00:32:20 mr Exp $ */

#include <setup.h>

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
#include <arpa/inet.h>
#include <pthread.h>

#include "server.h"
#include "config.h"
#include "misc_func.h"
#include "parser.h"
#include "errors.h"
#include "timer.h"
#include "p10.h"
#include "dcc.h"

extern sock_info *irc;

int com_sock_array_count = 0;
sock_info **com_sock_array = NULL;

pthread_mutex_t sock_mutex;

/* Is there a server currently bursting ?
   If yes, stall timer events */
int com_burst = 0;

/*
 ****************************************************************************************
 * function-name
 ****************************************************************************************
 *   Function description
 ****************************************************************************************
 * Params:
 *   [IN]  type name - description
 *   [OUT] type name - description
 *   [OUT] type <result> - return value description
 ****************************************************************************************
 */


sock_info *com_sock_create(sock_type type)
{
  sock_info *sock;
  
  pthread_mutex_lock(&sock_mutex);
  
  if ((sock = (sock_info *) malloc(sizeof(sock_info))))
  {
    sock->type = type;
    sock->datarecv = 0;
    sock->datasend = 0;
    sock->lines    = 0;
    sock->sockfd   = INVALID_SOCKET;
    sock->from     = NULL;
    com_sock_array = (sock_info **) realloc(com_sock_array, com_sock_array_count+1);
    com_sock_array[com_sock_array_count] = sock;
    memset(&sock->buffer, 0, BUFFER_SIZE);
    com_sock_array_count++;
  }
  
  pthread_mutex_unlock(&sock_mutex);

  return sock;
}

sock_info *com_connect(sock_info *sock)
{
  struct sockaddr_in dest;
  struct hostent *hostinfo;
  long t;

  if ((hostinfo = gethostbyname(conf->uplink)) == NULL) return NULL;

  dest.sin_family = hostinfo->h_addrtype;
  dest.sin_port = htons(conf->port);

  dest.sin_addr = *((struct in_addr *)hostinfo->h_addr);
  memset(&(dest.sin_zero), 0, 8);

  if ((sock->sockfd = socket(hostinfo->h_addrtype, SOCK_STREAM, 0)) == INVALID_SOCKET)
  {
    xfree(sock);
    return NULL;
  }

  if (conf->bind)
  {
    struct sockaddr_in virtual;

    memset(&virtual, 0, sizeof(virtual));

    virtual.sin_family = AF_INET;
    virtual.sin_addr.s_addr = INADDR_ANY;
   
    virtual.sin_addr.s_addr = inet_addr(conf->bind);
  
    if (virtual.sin_addr.s_addr == INADDR_NONE)
      virtual.sin_addr.s_addr = INADDR_ANY;

    bind(sock->sockfd, (struct sockaddr *)&virtual, sizeof(virtual));
  }

  if (connect(sock->sockfd, (struct sockaddr *)&dest, sizeof(struct sockaddr)))
  {
    xfree(sock);
    return NULL;
  }

  t = time(0);
  com_send(irc, "PASS :%s\r\n", conf->pass);
  com_send(irc, "SERVER %s 1 %ld %ld J10 %sKA] + :%s IRC Services\r\n", conf->host, t, t, conf->numeric, NETWORK_NAME);
  conf->starttime = t;
  com_burst = 1;

  return sock;
}

int com_mainloop(void)
{
  int sel = 0, i, res, nfds = 0;
  fd_set rset;
  struct timeval timeout;

  while(1)
  {
    timeout.tv_sec =  3;
    timeout.tv_usec = 0;

    FD_ZERO(&rset);
    nfds = 0;
    
    pthread_mutex_lock(&sock_mutex);
    for (i = 0; i < com_sock_array_count; i++)
    {
      if (com_sock_array[i]->sockfd == INVALID_SOCKET) continue;
      FD_SET(com_sock_array[i]->sockfd, &rset);
      if (com_sock_array[i]->sockfd > nfds) nfds = com_sock_array[i]->sockfd;
    }
    pthread_mutex_unlock(&sock_mutex);

    nfds++;

    sel = select(nfds, &rset, NULL, NULL, &timeout);

    if (sel > 0)
    {
      for (i = 0; i < com_sock_array_count; i++)
      {
        if (FD_ISSET(com_sock_array[i]->sockfd, &rset))
        {
          if ((res = com_recieve(com_sock_array[i])))
          {
            if (com_sock_array[i]->type == SOCK_SERVER) return res;
            else com_free(com_sock_array[i--]);
          }
        }
      }
    }
    else if (sel < 0)
    {
      /* if dcc / telnet close/free */
      /* if server - try to free all and reconnect */
    }
    if (!com_burst) timer_check();
  }

  return 0;
}

int com_free(sock_info *sock)
{
  int i;
  for (i = 0; i < com_sock_array_count; i++)
  {
    if (com_sock_array[i] == sock)
    {
      if (sock->type == SOCK_DCC)
        dcc_on_free(sock);
      
      close(sock->sockfd);
      
      memmove(&com_sock_array[i], &com_sock_array[i+1], (com_sock_array_count - i - 1) * sizeof(sock_info*));
      com_sock_array = (sock_info**)realloc(com_sock_array, (--com_sock_array_count) * sizeof(sock_info*));
      
      debug_out("Closing socket, buffer: %s\n", sock->buffer);

      xfree(sock);
      return 0;
    }
  }
  return 0;
}


int com_free_all(void)
{
  int i;
  for (i = 0; i < com_sock_array_count; i++)
  {
    close(com_sock_array[i]->sockfd);
    xfree(com_sock_array[i]);
  }
  xfree(com_sock_array);
  com_sock_array = NULL;
  com_sock_array_count = 0;
  return 0;
}


int com_recieve(sock_info *sock)
{
  char in[BUFFER_SIZE], *p;
  int recv_len = 0, res;
  int pre = strlen(sock->buffer);

  recv_len = read(sock->sockfd, in, sizeof(in) - pre - 1);

  if (recv_len > 0)
    sock->datarecv += recv_len;
  else
    return  ERROR_COM_CONNECTION_LOST;

  in[recv_len] = '\0';

  strcat(sock->buffer, in);
  pre = strlen(sock->buffer);
  memset(sock->buffer+pre, 0, BUFFER_SIZE - pre);

  while ((p = strchr(sock->buffer, '\xa')))
  {
    *p-- = '\0';
    if (*p == '\xd') *p = '\0';
    p++; p++;
    sock->lines++;
    switch (sock->type)
    {
      case SOCK_SERVER:
        debug_out("[RAW/IN]  %s\n", sock->buffer);
        if ((res = parser_p10(sock, sock->buffer))) return res;
        break;
      case SOCK_DCC:
        debug_out("[DCC/IN]  [%s] %s\n", sock->from->nickserv->nick, sock->buffer);
        parser_dcc(sock, sock->buffer);
        break;
      case SOCK_TELNET:
        /*parser_server(sock, sock->buffer);*/
        break;
    }
    memmove(sock->buffer, p, BUFFER_SIZE - (p - sock->buffer));
  }
  return 0;
}

int com_send(sock_info *sock, const char *string, ...)
{
  char buffer[BUFFER_SIZE];
  va_list ag;

  if (!string) return 0;
  if (!sock) return 0;

  va_start(ag, string);
  vsnprintf(buffer, BUFFER_SIZE, string, ag);
  va_end(ag);

  if (sock->type == SOCK_SERVER)
    debug_out("[RAW/OUT] %s", buffer);
  else if (sock->type == SOCK_DCC)
    debug_out("[DCC/OUT] [%s] %s", sock->from->nickserv->nick, buffer);

  sock->datasend += send(sock->sockfd, buffer, strlen(buffer), 0);
  return 0;
}

int com_message(sock_info *sock, char *from, char *to, char *format, char *string, ...)
{
  char buffer[10*BUFFER_SIZE], *buf, *p;
  va_list ag;

  if (!string) return 0;

  memset(buffer, 0, sizeof(buffer));

  va_start(ag, string);
  vsnprintf(buffer, 10*BUFFER_SIZE, string, ag);
  va_end(ag);

  buf = buffer;

  while ((p = strchr(buf, '\xa')))
  {
    *p++ = '\0';
    if (sock->type == SOCK_SERVER)
      com_send(sock, "%s %s %s :%s\n", from, format, to, buf);
    else
      com_send(sock, "%s\n", buf);
    buf = p;
  }
  if (*buf)
  {
    if (sock->type == SOCK_SERVER)
      com_send(sock, "%s %s %s :%s\n", from, format, to, buf);
    else
      com_send(sock, "%s\n", buf);
  }

  return 0;
}

int com_wallops(char *sender, const char *string, ...)
{
  char buffer[BUFFER_SIZE], *p;
  va_list ag;

  if (!string) return 0;
  if (!irc) return 0;

  va_start(ag, string);
  vsnprintf(buffer, BUFFER_SIZE, string, ag);
  va_end(ag);
  
  p = buffer;
  p10_wallops(irc, sender, &p);
  com_send(irc, "%s WA :%s\n", sender, buffer);

  return 0;
}
