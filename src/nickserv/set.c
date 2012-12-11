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
/* $Id: set.c,v 1.3 2003/03/01 16:47:08 cure Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nickserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define NICKSERV_SET_OK                     "Successfully set setting %s to %s"
#define NICKSERV_SET_INFO_MAXLENGTH         "The info line can only contain 254 characters."
#define NICKSERV_SET_NOT_OPER               "You must be a Global IRCop to activate OPER mode."

/**************************************************************************************************
 * nickserv_set
 **************************************************************************************************
 *   SET <option> <parameter>
 *      +R
 *      Changes settings for the users account in nickserv
 *       <option>    = getnext-string
 *       <parameter> = getrest-string
 **************************************************************************************************
 * Params:
 *   [IN] sock_info *sock    : The socket from which the data was recieved
 *   [IN] dbase_nicks *from  : Pointer to the user who issued this command
 *   [IN] char **params      : The parameters to the command (to be used with getnext and getrest)
 *   [IN] char *format       : The format the message should be returned (privmsg or notice)
 *   [IN] parser_command_data *command_info : struct containing syntax, access-level etc.
 * Return:
 *  [OUT] int return         : return 0 if success, anything else will cause the services to stop
 **************************************************************************************************/ 
FUNC_COMMAND(nickserv_set)
{
  char *option = getnext(params);
  char *para = getrest(params);
  unsigned long flag = 0;

  /* is the user authed */
  if (!from->nickserv) return ERROR_NO_ACCESS;

  /* enouth parameters */
  if (!para) return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);

  /* uppercase the option */
  option = uppercase(option);

  /* find what bits to set/remove if the option is a true/false option */
  if (!strcmp(option, "PRIVMSG"))     flag = BITS_NICKSERV_PRIVMSG;
  else if (!strcmp(option, "OPER"))   flag = BITS_NICKSERV_OPER | BITS_OPERSERV_GLINE | BITS_OPERSERV_MODES;
  else if (!strcmp(option, "SECRET")) flag = BITS_NICKSERV_SECRET;

  /* is the option a true/false option */
  if (flag)
  {
    int res;
    /* try to parse the on/off parameter, returns 1 = off, 2 = on */
    if ((res = nickserv_dbase_set_is_ok(para)))
    {
      /* convert nickserv_dbase_set_is_ok to C-style true/false */
      res--;
      /* Is the user trying to set oper on without being a global oper */
      if ((flag & BITS_NICKSERV_OPER) && (!isbiton(from->modes, 'o'-'a')))
        return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_SET_NOT_OPER);
      /* set/remove the flags in the database */
      if (res) nickserv_dbase_setbit(from->nickserv, flag, 1);
      else nickserv_dbase_removebit(from->nickserv, flag, 1);

      /* log the command, and send a message to the user that the option was set */
      log_command(LOG_NICKSERV, from, "SET", "%s %s", option, para);
      return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_SET_OK, option, para);
    }
  }
  else if (!strcmp(option, "EMAIL"))
  {
    char buf[BUFFER_SIZE];

    /* Is it a valid email-address, or is it used by another account ? */
    if ((!nickserv_dbase_valid_email(para)) || (nickserv_dbase_email_exists(para)))
      return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_INVALID_EMAIL, para);

    /* Set the new email addres, and save it to the database */
    from->nickserv->email = (char *)realloc(from->nickserv->email, strlen(para)+1);
    strcpy(from->nickserv->email, para);
    snprintf(buf, BUFFER_SIZE, "UPDATE nickdata SET email='%s' WHERE nick='%s'", para, queue_escape_string(from->nickserv->nick));
    queue_add(buf);

    /* log the command and return a confirmation to the user */
    log_command(LOG_NICKSERV, from, "SET", "EMAIL %s", para);
    return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_SET_OK, option, para);
  }
  if (!strcmp(option, "INFO"))
  {
    char buf[BUFFER_SIZE];
    
    /* is the info-line too long ? */
    if (strlen(para) > 254) return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_SET_INFO_MAXLENGTH);
    
    /* set the new info line, and save it to the database */
    para = queue_escape_string(para);
    from->nickserv->info = (char *)realloc(from->nickserv->info, strlen(para)+1);
    strcpy(from->nickserv->info, para);

    snprintf(buf, BUFFER_SIZE, "UPDATE nickdata SET info='%s' WHERE nick='%s'", from->nickserv->info, queue_escape_string(from->nickserv->nick));
    queue_add(buf);
    
    /* log the command, and return a confirmation to the user */
    log_command(LOG_NICKSERV, from, "SET", "INFO %s", queue_escape_string(para));
    return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_SET_OK, option, para);
  }
  /* the option was not recongnised, return syntax to the user */
  return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);
}
