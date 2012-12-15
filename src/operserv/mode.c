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
/* $Id: mode.c,v 1.2 2003/02/25 23:15:03 cure Exp $ */

#include <stdlib.h>
#include <string.h>

#include "operserv.h"
#include "misc_func.h"
#include "config.h"
#include "errors.h"
#include "queue.h"
#include "log.h"

extern sock_info *irc;

#define OPERSERV_MODE_SET         "Modes set."
#define OPERSERV_MODE_USER_NOT_ON "Can't set mode for %s, user is not on the channel."

/**************************************************************************************************
 * operserv_mode
 **************************************************************************************************
 *   MODE <#channel> <modes [mode parameters]>
 *   Changes a mode on a channel. 
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
FUNC_COMMAND(operserv_mode)
{
  dbase_channels *info;  
  char *num = conf->numeric;
  char *chan = getnext(params);
  char *modestr = getnext(params);

  if (!operserv_have_access(from->nickserv->flags, command_info->flags)) return ERROR_NO_ACCESS;

  if (!modestr) return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);

  if ((info = channels_getinfo(-1, chan)))
  {
    long channr = channels_search(chan);
    char minus = '+';
    const char *p;
    char *key = NULL;
    char mode[3], *param, empty[2] = "";
    int mcnt = 0, err = 0;
    char mbuf[BUFFER_SIZE], pbuf[BUFFER_SIZE], lbuf[BUFFER_SIZE];
    pbuf[0] = '\0'; lbuf[0] = '\0';

    if (info->chanserv) num = conf->cs->numeric;
      
    if (!((*modestr == '+') || (*modestr == '-')))
       return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);

    while (*modestr)
    {
      mode[0] = minus; mode[1] = *modestr; mode[2] = '\0';
      param = empty;
      switch (*modestr)
      {
        case '+': case '-':
          minus = *modestr;
          mbuf[mcnt++] = minus;        
          break;
        case 'b':
          if (!(param = getnext(params))) break;
          if (minus == '-') { channels_remban(channr, chan, param); }
          else { channels_addban(channr, chan, param); }
          mbuf[mcnt++] = 'b';
          strcat(pbuf, param); strcat(pbuf, " ");
          strcat(lbuf, param); strcat(lbuf, " ");
          break;
        case 'k':
          if (!(param = getnext(params))) break;
          if (minus == '-')
          {
            if (!info->key) break;
            key = (char *)realloc(key, strlen(info->key) + 1);
            strcpy(key, info->key);
            channels_remkey(channr, chan, key);
            param = key;
          }
          else channels_addkey(channr, chan, param);
          mbuf[mcnt++] = 'k';
          strcat(pbuf, param); strcat(pbuf, " ");
          strcat(lbuf, param); strcat(lbuf, " ");
          break;
        case 'l':
          if (minus == '-') { channels_remlimit(channr, chan); }
          else
          {
            if (!(param = getnext(params))) break;
            channels_addlimit(channr, chan, tr_atoi(param));
            strcat(pbuf, param); strcat(pbuf, " ");
            strcat(lbuf, param); strcat(lbuf, " ");
          }
          mbuf[mcnt++] = 'l';
          break;
        case 'i': case 'm': case 'n': case 'p': case 's': case 't':
          channels_setmode(channr, chan, mode);
          mbuf[mcnt++] = *modestr;
          break;
        case 'o': case 'v':
          if (!(param = getnext(params))) break;
          p = nicks_getnum(param);          
          if (p)
          {
            if (channels_usermode(channr, chan, mode, p) >= 0)
            {
              mbuf[mcnt++] = mode[1];
              strcat(pbuf, p); strcat(pbuf, " ");
              strcat(lbuf, param); strcat(lbuf, " ");
            }
            else
            {
              com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_MODE_USER_NOT_ON, param);              
              err = 1;
            }
          }
          break;
      }
      modestr++;
    } /* while (*modestr) */
    
    xfree(key);

    mbuf[mcnt] = '\0';
        
    if (mcnt > 1)
    {
      com_send(irc, "%s M %s %s %s\n", num, chan, mbuf, pbuf);
      com_wallops(conf->os->numeric, "%s requested mode: %s %s %s\n", from->nick, chan, mbuf, lbuf);
    
      strcpy(pbuf, queue_escape_string(chan));
      log_command(LOG_OPERSERV, from, "MODE", "%s %s %s", pbuf, mbuf, queue_escape_string(lbuf));
    }
    else if (err)
      return 0;
    else
      return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);
      
    return com_message(sock, conf->os->numeric, from->numeric, format, OPERSERV_MODE_SET);
  }
  return com_message(sock, conf->os->numeric, from->numeric, format, command_info->syntax);
}
