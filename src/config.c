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
/* $Id: config.c,v 1.2 2003/01/19 02:15:40 mr Exp $ */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "setup.h"
#include "config.h"
#include "misc_func.h"

config *conf;

int conf_load(void)
{
  FILE *fp;
  char buffer[512], *p, *ptr;
  int i;

  if ((fp = fopen(CONFIG_FILE, "r")) == NULL)
  {
    /* error reading from CONFIG_FILE */
    perror("-CONFIG_FILE Error");
    fprintf(stderr, "-Shutting down services.\n");
    return 1;
  }

  conf = (config *) malloc(sizeof(config));

  memset(conf, 0, sizeof(config));

  conf->ns = (services *) malloc(sizeof(services));
  conf->cs = (services *) malloc(sizeof(services));
  conf->os = (services *) malloc(sizeof(services));
  conf->ms = (services *) malloc(sizeof(services));
  conf->mysql = (conf_mysql *) malloc(sizeof(conf_mysql));

  memset(conf->ns, 0, sizeof(services));
  memset(conf->cs, 0, sizeof(services));
  memset(conf->os, 0, sizeof(services));
  memset(conf->ms, 0, sizeof(services));
  memset(conf->mysql, 0, sizeof(conf_mysql));

  while (fgets(buffer, 512, fp))
  {
    p = buffer;
    while (*p == ' ') p++;
    if ((*p == '\n') || (*p == '#')) continue;
    strip_rn(p);
    if ((ptr = strchr(p, ' '))) *ptr++ = '\0';
    else continue;
    while (*ptr == ' ') *ptr++ = '\0';
    i = strlen(ptr) - 1;
    while (ptr[i] == ' ') ptr[i--] = '\0';

    /* Parsing CONFIG_FILE into main struct */

    if (!strcasecmp(p, "UPLINK"))
    {
      conf->uplink = (char*)malloc(sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->uplink, ptr);
    }
    else if (!strcasecmp(p, "PORT"))
    {
      conf->port = tr_atoi(ptr);
    }
    else if (!strcasecmp(p, "HOST"))
    {
      conf->host = (char*)realloc(conf->host, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->host, ptr);
    }
    else if (!strcasecmp(p, "BIND"))
    {
      conf->bind = (char*)realloc(conf->bind, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->bind, ptr);
    }
    else if (!strcasecmp(p, "PASS"))
    {
      conf->pass = (char*)realloc(conf->pass, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->pass, ptr);
    }
    else if (!strcasecmp(p, "NUMERIC"))
    {
      strcpy(conf->numeric, ptr);
    }

    else if (!strcasecmp(p, "LOGFILE"))
    {
      conf->logfile = (char*)realloc(conf->logfile, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->logfile, ptr);
    }

    /* Parsing CONFIG_FILE into NICKSERV struct */

    else if (!strcasecmp(p, "NICKSERV_NICK"))
    {
      conf->ns->nick = (char*)realloc(conf->ns->nick, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->ns->nick, ptr);
    }
    else if (!strcasecmp(p, "NICKSERV_USERNAME"))
    {
      conf->ns->username = (char*)realloc(conf->ns->username, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->ns->username, ptr);
    }
    else if (!strcasecmp(p, "NICKSERV_USERINFO"))
    {
      conf->ns->userinfo = (char*)realloc(conf->ns->userinfo, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->ns->userinfo, ptr);
    }

    /* Parsing CONFIG_FILE into CHANSERV struct */

    else if (!strcasecmp(p, "CHANSERV_NICK"))
    {
      conf->cs->nick = (char*)realloc(conf->cs->nick, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->cs->nick, ptr);
    }
    else if (!strcasecmp(p, "CHANSERV_USERNAME"))
    {
      conf->cs->username = (char*)realloc(conf->cs->username, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->cs->username, ptr);
    }
    else if (!strcasecmp(p, "CHANSERV_USERINFO"))
    {
      conf->cs->userinfo = (char*)realloc(conf->cs->userinfo, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->cs->userinfo, ptr);
    }

    /* Parsing CONFIG_FILE into OPERSERV struct */

    else if (!strcasecmp(p, "OPERSERV_NICK"))
    {
      conf->os->nick = (char*)realloc(conf->os->nick, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->os->nick, ptr);
    }
    else if (!strcasecmp(p, "OPERSERV_USERNAME"))
    {
      conf->os->username = (char*)realloc(conf->os->username, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->os->username, ptr);
    }
    else if (!strcasecmp(p, "OPERSERV_USERINFO"))
    {
      conf->os->userinfo = (char*)realloc(conf->os->userinfo, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->os->userinfo, ptr);
    }

    /* Parsing CONFIG_FILE into MULTISERV struct */

    else if (!strcasecmp(p, "MULTISERV_NICK"))
    {
      conf->ms->nick = (char*)realloc(conf->ms->nick, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->ms->nick, ptr);
    }
    else if (!strcasecmp(p, "MULTISERV_USERNAME"))
    {
      conf->ms->username = (char*)realloc(conf->ms->username, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->ms->username, ptr);
    }
    else if (!strcasecmp(p, "MULTISERV_USERINFO"))
    {
      conf->ms->userinfo = (char*)realloc(conf->ms->userinfo, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->ms->userinfo, ptr);
    }

    /* Parsing CONFIG_FILE into CONF_MYSQL struct */

    else if (!strcasecmp(p, "MYSQL_DATABASE"))
    {
      conf->mysql->database = (char*)realloc(conf->mysql->database, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->mysql->database, ptr);
    }
    else if (!strcasecmp(p, "MYSQL_USERNAME"))
    {
      conf->mysql->username = (char*)realloc(conf->mysql->username, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->mysql->username, ptr);
    }
    else if (!strcasecmp(p, "MYSQL_PASSWORD"))
    {
      conf->mysql->password = (char*)realloc(conf->mysql->password, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->mysql->password, ptr);
    }
    else if (!strcasecmp(p, "MYSQL_UNIXSOCK"))
    {
      conf->mysql->unixsock = (char*)realloc(conf->mysql->unixsock, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->mysql->unixsock, ptr);
    }
    else if (!strcasecmp(p, "MYSQL_HOST"))
    {
      conf->mysql->host = (char*)realloc(conf->mysql->host, sizeof(char)*(strlen(ptr)+1));
      strcpy(conf->mysql->host, ptr);
    }
    else if (!strcasecmp(p, "MYSQL_PORT"))
    {
      conf->mysql->port = tr_atoi(ptr);
    }
  }


  strcpy(conf->ns->numeric, conf->numeric);
  strcat(conf->ns->numeric, "AAN");

  strcpy(conf->cs->numeric, conf->numeric);
  strcat(conf->cs->numeric, "AAC");

  strcpy(conf->os->numeric, conf->numeric);
  strcat(conf->os->numeric, "AAO");

  strcpy(conf->ms->numeric, conf->numeric);
  strcat(conf->ms->numeric, "AAM");

  return 0;

}

int conf_unload(void)
{
  if (conf)
  {
    if (conf->ns)
    {
      xfree(conf->ns->nick);
      xfree(conf->ns->username);
      xfree(conf->ns->userinfo);
      xfree(conf->ns);
    }
    if (conf->cs)
    {
      xfree(conf->cs->nick);
      xfree(conf->cs->username);
      xfree(conf->cs->userinfo);
      xfree(conf->cs);
    }
    if (conf->os)
    {
      xfree(conf->os->nick);
      xfree(conf->os->username);
      xfree(conf->os->userinfo);
      xfree(conf->os);
    }
    if (conf->ms)
    {
      xfree(conf->ms->nick);
      xfree(conf->ms->username);
      xfree(conf->ms->userinfo);
      xfree(conf->ms);
    }
    if (conf->mysql)
    {
      xfree(conf->mysql->database);
      xfree(conf->mysql->username);
      xfree(conf->mysql->password);
      xfree(conf->mysql->unixsock);
      xfree(conf->mysql->host);
      xfree(conf->mysql);
    }
    xfree(conf->logfile);
    xfree(conf->uplink);
    xfree(conf->host);
    xfree(conf->bind);
    xfree(conf->pass);
    xfree(conf);
  }

  /* pusssha! */

  return 0;
}



