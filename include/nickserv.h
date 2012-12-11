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
/* $Id: nickserv.h,v 1.8 2003/02/25 23:48:39 mr Exp $ */

#ifndef INC_NICKSERV_H
#define INC_NICKSERV_H

#include "parser.h"

/* For how long should we hold the nickname JUPED? */
#define JUPE_TIME 31337

/* The time (in seconds) the should have passed since lastlogin for an account to expire */
#define NICKSERV_EXPIRE_TIME    (30 * 24 * 60 * 60)

/* How often should we check for expired nicks (in seconds) */
#define NICKSERV_EXPIRE_CHECK   (24 * 60 * 60)

/* Welcome greeting defines */
#define NICKSERV_NEW_NICK_NOTICE            "Hello %s, and welcome to "NETWORK_NAME"."
#define NICKSERV_AUTHED_REGNICK             "The nickname %s is registered to somebody else.\n"\
                                            "You are currently authenticate as %s."
#define NICKSERV_NEW_NICK_REGNICK           "The nickname %s is already registered. If you're the rightful owner of this nickname\n"\
                                            "please authenticate to it. If it isn't your nickname please change your\n"\
                                            "current nickname to your registered nickname or something else and register.\n"\
                                            "For more information /msg %s help\n"
#define NICKSERV_NEW_NICK_NOTREGNICK        "The nickname %s is not registered. If you wish to register it you can do it by\n"\
                                            "/msg %s register <email> [password]\n"

/* Syntax defines for nickserv */
#define NICKSERV_SYNTAX_HELP                "Syntax: HELP [command]"
#define NICKSERV_SYNTAX_INFO                "Syntax: INFO <nickname>"
#define NICKSERV_SYNTAX_WHOIS               "Syntax: WHOIS <nickname>"
#define NICKSERV_SYNTAX_AUTH                "Syntax: AUTH [nickname] <password>"
#define NICKSERV_SYNTAX_REGISTER            "Syntax: REGISTER <email> [password]"
#define NICKSERV_SYNTAX_PASS                "Syntax: PASS <oldpass> <newpass>"
#define NICKSERV_SYNTAX_SET                 "Syntax: SET <option> <parameters>"
#define NICKSERV_SYNTAX_DROP                "Syntax: DROP <password>"
#define NICKSERV_SYNTAX_GHOST               "Syntax: GHOST <nickname> <password>"
#define NICKSERV_SYNTAX_FORBID              "Syntax: FORBID <ADD|REM|LIST> [registered nickname] [reason|id]"
#define NICKSERV_SYNTAX_CHPASS              "Syntax: CHPASS <nickname> <password>"
#define NICKSERV_SYNTAX_COMMENT             "Syntax: COMMENT <registered nickname> <ADD|REM|LIST> [comment|id]"

/* 
   GENERIC NICKSERV_ defines 
   NICKSERV_ defines that are used in common should be placed here. 
 */
#define NICKSERV_INVALID_NICK               "%s is not a valid nickname."
#define NICKSERV_INVALID_EMAIL              "%s is not a valid email."
#define NICKSERV_NOT_REGISTERED             "%s is not a registered nickname."
#define NICKSERV_ALREADY_AUTHED             "You are already authorized as %s."
#define NICKSERV_WRONG_PASSWORD             "Incorrect password."

/* 
   FUNC_COMMAND prototypes.
   FUNC_COMMAND is #defined in parser.h
 */
FUNC_COMMAND(nickserv_auth);
FUNC_COMMAND(nickserv_register);
FUNC_COMMAND(nickserv_set);
FUNC_COMMAND(nickserv_info);
FUNC_COMMAND(nickserv_whois);
FUNC_COMMAND(nickserv_drop);
FUNC_COMMAND(nickserv_help);
FUNC_COMMAND(nickserv_pass);
FUNC_COMMAND(nickserv_ghost);
FUNC_COMMAND(nickserv_forbid);
FUNC_COMMAND(nickserv_unforbid);
FUNC_COMMAND(nickserv_forbidlist);
FUNC_COMMAND(nickserv_chpass);
FUNC_COMMAND(nickserv_comment);
FUNC_COMMAND(nickserv_helper);

/*
   Function prototypes for nickserv helper functions.
 */
nickserv_dbase_data  *nickserv_dbase_create(char *nick, char *email, char *passwd);
nickserv_dbase_data  *nickserv_dbase_find_nick(char *nick);
nickserv_dbase_data  *nickserv_dbase_add(const char *nick, const char *email, const char *password, const char *userhost, long lastlogin, char *info, unsigned long flags, long regdate, unsigned long console, int sql);
nickserv_dbase_data  *nickserv_dbase_get_juped(int nr);

void                  nickserv_dbase_generate_password(char *buf, int length);
void                  nickserv_dbase_update(dbase_nicks *info);
void                  nickserv_dbase_cleanup(void);
void                  nickserv_dbase_checkold(void *ptr);
void                  nickserv_dbase_op_on_auth(dbase_nicks *nick);

int                   nickserv_dbase_validate_password(char *nick, char *passwd, dbase_nicks *info);
int                   nickserv_dbase_valid_email(char *email);
int                   nickserv_dbase_unreg(nickserv_dbase_data *ns);
int                   nickserv_dbase_del(char *nick, int sql);
int                   nickserv_dbase_internal_search(long low, long high, const char *nick);
int                   nickserv_dbase_setbit(nickserv_dbase_data *nick, unsigned long bit, int sql);
int                   nickserv_dbase_removebit(nickserv_dbase_data *nick, unsigned long bit, int sql);
int                   nickserv_dbase_checkbit(nickserv_dbase_data *nick, unsigned long bit);
int                   nickserv_dbase_set_is_ok(char *str);
int                   nickserv_dbase_valid_nick(char *nick);
int                   nickserv_dbase_init_jupes(sock_info *sock);
int                   nickserv_dbase_email_exists(char *email);
int                   nickserv_dbase_comment_add(nickserv_dbase_data *who, nickserv_dbase_data *nick, const char *comment, long date, int sql);
int                   nickserv_dbase_comment_del(nickserv_dbase_data *who, unsigned long nr, int sql);
int                   nickserv_new_nick_notice(sock_info *sock, char *numeric, char *nick);
int                   nickserv_dbase_notice(nickserv_dbase_data *who, const char *msg, ...);
int                   nickserv_dbase_notice_check(dbase_nicks *from);

#endif /* INC_NICKSERV_H */
