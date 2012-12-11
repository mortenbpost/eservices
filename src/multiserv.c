/****************************************************************************
* Exiled.net IRC Services                                                   *
* Copyright (C) 2002-2003  Michael Rasmussen <the_real@nerdheaven.dk>       *
*                          Morten Post <cure@nerdheaven.dk>                 *
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
/* $Id: multiserv.c,v 1.3 2003/02/21 23:17:45 mr Exp $ */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "setup.h"
#include "misc_func.h"
#include "dbase.h"
#include "config.h"
#include "server.h"
#include "queue.h"
#include "log.h"
#include "parser.h"
#include "chanserv.h"
#include "nickserv.h"
#include "errors.h"
#include "help.h"
#include "multiserv.h"

FUNC_COMMAND(multiserv_news)
{
  /* TODO Fetch from database */
  
  /*
  com_message(sock, conf->ms->numeric, from->numeric, format, "News for %s", NETWORK_NAME);
  com_message(sock, conf->ms->numeric, from->numeric, format, "1) 01/01/02 Services update (cure)");
  com_message(sock, conf->ms->numeric, from->numeric, format, "2) 02/01/02 New server linked (cure)");
  com_message(sock, conf->ms->numeric, from->numeric, format, "No news.");
  */
  return 0;
}

