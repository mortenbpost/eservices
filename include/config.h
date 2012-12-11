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
/* $Id: config.h,v 1.3 2003/02/08 15:48:33 cure Exp $ */

#ifndef INC_CONFIG_H
#define INC_CONFIG_H

#include <time.h>

/* Prototypes */
int conf_load(void);
int conf_unload(void);

/* Structures */
typedef struct services
{
  char *nick;
  char *username;
  char *userinfo;
  char numeric[6];
} services;

typedef struct conf_mysql
{
  char *database; /* database name    */
  char *username; /* username         */
  char *password; /* password         */
  char *unixsock; /* unix-socket file */
  char *host;     /* host name        */
  int port;       /* port             */
} conf_mysql;

typedef struct config
{
  char *uplink;
  char *host;
  char *bind;
  char *pass;
  char numeric[3];
  short int port;
  time_t starttime;
  int servertype;

  services *ns, *cs, *os, *ms;
  char *logfile;
  conf_mysql *mysql;
} config;

extern config *conf;

#endif /* INC_CONFIG_H */
