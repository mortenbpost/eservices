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
/* $Id: msg.h,v 1.12 2003/02/25 22:41:04 cure Exp $ */

#ifndef INC_MSG_H
#define INC_MSG_H

#include "parser.h"
#include "p10.h"
#include "nickserv.h"
#include "chanserv.h"
#include "operserv.h"
#include "multiserv.h"
#include "dcc.h"

struct parser_command_data parser_nickserv_commands[] =
{
/*{command,        parser function,       authed required,            oper requirement,         syntax} */
   /* Not authed */  
  {"AUTH",         nickserv_auth,         0,                          0,                        NICKSERV_SYNTAX_AUTH},
  {"GHOST",        nickserv_ghost,        0,                          0,                        NICKSERV_SYNTAX_GHOST},
  {"HELP",         nickserv_help,         0,                          0,                        NICKSERV_SYNTAX_HELP},
  {"INFO",         nickserv_info,         0,                          0,                        NICKSERV_SYNTAX_INFO},
  {"REGISTER",     nickserv_register,     0,                          0,                        NICKSERV_SYNTAX_REGISTER},
  {"WHOIS",        nickserv_whois,        0,                          0,                        NICKSERV_SYNTAX_WHOIS},
   /* authed */
  {"DROP",         nickserv_drop,         1,                          0,                        NICKSERV_SYNTAX_DROP},
  {"PASS",         nickserv_pass,         1,                          0,                        NICKSERV_SYNTAX_PASS},
  {"SET",          nickserv_set,          1,                          0,                        NICKSERV_SYNTAX_SET},
   /* oper */
  {"COMMENT",      nickserv_comment,      1,                          BITS_OPERSERV_NS_OPER,    NICKSERV_SYNTAX_COMMENT},
   /* admin */
  {"CHPASS",       nickserv_chpass,       1,                          BITS_OPERSERV_NS_ADMIN,   NICKSERV_SYNTAX_CHPASS},
  {"FORBID",       nickserv_forbid,       1,                          BITS_OPERSERV_NS_ADMIN,   NICKSERV_SYNTAX_FORBID},
  {"",             NULL,                  0xffffffff,                 0,                        NULL},
  {NULL,           NULL,                  0xffffffff,                 0,                        NULL}
};


struct parser_command_data parser_chanserv_commands[] =
{
/*{command,        parser function,       access level,              oper requirement,         syntax} */
  {"HELP",         chanserv_help,         0,                         0,                        CHANSERV_SYNTAX_HELP},
  {"INFO",         chanserv_info,         0,                         0,                        CHANSERV_SYNTAX_INFO},
  {"REGISTER",     chanserv_register,     0,                         0,                        CHANSERV_SYNTAX_REGISTER},
  {"SHOWCOMMANDS", chanserv_showcommands, 0,                         0,                        CHANSERV_SYNTAX_SHOWCOMMANDS},
  /* level 1 */
  {"REMOVEME",     chanserv_removeme,     CHANSERV_LEVEL_REMOVEME,   0,                        CHANSERV_SYNTAX_REMOVEME},  
  /* level 50 */
  {"DEVOICE",      chanserv_devoice,      CHANSERV_LEVEL_DEVOICE,    0,                        CHANSERV_SYNTAX_DEVOICE},
  {"INVITE",       chanserv_invite,       CHANSERV_LEVEL_INVITE,     0,                        CHANSERV_SYNTAX_INVITE},
  {"TOPIC",        chanserv_topic,        CHANSERV_LEVEL_TOPIC,      0,                        CHANSERV_SYNTAX_TOPIC},
  {"VOICE",        chanserv_voice,        CHANSERV_LEVEL_VOICE,      0,                        CHANSERV_SYNTAX_VOICE},
   /* level 100 */
  {"OP",           chanserv_op,           CHANSERV_LEVEL_OP,         0,                        CHANSERV_SYNTAX_OP},
  {"OPME",         chanserv_opme,         CHANSERV_LEVEL_OPME,       0,                        CHANSERV_SYNTAX_OPME},
  {"DEOP",         chanserv_deop,         CHANSERV_LEVEL_DEOP,       0,                        CHANSERV_SYNTAX_DEOP},
  {"DEOPME",       chanserv_deopme,       CHANSERV_LEVEL_DEOPME,     0,                        CHANSERV_SYNTAX_DEOPME},
   /* level 200 */
  {"KICK",         chanserv_kick,         CHANSERV_LEVEL_KICK,       0,                        CHANSERV_SYNTAX_KICK},
  {"KICKBAN",      chanserv_kickban,      CHANSERV_LEVEL_KICKBAN,    0,                        CHANSERV_SYNTAX_KICKBAN},
   /* level 300 */
  {"BANLIST",      chanserv_banlist,      CHANSERV_LEVEL_BANLIST,    0,                        CHANSERV_SYNTAX_BANLIST},
  {"UNBAN",        chanserv_unban,        CHANSERV_LEVEL_UNBAN,      0,                        CHANSERV_SYNTAX_UNBAN},
  {"BAN",          chanserv_ban,          CHANSERV_LEVEL_BAN,        0,                        CHANSERV_SYNTAX_BAN},
  /* level 400 */
  {"ACCESS",       chanserv_access,       CHANSERV_LEVEL_ACCESS,     0,                        CHANSERV_SYNTAX_ACCESS},
  {"AUTOOP",       chanserv_autoop,       CHANSERV_LEVEL_AUTOOP,     0,                        CHANSERV_SYNTAX_AUTOOP},
   /* level 450 */
  {"CLEARMODES",   chanserv_clearmodes,   CHANSERV_LEVEL_CLEARMODES, 0,                        CHANSERV_SYNTAX_CLEARMODES},
  {"CYCLE",        chanserv_cycle,        CHANSERV_LEVEL_CYCLE,      0,                        CHANSERV_SYNTAX_CYCLE},
   /* level 500 */
  {"CHOWNER",      chanserv_chowner,      CHANSERV_LEVEL_CHOWNER,    0,                        CHANSERV_SYNTAX_CHOWNER},
  {"DROP",         chanserv_drop,         CHANSERV_LEVEL_DROP,       0,                        CHANSERV_SYNTAX_DROP},
   /* oper */
  {"COMMENT",      chanserv_comment,      0xffffffff,                BITS_OPERSERV_CS_OPER,    CHANSERV_SYNTAX_COMMENT},
  {"CYCLE",        chanserv_cycle,        0xffffffff,                BITS_OPERSERV_CS_OPER,    CHANSERV_SYNTAX_CYCLE},
  {"DISABLE",      chanserv_disable,      0xffffffff,                BITS_OPERSERV_CS_OPER,    CHANSERV_SYNTAX_DISABLE},
  {"ENABLE",       chanserv_enable,       0xffffffff,                BITS_OPERSERV_CS_OPER,    CHANSERV_SYNTAX_ENABLE},
  {"GREP",         chanserv_grep,         0xffffffff,                BITS_OPERSERV_CS_OPER,    CHANSERV_SYNTAX_GREP},
  {"LIST",         chanserv_lister,       0xffffffff,                BITS_OPERSERV_CS_OPER,    CHANSERV_SYNTAX_LIST},
  {"NOEXPIRE",     chanserv_noexpire,     0xffffffff,                BITS_OPERSERV_CS_OPER,    CHANSERV_SYNTAX_NOEXPIRE},
   /* admin */
  {"UNREG",        chanserv_unreg,        0xffffffff,                BITS_OPERSERV_CS_ADMIN,   CHANSERV_SYNTAX_UNREG},
  {"CHOWNER",      chanserv_chowner,      0xffffffff,                BITS_OPERSERV_CS_ADMIN,   CHANSERV_SYNTAX_CHOWNER_ADMIN},
  {"",             NULL,                  0xffffffff,                0,                        ""},
  {NULL,           NULL,                  0xffffffff,                0,                        NULL}
};


struct parser_command_data parser_operserv_commands[] =
{
/*{command,        parser function,       authed required,  oper requirement,                  syntax} */  
   /* +O */
  {"HELP",         operserv_help,         1,                BITS_OPERSERV_OPER,                OPERSERV_SYNTAX_HELP},
  {"BROADCAST",    operserv_broadcast,    1,                BITS_OPERSERV_OPER,                OPERSERV_SYNTAX_BROADCAST},
  {"OPERLIST",     operserv_operlist,     1,                BITS_OPERSERV_OPER,                OPERSERV_SYNTAX_OPERLIST},
  {"TRACE",        operserv_trace,        1,                BITS_OPERSERV_OPER,                OPERSERV_SYNTAX_TRACE},
   /* +M */
  {"MODE",         operserv_mode,         1,                BITS_OPERSERV_MODES,               OPERSERV_SYNTAX_MODE},
  {"OP",           operserv_op,           1,                BITS_OPERSERV_MODES,               OPERSERV_SYNTAX_OP},
   /* +G */
  {"GLINE",        operserv_gline,        1,                BITS_OPERSERV_GLINE,               OPERSERV_SYNTAX_GLINE},
  {"UNGLINE",      operserv_ungline,      1,                BITS_OPERSERV_GLINE,               OPERSERV_SYNTAX_UNGLINE},
   /* +a */
  {"ACCESS",       operserv_access,       1,                BITS_OPERSERV_SERVICES_SUB_ADMIN,  OPERSERV_SYNTAX_ACCESS},
   /* +A */
  {"DIE",          operserv_die,          1,                BITS_OPERSERV_SERVICES_ADMIN,      OPERSERV_SYNTAX_DIE},
  {"REMOPER",      operserv_remoper,      1,                BITS_OPERSERV_SERVICES_ADMIN,      OPERSERV_SYNTAX_REMOPER},
  {"",             NULL,                  0xffffffff,       0,                                 ""},
  {NULL,           NULL,                  0xffffffff,       0,                                 NULL}
};

struct parser_command_data parser_multiserv_commands[] =
{
/*{command,        parser function,       authed required,  oper requirement,                  syntax} */  
  {"HELP",         multiserv_help,        1,                0,                                 MULTISERV_SYNTAX_HELP},
  {"STATS",        multiserv_stats,       1,                BITS_OPERSERV_OPER,                MULTISERV_SYNTAX_STATS},
  {"",             NULL,                  0xffffffff,       0,                                 ""},
  {NULL,           NULL,                  0xffffffff,       0,                                 NULL}
};

struct parser_command_data parser_dcc_commands[] =
{
/*{command,        parser function,       authed required,  oper requirement,                  syntax} */  
  {"BROADCAST",    operserv_broadcast,    1,                BITS_OPERSERV_OPER,                OPERSERV_SYNTAX_BROADCAST},
  {"CLOSE",        dcc_close,             1,                BITS_OPERSERV_OPER,                DCC_SYNTAX_CLOSE},
  {"CONSOLE",      dcc_console,           1,                BITS_OPERSERV_OPER,                DCC_SYNTAX_CONSOLE},
  {"HELP",         dcc_help,              1,                BITS_OPERSERV_OPER,                DCC_SYNTAX_HELP},
  {"OPERLIST",     operserv_operlist,     1,                BITS_OPERSERV_OPER,                OPERSERV_SYNTAX_OPERLIST},
  {"SAY",          dcc_say,               1,                BITS_OPERSERV_OPER,                DCC_SYNTAX_SAY},
  {"TRACE",        operserv_trace,        1,                BITS_OPERSERV_OPER,                OPERSERV_SYNTAX_TRACE},
  {"WHO",          dcc_who,               1,                BITS_OPERSERV_OPER,                DCC_SYNTAX_WHO}, 
   /* +M */
  {"MODE",         operserv_mode,         1,                BITS_OPERSERV_MODES,               OPERSERV_SYNTAX_MODE},
  {"OP",           operserv_op,           1,                BITS_OPERSERV_MODES,               OPERSERV_SYNTAX_OP},
   /* +G */
  {"GLINE",        operserv_gline,        1,                BITS_OPERSERV_GLINE,               OPERSERV_SYNTAX_GLINE},
  {"UNGLINE",      operserv_ungline,      1,                BITS_OPERSERV_GLINE,               OPERSERV_SYNTAX_UNGLINE},
   /* +c */
  {"CCOMMENT",     chanserv_comment,      1,                BITS_OPERSERV_CS_OPER,             CHANSERV_SYNTAX_COMMENT},
  {"CYCLE",        chanserv_cycle,        1,                BITS_OPERSERV_CS_OPER,             CHANSERV_SYNTAX_CYCLE},
  {"CINFO",        chanserv_info,         1,                BITS_OPERSERV_CS_OPER,             CHANSERV_SYNTAX_INFO},
  {"DISABLE",      chanserv_disable,      1,                BITS_OPERSERV_CS_OPER,             CHANSERV_SYNTAX_DISABLE},
  {"ENABLE",       chanserv_enable,       1,                BITS_OPERSERV_CS_OPER,             CHANSERV_SYNTAX_ENABLE},
  {"GREP",         chanserv_grep,         1,                BITS_OPERSERV_CS_OPER,             CHANSERV_SYNTAX_GREP},
  {"LIST",         chanserv_lister,       1,                BITS_OPERSERV_CS_OPER,             CHANSERV_SYNTAX_LIST},
  {"NOEXPIRE",     chanserv_noexpire,     1,                BITS_OPERSERV_CS_OPER,             CHANSERV_SYNTAX_NOEXPIRE},
   /* +C */
  {"CHOWNER",      chanserv_chowner,      1,                BITS_OPERSERV_CS_ADMIN,            CHANSERV_SYNTAX_CHOWNER_ADMIN},
  {"UNREG",        chanserv_unreg,        1,                BITS_OPERSERV_CS_ADMIN,            CHANSERV_SYNTAX_UNREG},
   /* +n */
  {"NCOMMENT",     nickserv_comment,      1,                BITS_OPERSERV_NS_OPER,             NICKSERV_SYNTAX_COMMENT},
  {"NINFO",        nickserv_info,         1,                BITS_OPERSERV_NS_OPER,             NICKSERV_SYNTAX_INFO},
   /* +N */
  {"CHPASS",       nickserv_chpass,       1,                BITS_OPERSERV_NS_ADMIN,            NICKSERV_SYNTAX_CHPASS},
  {"FORBID",       nickserv_forbid,       1,                BITS_OPERSERV_NS_ADMIN,            NICKSERV_SYNTAX_FORBID},
   /* +a */
  {"ACCESS",       operserv_access,       1,                BITS_OPERSERV_SERVICES_SUB_ADMIN,  OPERSERV_SYNTAX_ACCESS},
   /* +A */
  {"DIE",          operserv_die,          1,                BITS_OPERSERV_SERVICES_ADMIN,      OPERSERV_SYNTAX_DIE},
  {"REMOPER",      operserv_remoper,      1,                BITS_OPERSERV_SERVICES_ADMIN,      OPERSERV_SYNTAX_REMOPER},
  
   /* Over and out */
  {"",             NULL,                  0xffffffff,       0,                                 ""},
  {NULL,           NULL,                  0xffffffff,       0,                                 NULL}
};

struct parser_p10_data parser_p10_commands[] =
{
  {"S",       p10_server},
  {"B",       p10_burst},
  {"N",       p10_nick},
  {"J",       p10_join},
  {"P",       p10_privmsg},
  {"O",       p10_notice},
  {"Q",       p10_quit},
  {"L",       p10_part},
  {"M",       p10_mode},
  {"K",       p10_kick},
  {"D",       p10_kill},
  {"T",       p10_topic},
  {"WA",      p10_wallops},
  {"I",       p10_invite},
  {"C",       p10_create},
  {"A",       p10_away},
  {"EB",      p10_end_of_burst},
  {"EA",      p10_end_of_burst_acknowledge},
  {"G",       p10_ping},
  {"SQ",      p10_squit},
  {"V",       p10_version},
  {NULL,      NULL}
};

#endif /* INC_MSG_H */
