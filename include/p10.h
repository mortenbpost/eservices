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
/* $Id: p10.h,v 1.1.1.1 2002/08/20 15:20:02 mr Exp $ */

#ifndef INC_P10_H
#define INC_P10_H

#include "parser.h"

int p10_end_of_burst              (sock_info *sock, char *from, char **params);
int p10_end_of_burst_acknowledge  (sock_info *sock, char *from, char **params);
int p10_ping                      (sock_info *sock, char *from, char **params);
int p10_squit                     (sock_info *sock, char *from, char **params);
int p10_nick                      (sock_info *sock, char *from, char **params);
int p10_burst                     (sock_info *sock, char *from, char **params);
int p10_join                      (sock_info *sock, char *from, char **params);
int p10_part                      (sock_info *sock, char *from, char **params);
int p10_create                    (sock_info *sock, char *from, char **params);
int p10_quit                      (sock_info *sock, char *from, char **params);
int p10_invite                    (sock_info *sock, char *from, char **params);
int p10_privmsg                   (sock_info *sock, char *from, char **params);
int p10_notice                    (sock_info *sock, char *from, char **params);
int p10_mode                      (sock_info *sock, char *from, char **params);
int p10_away                      (sock_info *sock, char *from, char **params);
int p10_topic                     (sock_info *sock, char *from, char **params);
int p10_kick                      (sock_info *sock, char *from, char **params);
int p10_kill                      (sock_info *sock, char *from, char **params);
int p10_server                    (sock_info *sock, char *from, char **params);
int p10_pass                      (sock_info *sock, char *from, char **params);
int p10_error                     (sock_info *sock, char *from, char **params);
int p10_wallops                   (sock_info *sock, char *from, char **params);
int p10_version                   (sock_info *sock, char *from, char **params);

#endif /* INC_P10_H */
