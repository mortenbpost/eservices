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
/* $Id: comment.c,v 1.3 2003/03/01 16:47:08 cure Exp $ */

#include <string.h>

#include "nickserv.h"
#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "setup.h"
#include "queue.h"
#include "log.h"

#define NICKSERV_COMMENT_ADD                "Comment successfully added."
#define NICKSERV_COMMENT_REMOVED            "Comment successfully removed."
#define NICKSERV_COMMENT_NOT_YOURS          "You cannot remove comments set by other."
#define NICKSERV_COMMENT_LIST_FOOTER        "%lu comments listed."
#define NICKSERV_COMMENT_LIST_HEADER        "Comment record for %s:"
#define NICKSERV_COMMENT_ENTRY              "  %d) On %2.2d/%2.2d/%4.4d %s wrote:\n"\
                                            "  %s"
#define NICKSERV_COMMENT_EMPTY              "There are no comments set on %s."

/**************************************************************************************************
 * nickserv_comment
 **************************************************************************************************
 *   COMMENT <rnick> <ADD> <text>
 *   COMMENT <rnick> <DEL> <nr>
 *   COMMENT <rnick> [LIST]
 *      +C
 *      Adds/removes a comment for <rnick>, or shows all comments.
 *      <rnick>     = getnext-string
 *      <ADD>/<DEL> = gettext-string
 *      <text>/<nr> = getrest-string
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
FUNC_COMMAND(nickserv_comment)
{
  char *nick    = getnext(params);
  char *cmd     = getnext(params);
  char *comment = getrest(params);
  char buf[BUFFER_SIZE];
  nickserv_dbase_data *ns;

  /* does the user have access to the command */
  if (!from->nickserv) return ERROR_NO_ACCESS;
  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;
  
  /* enough parameters */
  if (!nick || !cmd) return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);
  
  /* check if the nick is registered */
  if (!(ns = nickserv_dbase_find_nick(nick))) return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_NOT_REGISTERED, nick);
    
  strcpy(buf, queue_escape_string(ns->nick));

  /* uppercase cmd - if specified */
  cmd = uppercase(cmd);

  /* is cmd = LIST or not specified */
  if (!strcmp(cmd, "LIST"))
  {
    int i;
    struct tm *td;
    
    /* log the command */
    log_command(LOG_NICKSERV, from, "COMMENT", "LIST %s", buf);

    if (!ns->comment_count)
      return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_COMMENT_EMPTY, ns->nick);

    /* write header */
    com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_COMMENT_LIST_HEADER, ns->nick);
    /* loop through all comments */
    for (i = 0; i < ns->comment_count; i++)                                                                                    
    {
      td = localtime((time_t*)&ns->comments[i]->date);
      com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_COMMENT_ENTRY, (i+1), td->tm_mday, (td->tm_mon +1), (td->tm_year + 1900), ns->comments[i]->nick, ns->comments[i]->comment);
    }
  }  
  /* is cmd = REM */
  else if (!strcmp(cmd, "REM"))
  {
    long nr = tr_atoi(comment);
    /* is the specified comment-number valid ? */
    if ((nr == 0) || (nr > ns->comment_count))
      return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);

    /* is the user an NS_ADMIN, or is the user the user who posted the comment */
    if (!(strcmp(from->nickserv->nick, ns->comments[nr-1]->nick)) || operserv_have_access(from->nickserv->flags, BITS_OPERSERV_NS_ADMIN))
    {
      /* delete the comment */
      nickserv_dbase_comment_del(ns, nr-1, COMMIT_TO_MYSQL);
      
      /* log the command */
      log_command(LOG_NICKSERV, from, "COMMENT", "REM %s %lu (%s)", buf, nr, queue_escape_string(ns->comments[nr-1]->comment));
      
      /* write a confirmation, and return */
      return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_COMMENT_REMOVED);
    }
    /* write error-message, about only removing own comments */
    return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_COMMENT_NOT_YOURS);
  }
  /* if cmd = ADD */
  else if (!strcmp(cmd, "ADD"))
  {
    /* is the comment parameter specified */
    if (!comment) return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax); 

    /* add the comment */
    nickserv_dbase_comment_add(ns, from->nickserv, comment, time(0), COMMIT_TO_MYSQL);
    
    /* log the command */
    log_command(LOG_NICKSERV, from, "COMMENT", "ADD %s %s", buf, queue_escape_string(comment));    
    
    /* write confirmation message */
    com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_COMMENT_ADD);
  }
  else
    return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);
  
  return 0;
}
