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
/* $Id: operserv.c,v 1.11 2003/02/21 23:17:45 mr Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "setup.h"
#include "misc_func.h"
#include "dbase.h"
#include "config.h"
#include "server.h"
#include "queue.h"
#include "log.h"
#include "parser.h"
#include "chanserv.h"
#include "operserv.h"
#include "nickserv.h"
#include "errors.h"
#include "help.h"

struct operserv_mode_struct
{
  unsigned long flag;
  char sign;
};

static struct operserv_mode_struct operserv_mode_flags[] =
{
  {BITS_OPERSERV_OPER,                'O'},
  {BITS_OPERSERV_GLINE,               'G'},
  {BITS_OPERSERV_MODES,               'M'},
  {0x00040000,                        ' '},
  {0x00080000,                        ' '},
  {0x00100000,                        ' '},
  {0x00200000,                        ' '},
  {0x00400000,                        ' '},
  {0x00800000,                        ' '},
  {BITS_OPERSERV_NS_OPER,             'n'},
  {BITS_OPERSERV_CS_OPER,             'c'},
  {BITS_OPERSERV_DEVELOPER,           'D'},
  {BITS_OPERSERV_DCC,                 'P'},
  {BITS_OPERSERV_NS_ADMIN,            'N'},
  {BITS_OPERSERV_CS_ADMIN,            'C'},
  {BITS_OPERSERV_SERVICES_SUB_ADMIN,  'a'},
  {BITS_OPERSERV_SERVICES_ADMIN,      'A'},
  {0,                                '\0'},
};

/************************************************************************************
                          MISC OPERSERV HELPER FUNCTIONS
 ************************************************************************************/

int operserv_valid_gline(char *userhost)
{
  int wildcards = 0;
  int normal = 0;
  int at_sign = 0;
  int after_at = 0;

  while (*userhost)
  {
    if ((*userhost == '*') || (*userhost == '?')) wildcards++;
    else if (*userhost == '@')
    {
      if (at_sign) return 0;
      at_sign++;
    }
    else if (((*userhost >= '0') && (*userhost <= '9')) ||
             ((*userhost >= 'a') && (*userhost <= 'z')) ||
             ((*userhost >= 'A') && (*userhost <= 'Z')) ||
             (*userhost == '.') || (*userhost == '-') || (*userhost == '_'))
             {
               normal++;
               if (at_sign) after_at++;
             }
    else return 0;
    userhost++;
  }
  return ((after_at > 3) && (normal > wildcards) && (at_sign) && (normal >= 5));
}

char *operserv_flags_to_str(unsigned long flags, char *buf)
{
  int i = 0, j = 0;
  buf[0] = '\0';
  while (operserv_mode_flags[i].flag)
  {
    if (flags & operserv_mode_flags[i].flag)
    {
      buf[j] = operserv_mode_flags[i].sign;
      buf[++j] = '\0';
    }
    i++;
  }
  return buf;
}

unsigned long operserv_str_to_flags(char *str, unsigned long flags, unsigned long perm)
{
  int i, add = 1;  
  while (*str)
  {
    if (*str == '+') add = 1;
    else if (*str == '-') add = 0;
    else
    {
      i = 0;
      while (operserv_mode_flags[i].flag)
      {
        if (*str == operserv_mode_flags[i].sign)
        {
          if (((operserv_mode_flags[i].flag == BITS_OPERSERV_DEVELOPER) && (!(perm & BITS_OPERSERV_DEVELOPER))) ||
             ((operserv_mode_flags[i].flag == BITS_OPERSERV_SERVICES_ADMIN) && (!(perm & (BITS_OPERSERV_DEVELOPER | BITS_OPERSERV_SERVICES_ADMIN)))))
            return 0xffffffff;
          if (add) flags |= operserv_mode_flags[i].flag;
          else     flags &= ~operserv_mode_flags[i].flag;
          break;
        }
        i++;
      }
    }
    str++;
  }
  return flags | BITS_OPERSERV_OPER;
}

char *operserv_flags_to_title(unsigned long flags, char *buf)
{
  if (flags & BITS_OPERSERV_DEVELOPER)
    return strcpy(buf, "Service developer");
  else if (flags & BITS_OPERSERV_SERVICES_ADMIN)
    return strcpy(buf, "Service administrator");
  else if (flags & BITS_OPERSERV_SERVICES_SUB_ADMIN)
    return strcpy(buf, "Service administrator");
  else
  {
    strcpy(buf, "");
    if (flags & BITS_OPERSERV_NS_ADMIN)
    {
      strcat(buf, conf->ns->nick);
      strcat(buf, " admin");
    }
    else if (flags & BITS_OPERSERV_NS_OPER)
    {
      strcat(buf, conf->ns->nick);
      strcat(buf, " oper");
    }

    if (flags & BITS_OPERSERV_CS_ADMIN)
    {
      if (strlen(buf)) strcat(buf, " and ");
      strcat(buf, conf->cs->nick);
      strcat(buf, " admin");
    }
    else if (flags & BITS_OPERSERV_CS_OPER)
    {
      if (strlen(buf)) strcat(buf, " and ");
      strcat(buf, conf->cs->nick);
      strcat(buf, " oper");
    }
    
    if ((!strlen(buf)) && (flags & BITS_OPERSERV_OPER))
      strcat(buf, "Operator");
    
    return buf;
  }
}


int operserv_have_access(unsigned long user, unsigned long flags)
{
  /* Is user an oper, if not, no access */
  if (!(user & BITS_OPERSERV_OPER)) return 0;

  /* is user a developer, if yes, all access */
  if (user & BITS_OPERSERV_DEVELOPER) return 1;

  /* is this a developer-only function */
  if (flags & BITS_OPERSERV_DEVELOPER) return 0;

  /* is user an admin, if yes all access */
  if (user & BITS_OPERSERV_SERVICES_ADMIN) return 1;
  
  /* is user an sub-admin, if yes, access to all but admin-only commands */
  if ((!(flags & BITS_OPERSERV_SERVICES_ADMIN)) && (user & BITS_OPERSERV_SERVICES_SUB_ADMIN)) return 1;
    
  if (flags == BITS_OPERSERV_NS_OPER) flags |= BITS_OPERSERV_NS_ADMIN;
  else if (flags == BITS_OPERSERV_CS_OPER) flags |= BITS_OPERSERV_CS_ADMIN;
  
  /* do the user have the access */
  return (user & flags);
}
