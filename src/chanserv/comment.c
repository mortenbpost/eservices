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
/* $Id: comment.c,v 1.3 2003/03/01 16:47:04 cure Exp $ */

#include <string.h>

#include "chanserv.h"
#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define CHANSERV_COMMENT_LIST_EMPTY   "There are no comments recorded on %s."
#define CHANSERV_COMMENT_LIST_HEAD    "Comments record for %s:"
#define CHANSERV_COMMENT_LIST_ENTRY   "  %d) On %2.2d/%2.2d/%4.4d %s wrote:\n"\
                                      "  %s"
#define CHANSERV_COMMENT_REMOVED      "Comment successfully removed."
#define CHANSERV_COMMENT_ADDED        "Comment successfully added."
#define CHANSERV_COMMENT_NOT_YOURS    "You cannot remove comments set by others."

/**************************************************************************************************
 * chanserv_comment
 **************************************************************************************************
 *   COMMENT <#channel> <ADD> <text>
 *   COMMENT <#channel> <DEL> <nr>
 *   COMMENT <#channel> [LIST]
 *      +C
 *      Adds/removes a comment for <#channel>, or shows all comments.
 *      <#channel>  = getnext-string
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
FUNC_COMMAND(chanserv_comment)
{
  char *chan    = getnext(params);
  char *cmd     = getnext(params);
  char *comment = getrest(params);
  char buf[BUFFER_SIZE];
  chanserv_dbase_channel *ch;
    
  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;
  
  if (!chan) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
  
  /* check if the channel is registered */
  if (!(ch = chanserv_dbase_find_chan(chan))) return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
    
  strcpy(buf, queue_escape_string(ch->name));  
  
  if (cmd)
    cmd = uppercase(cmd);

  if ((!strcmp(cmd, "LIST")))
  {
    int i;
    struct tm *td;
      
    log_command(LOG_CHANSERV, from, "COMMENT", "LIST %s", buf);

    if (!ch->comment_count)
      return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_COMMENT_LIST_EMPTY, chan); 
    com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_COMMENT_LIST_HEAD, chan);
    for (i = 0; i < ch->comment_count; i++)
    {
      td = localtime((time_t*)&ch->comments[i]->date);
      com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_COMMENT_LIST_ENTRY, (i+1), td->tm_mday, (td->tm_mon +1), (td->tm_year + 1900), ch->comments[i]->nick, ch->comments[i]->comment);
    }
  }  
  else if (!strcmp(cmd, "REM"))
  {
    long nr = tr_atoi(comment);
    if ((nr == 0) || (nr > ch->comment_count))
      return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
    
    if (!(strcmp(from->nickserv->nick, ch->comments[nr-1]->nick)) || operserv_have_access(from->nickserv->flags, BITS_OPERSERV_SERVICES_SUB_ADMIN))
    {
      chanserv_dbase_comment_del(ch, nr-1, COMMIT_TO_MYSQL);
      
      log_command(LOG_CHANSERV, from, "COMMENT", "REM %s %lu (%s)", buf, nr, queue_escape_string(ch->comments[nr-1]->comment));
      
      return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_COMMENT_REMOVED);
    }
    return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_COMMENT_NOT_YOURS);
  }
  else if (!strcmp(cmd, "ADD"))
  {
    if (!comment) return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

    chanserv_dbase_comment_add(ch, from->nickserv, comment, time(0), COMMIT_TO_MYSQL);
    
    log_command(LOG_CHANSERV, from, "COMMENT", "ADD %s %s", buf, queue_escape_string(comment));    
    
    com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_COMMENT_ADDED);
  }
  else
    return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);
  
  return 0;
}
