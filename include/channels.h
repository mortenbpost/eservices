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
/* $Id: channels.h,v 1.1.1.1 2002/08/20 15:20:02 mr Exp $ */

#ifndef INC_CHANNELS_H
#define INC_CHANNELS_H

#include "dbase.h"

long channels_search(const char *name);

long channels_add(const char *name, long createtime);

long channels_setmode(long index, const char *name, char *modes);

long channels_settopic(long index, const char *name, char *topic);

long channels_userjoin(long index, const char *name, const char *mode, const char *numeric);
long channels_userpart(long index, const char *name, const char *numeric);
long channels_usermode(long index, const char *name, const char *mode, const char *numeric);

long channels_remove(long index, const char *name);

long channels_addlimit(long index, const char *name, long limit);
long channels_remlimit(long index, const char *name);

long channels_addkey(long index, const char *name, char *key);
long channels_remkey(long index, const char *name, const char *key);

long channels_addban(long index, const char *name, char *bans);
long channels_remban(long index, const char *name, char *bans);
long channels_remove_covered_ban(long index, const char *name, const char *ban);
int channels_check_ban_covered(long index, const char *name, const char *ban);

dbase_channels *channels_getinfo(long index, const char *name);
long channels_getcount(void);

long channels_internal_search(long low, long high, const char *name);
long channels_user_search(dbase_channels_nicks **arr, long low, long high,  const char *numeric);

long channels_user_find(const char *chan, const char *numeric);

void channels_cleanup(void);


#endif /* INC_CHANNELS_H */
