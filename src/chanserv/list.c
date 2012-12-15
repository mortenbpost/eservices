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
/* $Id: list.c,v 1.3 2003/03/01 16:47:04 cure Exp $ */

#include <string.h>

#include "chanserv.h"
#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

extern int chanserv_list_count;
extern chanserv_dbase_channel **chanserv_list;

/**************************************************************************************************
 * chanserv_lister
 **************************************************************************************************
 *   LIST <expired|disabled>
 *     +C
 *     Displays all currently expired channels.
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

FUNC_COMMAND(chanserv_lister)
{
  char *arg = getnext(params);

  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;
 
  if (!arg)
    return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);    

  if (arg)
    arg = uppercase(arg);

  /* listing all expired channels */
  if (!strcmp(arg, "EXPIRED"))
  {
    int i;

    com_message(sock, conf->cs->numeric, from->numeric, format, "Expired channels:");    
  
    for (i = 0; i < chanserv_list_count; i++)
    {
      if (chanserv_list[i]->flags & BITS_CHANSERV_EXPIRED)
      {
        com_message(sock, conf->cs->numeric, from->numeric, format, "  %s %s", gtime((time_t*)&chanserv_list[i]->lastlogin), chanserv_list[i]->name);
      }    
    }
  
    log_command(LOG_CHANSERV, from, "LIST", "EXPIRED");
  } /* listing all disabled channels */
  else if (!strcmp(arg, "DISABLED"))
  {
    int i;

    com_message(sock, conf->cs->numeric, from->numeric, format, "Disabled channels:");    
        
    for (i = 0; i < chanserv_list_count; i++)
    {
      if (chanserv_list[i]->flags & BITS_CHANSERV_DISABLED)      
      {
        if (chanserv_list[i]->flags & BITS_CHANSERV_EXPIRED) continue;
        com_message(sock, conf->cs->numeric, from->numeric, format, "  %s", chanserv_list[i]->name);       
      }
    }
    
    log_command(LOG_CHANSERV, from, "LIST", "DISABLED");
  } /* listing all channels with the noexpire setttings on */
  else if (!strcmp(arg, "NOEXPIRE"))
  {
    int i;

    com_message(sock, conf->cs->numeric, from->numeric, format, "Noexpire channels:");    
        
    for (i = 0; i < chanserv_list_count; i++)
    {
      if (chanserv_list[i]->flags & BITS_CHANSERV_NOEXPIRE)
      {
        com_message(sock, conf->cs->numeric, from->numeric, format, "  %s", chanserv_list[i]->name);       
      }
    }
    
    log_command(LOG_CHANSERV, from, "LIST", "NOEXPIRE");    
  }
  else
    return com_message(sock, conf->cs->numeric, from->numeric, format, command_info->syntax);

  return 0;
}
