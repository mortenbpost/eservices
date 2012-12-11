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
/* $Id: forbid.c,v 1.3 2003/03/01 16:47:08 cure Exp $ */

#include <string.h>

#include "nickserv.h"
#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

extern sock_info *irc;
extern nickserv_dbase_data *jupe_list[26*26];

#define NICKSERV_FORBID_UNFORBID_SUCCESSFUL "%s is no longer a forbidden nickname."
#define NICKSERV_FORBID_SUCCESSFUL          "%s is now a forbidden nickanem."
#define NICKSERV_FORBID_NOT_FORBIDDEN       "%s is not a forbidden nickname."
#define NICKSERV_FORBID_ALREADY_FORBIDDEN   "Nickname is already forbidden."
#define NICKSERV_FORBID_FULL                "ALARM ALARM ALARM! MAX FORBIDDEN NICKNAMES REACHES PLEASE CONTACT SERVICES ADMIN!!!"
#define NICKSERV_FORBID_LIST_HEADER         "Forbidden nicknames:\n"\

/**************************************************************************************************
 * nickserv_forbid
 **************************************************************************************************
 *   FORBID <nick> <reason>
 *   +R +O +N
 *      Marks <nick> as forbidden with <reason>.
 *      If <nick> is regged, the account is removed.
 *      If a user is currently using the nick, he/she will be killed.
 *       <nick>   = getnext-string
 *       <reason> = getrest-string
 **************************************************************************************************
 * Params:
 *   [IN] sock_info *sock    : The socket from which the data was recieved
 *   [IN] dbase_nicks *from  : Pointer to the user who issued this command
 *   [IN] char **params      : The parameters to the command (to be used with getnext and getrest)
 *   [IN] char *format       : The format the message should be returned (privmsg or notice)
 *   [IN] parser_command_data: *command_info : struct containing syntax, access-level etc.
 * Return:
 *  [OUT] int return         : return 0 if success, anything else will cause the services to stop
 **************************************************************************************************/ 
FUNC_COMMAND(nickserv_forbid)
{
  char *type = getnext(params);

  if (!type)
    return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);

  type = uppercase(type);

  if (!strcmp(type, "ADD"))
  {
    char *nick, *reason, num[4] = "JAA";
    char buf[BUFFER_SIZE];
    nickserv_dbase_data *data;
    int i, found = -1;

    /* is user authed */
    if (!from->nickserv) return ERROR_NO_ACCESS;

    /* Does the user have oper-access to this command */
    if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;

    /* Ok, it looks like the users actually do have access to this command,
    proceed with checking for correct parameters */
    nick = getnext(params);
    reason = getrest(params);

    if (!reason) return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);

    /* The nick we are trying to jupe, is it valid ? */
    if (!nickserv_dbase_valid_nick(nick)) return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_INVALID_NICK, nick);

    /* For-Loop the jupe_list to find a empty numeric */
    for (i = 0; i < (26*26); i++)
    {
      if (jupe_list[i])
      {
        /* is the nick alrease juped ? */
        if (!strcasecmp(jupe_list[i]->nick, nick))
        return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_FORBID_ALREADY_FORBIDDEN);
      }
      else if (found == -1)
      {
        /* Create numeric from the index in the array */
        num[1] = 'A' + (i / 26);
        num[2] = 'A' + (i % 26);
        num[3] = '\0';
        found = i;
      }
    }
    /* if found == -1, then the jupe array is full, ie. no more jupes allowed */
    if (found == -1) return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_FORBID_FULL);
  
    /* OK, got a numeric for the juped nick, now we can move on...
       Does the user already exist in the database, if so, unregister the user */
    if ((data = nickserv_dbase_find_nick(nick)))
    {
      /* Remove the user from nickserv's database */
      nickserv_dbase_unreg(data);
    }

    /* Add the juped nick to nickserv's database and to the jupe array */
    jupe_list[found] = nickserv_dbase_add(nick, nick, "nothing", "juped@"NETWORK_NAME, time(0), reason, BITS_NICKSERV_JUPED, time(0), 0, COMMIT_TO_MYSQL);

    /* Introduce the juped nick to the network, with an old connect time (JUPE_TIME) to
     make sure it doesn't get killed in a nick collide etc. */
    com_send(irc, "%s N %s 1 %ld juped %s +kid xXxXxX %s%s :%s\n", conf->numeric, nick, JUPE_TIME, NETWORK_NAME, conf->numeric, num, reason);

    /* Write to the logfile who did it, and why */
    strcpy(buf, queue_escape_string(nick));
    log_command(LOG_NICKSERV, from, "FORBID", "%s %s %s", type, buf, queue_escape_string(reason));
  
    /* Tell oper that the nick was successfully juped */
    return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_FORBID_SUCCESSFUL, nick);

  }
  else if (!strcmp(type, "REM"))
  {
    /* UNFORBID <nick> */
    char *nick, num[4] = "JAA";
    nickserv_dbase_data *data;
    int i;
    /* is user auth'ed in NS ? */
    if (!from->nickserv) return ERROR_NO_ACCESS;

    /* Does the user have access to this command */
    if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;

    /* Ok, it looks like the users actually do have access to this command,
     proceed with checking for correct parameters */
    nick = getnext(params);

    /* Ok, did the oper remember the parameter ? */
    if (!nick) return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);

    /* Does the nick exist in nickserv's database ? */
    if (!(data = nickserv_dbase_find_nick(nick))) return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_NOT_REGISTERED, nick);

    /* Is the nick actually juped ? */
    if (!(data->flags & BITS_NICKSERV_JUPED)) return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_FORBID_NOT_FORBIDDEN, nick);

    /* Ok, lets search the jupe_list for a match.
       Since we don't have the numeric we check them all using a for loop, compariong pointers */
    for (i = 0; i < (26*26); i++)
    {
      if (jupe_list[i] == data)
      {
        jupe_list[i] = NULL;
        num[1] = 'A' + (i / 26);
        num[2] = 'A' + (i % 26);
        break;
      }
    }
    /* Hmm, listed as juped, but not in the jupe_list, should NEVER happen  */
    if (i >= (26*26))
    {
      log_command(LOG_SERVICES, NULL, "", "BUG! nickserv_unforbid, Didn't find juped nick in jupe_list");
      return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_FORBID_NOT_FORBIDDEN);
    }
    
    com_send(irc, "%s%s Q :%s released jupe.\n", conf->numeric, num, from->nick);
    nickserv_dbase_del(data->nick, COMMIT_TO_MYSQL);

    /* Write to logfile */
    log_command(LOG_NICKSERV, from, "FORBID", "%s %s", type, queue_escape_string(nick));

    /* Tell oper that the nick was successfully unjuped */
    return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_FORBID_UNFORBID_SUCCESSFUL, nick);
  } 
  else if (!strcmp(type, "LIST"))
  {
    int i;
    /* do the user have access to this command */
    if (!from->nickserv) return ERROR_NO_ACCESS;
    if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;
  
    /* write header */
    com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_FORBID_LIST_HEADER);
    /* loop through the entire jupe-list, writing jupes as they are found */
    for (i = 0; i < (26*26); i++)
      if (jupe_list[i])          
      {
        com_message(sock, conf->ns->numeric, from->numeric, format, "  %-9s %s", jupe_list[i]->nick, jupe_list[i]->info);
      }
    /* log the command */
    log_command(LOG_NICKSERV, from, "FORBID", "LIST");
  }
  else
    return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);

  return 0; 
}
