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
/* $Id: noexpire.c,v 1.2 2003/02/25 22:41:36 cure Exp $ */

#include <stdio.h>

#include "chanserv.h"
#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

#define CHANSERV_NOEXPIRE_TOGGLED     "No expire flag toggled for %s to %s."
#define CHANSERV_KICK_NOT_IN_CHAN     "%s is not in %s."                                      

/**************************************************************************************************
 * chanserv_noexpire
 **************************************************************************************************
 *   NOEXPIRE <#channel>
 *     +C
 *     Toggle NOEXPIRE On/Off on <#channel> 
 *     [#channel] = getnext-string
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
FUNC_COMMAND(chanserv_noexpire)
{
  char *chan = getrest(params);
  char buf[BUFFER_SIZE];
  chanserv_dbase_channel *ch;

  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;
  
  if (!chan)
    return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

    
  if (!(ch = chanserv_dbase_find_chan(chan))) 
    return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_CHANNEL_NOT_FOUND, chan);
   
  if (ch->flags & BITS_CHANSERV_NOEXPIRE)
    ch->flags &= (~BITS_CHANSERV_NOEXPIRE);
  else
    ch->flags |= BITS_CHANSERV_NOEXPIRE;  
    
  log_command(LOG_CHANSERV, from, "NOEXPIRE", "%s", queue_escape_string(chan));
    
  snprintf(buf, BUFFER_SIZE, "UPDATE chandata SET flags='%ld' WHERE name='%s'", ch->flags, queue_escape_string(ch->name));
  queue_add(buf);
    
  return com_message(sock, conf->cs->numeric, from->numeric, format, CHANSERV_NOEXPIRE_TOGGLED, ch->name, (ch->flags & BITS_CHANSERV_NOEXPIRE)?"On":"Off");
}
