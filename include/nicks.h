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
/* $Id: nicks.h,v 1.1.1.1 2002/08/20 15:20:02 mr Exp $ */

#ifndef INC_NICKS_H
#define INC_NICKS_H

long nicks_search_nick(const char *nick);
long nicks_search_numeric(const char *numeric);

long nicks_add(dbase_nicks *data, char *modes);
long nicks_remove(const char *numeric);
long nicks_renick(const char *numeric, const char *newnick);

long nicks_setmode(const char *numeric, char *modes);
long nicks_setaway(const char *numeric, const char *awaymsg);

long nicks_add(dbase_nicks *data, char *modes);

dbase_nicks *nicks_getinfo(const char *numeric, const char *nick, int nr);
const char *nicks_getnick(const char *numeric);
const char *nicks_getnum(const char *nick);

long nicks_join_channel(const char *numeric, dbase_channels_nicks *cn);
long nicks_part_channel(const char *numeric, dbase_channels_nicks *cn);

long nicks_getcount(void);

long nicks_internal_search(long low, long high, const char *nick);
long nicks_internal_num_search(long low, long high, const char *numeric);

void nicks_dump(void);

#endif /* INC_NICKS_H */
