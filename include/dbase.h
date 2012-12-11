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
/* $Id: dbase.h,v 1.6 2003/02/25 23:48:39 mr Exp $ */

#ifndef INC_DBASE_H
#define INC_DBASE_H
/*

| BIT |   NickServ   |   ChanServ   | BIT |   OperServ   |   ChanServ   |
+-----+--------------+--------------+-----+--------------+--------------+
|  0  |   Is Oper    |   Disabled   | 16  |    GLine     |              |
|  1  | Use privmsg  |  Hide owner  | 17  |    Modes     |              |
|  2  |    Secret    |              | 18  |              |              |
|  3  |  No Expire   |   No Expire  | 19  |              |              |
|  4  |              |              | 20  |              |              |
|  5  |              |              | 21  |              |              |
|  6  |              |              | 22  |              |              |
|  7  |              |              | 23  |              |              |
+-----+--------------+--------------+-----+--------------+--------------+
|  8  |              |              | 24  |   NS Oper    |              |
|  9  |              |              | 25  |   CS Oper    |              |
| 10  |              |              | 26  |  Developer   |              |
| 11  |              |              | 27  |     DCC      |              |
| 12  |              |              | 28  |   NS Admin   |              |
| 13  |              |              | 29  |   CS Admin   |              |
| 14  |              |              | 30  |  Sub-Admin   |              |
| 15  |    Juped     |   Expired    | 31  |    Admin     |              |
+-----+--------------+--------------+-----+--------------+--------------+

*/

#define BITS_NICKSERV_OPER               0x00000001
#define BITS_NICKSERV_PRIVMSG            0x00000002
#define BITS_NICKSERV_SECRET             0x00000004
#define BITS_NICKSERV_NOEXPIRE           0x00000008

#define BITS_NICKSERV_JUPED              0x00008000

#define BITS_CHANSERV_DISABLED           0x00000001
#define BITS_CHANSERV_NOEXPIRE           0x00000008
#define BITS_CHANSERV_EXPIRED            0x00008000

#define BITS_OPERSERV_OPER               0x00000001

#define BITS_OPERSERV_GLINE              0x00010000
#define BITS_OPERSERV_MODES              0x00020000

#define BITS_OPERSERV_NS_OPER            0x01000000
#define BITS_OPERSERV_CS_OPER            0x02000000
#define BITS_OPERSERV_DEVELOPER          0x04000000
#define BITS_OPERSERV_DCC                0x08000000

#define BITS_OPERSERV_NS_ADMIN           0x10000000
#define BITS_OPERSERV_CS_ADMIN           0x20000000
#define BITS_OPERSERV_SERVICES_SUB_ADMIN 0x40000000
#define BITS_OPERSERV_SERVICES_ADMIN     0x80000000

typedef struct dbase_comment
{
  char *nick;
  char *comment;
  long date;
} dbase_comment;

typedef struct chanserv_dbase_bans
{
  char *mask;
  char *nick;
  long expire;
} chanserv_dbase_bans;

typedef struct nickserv_data
{
  char *nick;
  char *password;

  char *userhost;

  long regdate;
  long lastlogin;

  char *email;
  char *info;
  unsigned long flags;
  unsigned long console;

  struct dbase_nicks *entry;
  
  struct chanserv_access **access;
  unsigned long access_count;
    
  struct dbase_comment **comments;
  unsigned long comment_count;
  
  struct dbase_comment **notices;
  unsigned long notice_count;

} nickserv_dbase_data;

typedef struct chanserv_access
{
  struct nickserv_data *nick;
  struct chanserv_channel *channel;
  int level;
  int autoop;
} chanserv_dbase_access;

typedef struct chanserv_channel
{
  char *name;

  char *owner;
  char *topic;
  char *keepmode;

  unsigned long flags;

  long lastlogin;
  
  struct chanserv_dbase_bans **bans;
  unsigned long bancount;

  struct chanserv_access **access;
  unsigned long access_count;
    
  struct dbase_comment **comments;
  unsigned long comment_count;
  
} chanserv_dbase_channel;

typedef struct dbase_channels_nicks dbase_channels_nicks;

typedef struct dbase_channels
{
  char *name;
  char *topic;

  unsigned long modes;
  char *key;
  long limit;

  unsigned long createtime;

  long usercount;
  dbase_channels_nicks **users;

  long bancount;
  char **bans;

  chanserv_dbase_channel *chanserv;
} dbase_channels;

struct dbase_channels_nicks
{
  long mode; /* = 0 normal, 1 = voice, 2 = op, 3 = voice & op */
  struct dbase_nicks *nick;
  struct dbase_channels *channel;
};

typedef struct dbase_nicks
{
  char *nick;
  char *username;
  char *host;
  char *userinfo;
  char *away;
  char *numeric;
  unsigned long IP;
  unsigned long timestamp;
  unsigned long modes;
  long hopcount;
  dbase_channels_nicks **channels;
  long channels_count;
  nickserv_dbase_data *nickserv;
  
  int ignored;
  int ignore_lines;
  unsigned long ignore_ts[2];
} dbase_nicks;

#include "nicks.h"
#include "channels.h"

void dbase_clear(void);
int dbase_load_persistant(void);

#endif /* INC_DBASE_H */
