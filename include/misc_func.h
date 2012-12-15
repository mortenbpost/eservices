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
/* $Id: misc_func.h,v 1.5 2003/03/01 16:46:13 cure Exp $ */

#ifndef INC_MISC_FUNC_H
#define INC_MISC_FUNC_H

#include "time.h"
#include "dbase.h"

#define bitadd(test, bit) (test |= (1 << ((bit)%32)))
#define bitdel(test, bit) (test &= ~(1 << ((bit)%32)))
#define isbiton(test, bit) (test & (1 << ((bit)%32)))

typedef struct string_chain string_chain;
struct string_chain
{
  string_chain *next;
  char *str;
};

unsigned long tr_atoi(const char *s);
void strip_rn(char *str);
char *skipcolon(char **str);
char *getnext(char **str);
char *getrest(char **str);
char *uppercase(char *str);
unsigned long str64long(char *str);

void xfree(void *ptr);
int time_string_to_int(const char *str);
int wildcard_compare(const char *str, const char *wstr);
int match(const char *mask, const char *string);
const char *ircd_crypt(const char *key, const char *salt);
char *password_crypt(nickserv_dbase_data *nick, const char *pass);
int password_compare(nickserv_dbase_data *nick, const char *pass);
const char *gtime(const time_t *clock);

void string_chain_traverse(void *ptr);
void string_chain_free(void *ptr);

#endif /* INC_MISC_FUNC_H */
