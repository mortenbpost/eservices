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
/* $Id: setup.h,v 1.5 2003/02/25 23:44:28 mr Exp $ */

#ifndef INC_SETUP_H
#define INC_SETUP_H

#include "../defines.h"

/* Default buffer size */
#define BUFFER_SIZE 712
#define MAX_NICK_LEN 35
/* 
   Defines used in the auto ignoring feature, which will ignore
   noisy/spammy users.

   IGNORE_LINES  is the number of lines that services will accept from
                 a user in IGNORE_TIME
   IGNORE_LENGTH is the time that the user will be ignored for.

   All times are in seconds.
 */ 
#define IGNORE_LINES    5
#define IGNORE_TIME    15
#define IGNORE_LENGTH 120

/*
   These defines are message tokens.
   If you change these the service might not work properly!
 */
#define MODE_PRIVMSG "P"
#define MODE_NOTICE  "O"

/* Defines the NETWORK_NAME */
#define NETWORK_NAME "Exiled.Net"

/* Defines the config file which is going to be used */
#define CONFIG_FILE "services.conf"

/* DEBUG macros 
   You should under no cirsumstances change these!
 */
#ifdef NDEBUG
#define debug_out(x...)
#else
#define debug_out(x...) printf(x)
#endif

#endif /* INC_SETUP_H */
