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
/* $Id: chanserv.h,v 1.14 2003/02/25 23:49:50 mr Exp $ */

#ifndef INC_CHANSERV_H
#define INC_CHANSERV_H

#include "parser.h"

/* The time (in seconds) the should have passed since lastlogin for an account to expire */
#define CHANSERV_EXPIRE_TIME    (30 * 24 * 60 * 60)
/* How often should we check for expired nicks (in seconds) */
#define CHANSERV_EXPIRE_CHECK   (24 * 60 * 60)


/* CHANSERV_LEVEL */
#define CHANSERV_LEVEL_REMOVEME    1
#define CHANSERV_LEVEL_INVITE     50
#define CHANSERV_LEVEL_TOPIC      50
#define CHANSERV_LEVEL_VOICE      50
#define CHANSERV_LEVEL_DEVOICE    50
#define CHANSERV_LEVEL_OP         100
#define CHANSERV_LEVEL_DEOP       100
#define CHANSERV_LEVEL_OPME       100
#define CHANSERV_LEVEL_DEOPME     100
#define CHANSERV_LEVEL_KICK       200
#define CHANSERV_LEVEL_KICKBAN    200
#define CHANSERV_LEVEL_UNBAN      300
#define CHANSERV_LEVEL_BAN        300
#define CHANSERV_LEVEL_BANLIST    300
#define CHANSERV_LEVEL_ACCESS     400
#define CHANSERV_LEVEL_AUTOOP     400
#define CHANSERV_LEVEL_CLEAROPS   450
#define CHANSERV_LEVEL_CLEARMODES 450
#define CHANSERV_LEVEL_CYCLE      450
#define CHANSERV_LEVEL_STATUS     450
#define CHANSERV_LEVEL_CLEAR      500
#define CHANSERV_LEVEL_CHOWNER    500
#define CHANSERV_LEVEL_DROP       500

#define CHANSERV_LEVEL_OWNER      500

/* CHANSERV_SYNTAX */
#define CHANSERV_SYNTAX_HELP              "Syntax: HELP [command]"
#define CHANSERV_SYNTAX_INFO              "Syntax: INFO <#channel>"
#define CHANSERV_SYNTAX_REGISTER          "Syntax: REGISTER <#channel>"
#define CHANSERV_SYNTAX_SHOWCOMMANDS      "Syntax: SHOWCOMMANDS <#channel>"

#define CHANSERV_SYNTAX_INVITE            "Syntax: INVITE <#channel>"
#define CHANSERV_SYNTAX_REMOVEME          "Syntax: REMOVEME <#channel> <password>"

#define CHANSERV_SYNTAX_VOICE             "Syntax: VOICE <#channel> [{nickname,...}]"
#define CHANSERV_SYNTAX_DEVOICE           "Syntax: DEVOICE <#channel> [{nickname,...}]"
#define CHANSERV_SYNTAX_TOPIC             "Syntax: TOPIC <#channel> <topic>"

#define CHANSERV_SYNTAX_OP                "Syntax: OP <#channel> [{nickname,...}]"
#define CHANSERV_SYNTAX_OPME              "Syntax: OPME"
#define CHANSERV_SYNTAX_DEOP              "Syntax: DEOP <#channel> [{nickname,...}]"
#define CHANSERV_SYNTAX_DEOPME            "Syntax: DEOPME"

#define CHANSERV_SYNTAX_KICK              "Syntax: KICK <#channel> <nickname> <reason>"
#define CHANSERV_SYNTAX_KICKBAN           "Syntax: KICKBAN <#channel> <nickname> <reason>"

#define CHANSERV_SYNTAX_BANLIST           "Syntax: BANLIST <#channel>"
#define CHANSERV_SYNTAX_BAN               "Syntax: BAN <#channel> <banmask> [duration]"
#define CHANSERV_SYNTAX_UNBAN             "Syntax: UNBAN <#channel> <id>"

#define CHANSERV_SYNTAX_ACCESS            "Syntax: ACCESS <#channel> <SET|REM|LIST> <registered nickname> [<level>|<registered nickname>]"
#define CHANSERV_SYNTAX_AUTOOP            "Syntax: AUTOOP <#channel> <registered nickname>"

#define CHANSERV_SYNTAX_CLEARMODES        "Syntax: CLEARMODES <#channel>"
#define CHANSERV_SYNTAX_CYCLE             "Syntax: CYCLE <#channel>"

#define CHANSERV_SYNTAX_DROP              "Syntax: DROP <#channel> <password>"
#define CHANSERV_SYNTAX_CHOWNER           "Syntax: CHOWNER <#channel> <registered nickname> <password>"

#define CHANSERV_SYNTAX_LIST              "Syntax: LIST <disabled|expired|noexpire>"
#define CHANSERV_SYNTAX_UNREG             "Syntax: UNREG <#channel> FORCE"
#define CHANSERV_SYNTAX_CHOWNER_ADMIN     "Syntax: CHOWNER <#channel> <registered nickname> FORCE"
#define CHANSERV_SYNTAX_COMMENT           "Syntax: COMMENT <#channel> <ADD|REM|LIST> [comment|id]"
#define CHANSERV_SYNTAX_DISABLE           "Syntax: DISABLE <#channel>"
#define CHANSERV_SYNTAX_GREP              "Syntax: GREP <chan|user> <mask>"
#define CHANSERV_SYNTAX_ENABLE            "Syntax: ENABLE <#channel>"
#define CHANSERV_SYNTAX_NOEXPIRE          "Syntax: NOEXPIRE <#channel>"

/* CHANSERV_ GENERIC defines */
#define CHANSERV_WRONG_PASSWORD       "Incorrect password."
#define CHANSERV_CHANNEL_NOT_FOUND    "%s is not a registered channel."
#define CHANSERV_CHANNEL_DISABLED     "This channel has been disabled."
#define CHANSERV_VICTIM_HIGHER_LEVEL  "You cannot kick users with a higher level than yourself." 
#define CHANSERV_USER_NOT_FOUND       "%s is not a registered nickname."

/* User commands */
FUNC_COMMAND(chanserv_help);
FUNC_COMMAND(chanserv_removeme);
FUNC_COMMAND(chanserv_showcommands);
FUNC_COMMAND(chanserv_op);
FUNC_COMMAND(chanserv_deop);
FUNC_COMMAND(chanserv_deopme);
FUNC_COMMAND(chanserv_opme);
FUNC_COMMAND(chanserv_access);
FUNC_COMMAND(chanserv_clearmodes);
FUNC_COMMAND(chanserv_invite);
FUNC_COMMAND(chanserv_listaccess);
FUNC_COMMAND(chanserv_register);
FUNC_COMMAND(chanserv_drop);
FUNC_COMMAND(chanserv_info);
FUNC_COMMAND(chanserv_ban);
FUNC_COMMAND(chanserv_unban);
FUNC_COMMAND(chanserv_banlist);
FUNC_COMMAND(chanserv_topic);
FUNC_COMMAND(chanserv_voice);
FUNC_COMMAND(chanserv_devoice);
FUNC_COMMAND(chanserv_kick);
FUNC_COMMAND(chanserv_kickban);
FUNC_COMMAND(chanserv_autoop);
FUNC_COMMAND(chanserv_cycle);
FUNC_COMMAND(chanserv_chowner);

/* Admin commands */
FUNC_COMMAND(chanserv_lister);
FUNC_COMMAND(chanserv_noexpire);
FUNC_COMMAND(chanserv_unreg);
FUNC_COMMAND(chanserv_comment);
FUNC_COMMAND(chanserv_disable);
FUNC_COMMAND(chanserv_grep);
FUNC_COMMAND(chanserv_enable);


chanserv_dbase_access  *chanserv_dbase_has_access(char *nick, chanserv_dbase_channel *chan);
chanserv_dbase_access  *chanserv_dbase_find_access(nickserv_dbase_data *nick, chanserv_dbase_channel *chan);

chanserv_dbase_channel *chanserv_dbase_find_chan(const char *chan);
chanserv_dbase_channel *chanserv_dbase_create(char *chan, dbase_nicks *from);
chanserv_dbase_channel *chanserv_dbase_add(char *chan, char *owner, char *topic, char *mode, unsigned long flags, unsigned long lastlogin, int sql);

int                     chanserv_dbase_check_access(nickserv_dbase_data *nick, chanserv_dbase_channel *chan, int level);
int                     chanserv_dbase_access_add(chanserv_dbase_channel *chan, nickserv_dbase_data *nick, int level, int autoop, int sql);
int                     chanserv_dbase_access_delete(chanserv_dbase_channel *chan, nickserv_dbase_data *nick, int sql);
int                     chanserv_dbase_delete(chanserv_dbase_channel *chan);
int                     chanserv_dbase_internal_search(long low, long high, const char *chan);
int                     chanserv_dbase_update_lastlogin(chanserv_dbase_channel *ch);
int                     chanserv_dbase_disabled(chanserv_dbase_channel *ch);
int                     chanserv_dbase_comment_add(chanserv_dbase_channel *chan, nickserv_dbase_data *nick, const char *comment, long date, int sql);
int                     chanserv_dbase_comment_del(chanserv_dbase_channel *chan, unsigned long nr, int sql);
int                     chanserv_dbase_add_enforce_ban(chanserv_dbase_channel *ch, const char *banmask, int period, const char *nick, int sql);
int                     chanserv_dbase_check_enforced_ban(const char *chan, const char *mask);
int                     chanserv_dbase_check_enforced_ban_kick(const char *chan, const char *mask, const char *numeric);

void                    chanserv_dbase_burst_join(sock_info *sock);
void                    chanserv_dbase_cleanup(void);
void                    chanserv_dbase_expire(chanserv_dbase_channel *ch);
void                    chanserv_dbase_check_expire(void *ptr);
void                    chanserv_dbase_remove_covered(chanserv_dbase_channel *ch, const char *banmask);
void                    chanserv_dbase_remove_enforce_ban(chanserv_dbase_channel *ch, int nr, int nocancel, int sql);
void                    chanserv_dbase_remove_ban(void *ptr);
void                    chanserv_dbase_join(chanserv_dbase_channel *chan, unsigned long burst, char *mode);
void                    chanserv_dbase_part(chanserv_dbase_channel *chan);
void                    chanserv_dbase_resync_channel(dbase_channels *chan);
#endif /* INC_CHANSERV_H */
