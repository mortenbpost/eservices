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
/* $Id: errors.h,v 1.1.1.1 2002/08/20 15:20:02 mr Exp $ */

#ifndef INC_ERRORS_H
#define INC_ERRORS_H

#define ERROR_COM_QUIT 1
#define ERROR_COM_CONNECTION_LOST 2

#define ERROR_LOG_NOT_DEFINED 1000
#define ERROR_LOG_ERROR_OPENING_FILE 1001

#define ERROR_DBASE_MYSQL_ERROR 2000

#define ERROR_SERVER_COULD_NOT_CONNECT 4000

#define ERROR_OPERSERV_DIE 5000

#define ERROR_NO_ACCESS       0x7fffffff
#define ERROR_UNKNOWN_COMMAND 0x7fffffff

struct err_struct
{
  int errnr;
  char *errstr;
};

char *error_getstring(int errnr);

#endif /* INC_ERRORS_H */
