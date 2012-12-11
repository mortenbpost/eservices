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
/* $Id: multiserv.h,v 1.2 2003/02/25 22:41:04 cure Exp $ */

#ifndef INC_MULTISERV_H
#define INC_MULTISERV_H

#include "parser.h"

/* MULTISERV_SYNTAX */
#define MULTISERV_SYNTAX_HELP   "Syntax: HELP [command]"
#define MULTISERV_SYNTAX_STATS  "Syntax: STATS TODO"

/* user commands */
FUNC_COMMAND(multiserv_help);
FUNC_COMMAND(multiserv_stats);

#endif /* INC_MULTISERV_H */
