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
/* $Id: register.c,v 1.4 2003/03/01 16:47:08 cure Exp $ */

#include <string.h>

#include "nickserv.h"
#include "misc_func.h"
#include "config.h"
#include "queue.h"
#include "log.h"

#define NICKSERV_REGISTER_REGISTERED        "This nickname is already registered."
#define NICKSERV_REGISTER_EMAIL_EXIST       "A nickname has already been registered with this email-address."
#define NICKSERV_REGISTER_OK                "Congratulations, you have just registered the nickname: %s\n"\
                                            "The password is: %s\n"\
                                            "It is very important that you don't forget this password, as it cannot\n"\
                                            "be recovered. Please write it down and keep it a safe place or better yet,\n"\
                                            "just remember it."

/**************************************************************************************************
 * nickserv_register
 **************************************************************************************************
 *   REGISTER <email> <password>
 *      -R
 *      Registeres the users current nick
 *       <email>    = getnext-string
 *       <password> = getnext-string
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
FUNC_COMMAND(nickserv_register)
{
  char *email = getnext(params);
  char *password = getnext(params);
  char buf[20];
  nickserv_dbase_data *entry;
  
  /* Is the user currently authed to a nick ? */
  if (from->nickserv)
  {
    /* Inform the user that he/she is currently auther to a nick, hence cannot
      register another nick. Log the register failure */
    char buf[BUFFER_SIZE];
    queue_escape_string_buf(email, buf);
    if (password)
    {
      strcat(buf, " ");
      strcat(buf, queue_escape_string(password));
    }
    com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_ALREADY_AUTHED, from->nickserv->nick);
    log_command(LOG_NICKSERV, from, "REGISTER", "%s - FAILED - user already authed to %s", buf, queue_escape_string(from->nickserv->nick));
    return 0;
  }

  /* Is the nick the user is trying to register already regged */
  if (nickserv_dbase_find_nick(from->nick))
  {
    /* Inform the user that the nick is already reddeg, and log the failure */
    char buf[BUFFER_SIZE];
    queue_escape_string_buf(email, buf);
    if (password)
    {
      strcat(buf, " ");
      strcat(buf, queue_escape_string(password));
    }    
    com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_REGISTER_REGISTERED);
    log_command(LOG_NICKSERV, from, "REGISTER", "%s - FAILED - %s exsists in nickserv", buf, queue_escape_string(from->nick));
    return 0;
  }

  /* if no parameters given, show syntax */
  if (!email) return com_message(sock, conf->ns->numeric, from->numeric, format, command_info->syntax);

  /* is another nick regged using this email-address? */
  if (nickserv_dbase_email_exists(email))
    return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_REGISTER_EMAIL_EXIST);
    
  /* is the email a valid address? */
  if (!nickserv_dbase_valid_email(email))
    return com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_INVALID_EMAIL, email);
    
  /* is a password given, else generate a random one */
  if (!password)
  {
    password = buf;
    nickserv_dbase_generate_password(password, 8);
  }

  /* create the entry in the database, inform the user that the nick was regged, inform the
     user about his/her password, log the command */
  if ((entry = nickserv_dbase_create(from->nick, email, password)))
  {
    from->nickserv = entry;
    from->nickserv->entry = from;
    nickserv_dbase_update(from);
    com_message(sock, conf->ns->numeric, from->numeric, format, NICKSERV_REGISTER_OK, from->nick, password);
    log_command(LOG_NICKSERV, from, "REGISTER", "%s - SUCCESS", queue_escape_string(email));
  }
  else
    com_message(sock, conf->ns->numeric, from->numeric, format, "An error occured, please contact an oper and report the following error: nickserv_register");

  return 0;
}
