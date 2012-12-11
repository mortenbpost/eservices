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
/* $Id: misc_func.c,v 1.5 2003/03/01 16:46:10 cure Exp $ */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* Make Linux happy */
#define __USE_XOPEN
#include <unistd.h>
#ifdef MD5_PASSWORD
#include <md5.h>
#endif

#include "setup.h"
#include "misc_func.h"
#include "server.h"
#include "dbase.h"
#include "queue.h"

/* declare the external irc socket */
extern sock_info *irc;

static char gtime_str[30];

const char *ircd_crypt(const char *key, const char *salt)
{
  if ((!key) || (!salt)) return NULL;
  return crypt(key, salt);
}

char *password_crypt(nickserv_dbase_data *nick, const char *pass)
{
  char buf[35];
#ifdef MD5_PASSWORD
  MD5Data(pass, strlen(pass), buf);
#else
  strcpy(buf, crypt(pass, nick->userhost));
#endif
  nick->password = (char*)realloc(nick->password, strlen(buf)+1);
  strcpy(nick->password, buf);
  return nick->password;
}

int password_compare(nickserv_dbase_data *nick, const char *pass)
{
#ifdef MD5_PASSWORD
  if (strlen(nick->password) < 25)
  {
    int res = strcmp(nick->password, crypt(pas, nick->password));
    if (!res)
    {
      password_crypt(nick, pass);
      snprintf(buf2, BUFFER_SIZE, "UPDATE nickdata SET password='%s' where nick='%s'", nick->password, queue_escape_string(nick->nick));
      queue_add(buf2);
    }
    return res;
  }
  else
  {
    char buf[35];
    MD5Data(pass, strlen(pass), buf);
    return strcmp(nick->password, buf);
  }
#endif
  return strcmp(nick->password, crypt(pass, nick->password));
}

void strip_rn(char *str)
{
  int i = strlen(str);
  while (i)
  {
    i--;
    if (str[i] == '\r') str[i] = '\0';
    if (str[i] == '\n') str[i] = '\0';
    if (str[i] != '\0') break;
  }
}

unsigned long tr_atoi(const char *s)
{
  const char *p = s;
  if (!s) return 0;
  while(*p)
    if ((*p > '9') || (*p++ < '0'))
      return 0;
  return strtoul(s, (char**)NULL, 10);
}

char *skipcolon(char **str)
{
  char *res = *str;
  if (!str) return NULL;
  if (!*str) return NULL;
  if (!**str) return NULL;
  if (*res == ':') res++;
  while (*res == ' ') res++;
  *str = res;
  return res;
}

char *getrest(char **str)
{
  char *res = *str;
  if (!str) return NULL;
  if (!*str) return NULL;
  if (!**str) return NULL;
  *str = NULL;
  if (res[0] == ':') res++;
  return res;
}

char *getnext(char **str)
{
  char *tmp, *res;
  if (!str) return NULL;
  if (!*str) return NULL;
  if (!**str) return NULL;

  res = *str;
  if (res[0] == ':') return getrest(str);

  if ((tmp = strchr(*str, ' ')))
  {
    *tmp++ = '\0';
    while (*tmp == ' ')
    {
      tmp++;
    }
    *str = tmp;
    return res;
  }
  return getrest(str);
}

unsigned long str64long(char *str)
{
  short base64[75] = \
    {52, 53, 54, 55, 56, 57, 58, 59, 60, 61,100,100,100,100,100,100,100, \
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, \
     17, 18, 19, 20, 21, 22, 23, 24, 25, 62,100, 63,100,100,100, 26, 27, \
     28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, \
     45, 46, 47, 48, 49, 50, 51};
  long bases[6] = {0x1, 0x40, 0x1000, 0x40000, 0x1000000, 0x40000000};
  unsigned int i;
  unsigned long res = 0;
  if (!str) return 0;
  for (i=0; i < strlen(str); i++)
    res += ( bases[strlen(str)-1-i] * base64[str[i] - '0'] );
  return res;
}

void xfree(void *ptr)
{
  if (ptr) free(ptr);
}

int time_string_to_int(const char *str)
{
  const char *p = str;
  char buf[BUFFER_SIZE];
  int pos = 0, tid = 0, i;
  
  if (!str) return -1;
  if (str[0] == '%')
  {
    p++;
    while (*p)
    {
      if ((*p >= '0') && (*p <= '9'))
      {
        buf[pos++] = *p;
        buf[pos] = 0;
      }
      else if ((*p == 's') || (*p == 'S'))
      {
        if (!(i = tr_atoi(buf))) return -1;
        tid += i;
        pos = 0;
        buf[0] = 0;
      }
      else if ((*p == 'm') || (*p == 'M'))
      {
        if (!(i = tr_atoi(buf))) return -1;
        tid += (i * (60));
        pos = 0;
        buf[0] = 0;
      }
      else if ((*p == 'h') || (*p == 'H'))
      {
        if (!(i = tr_atoi(buf))) return -1;
        tid += (i * (60*60));
        pos = 0;
        buf[0] = 0;
      }
      else if ((*p == 'd') || (*p == 'D'))
      {
        if (!(i = tr_atoi(buf))) return -1;
        tid += (i * (60*60*24));
        pos = 0;
        buf[0] = 0;
      }
      else return -1;
      p++;
    }
  }
  else
  {
    tid = tr_atoi(str);
    if (tid == 0) return -1;
  }
  return tid;
}

int wildcard_compare(const char *str, const char *wstr)
{
  return (!match(wstr, str));
#if 0  
  unsigned int i;
  
  if ((!str) || (!wstr)) return 1;
  
  while (1)
  {
    switch (*wstr)
    {
      case 0: return (!*str);
      case '*':
      {
        if (!wstr[0]) return 1;
        while (wstr[1] == '*') wstr++;
        for (i = 0; str[i]; i++)
        {
          if (wildcard_compare(str+i, wstr+1))
            return 1;
        }
        return 0;
      }      
      case '?':
      {
        if (!*str) return 0;
        str++;
        wstr++;
        break;
      }
      default:
      {
        if (!*str) return 0;
        while ((*str) && (*wstr) && (*wstr != '*') && (*wstr != '?'))
        {
          if (tolower(*str) != tolower(*wstr)) return 0;
          str++;
          wstr++;
        }
      }
    }
  }
  return 0;
#endif
}

/*
 *  match - stolen from ircu
 *
 */
int match(const char *mask, const char *string)
{
  const char *m = mask, *s = string;
  char ch;
  const char *bm, *bs;          /* Will be reg anyway on a decent CPU/compiler */
  
  /* Process the "head" of the mask, if any */
  while ((ch = *m++) && (ch != '*'))
    switch (ch)
    {         
      case '\\':      
        if (*m == '?' || *m == '*')
          ch = *m++;               
      default:      
        if (tolower(*s) != tolower(ch))
          return 1;
      case '?':    
        if (!*s++)
          return 1;
    };
  if (!ch)
    return *s;
      
  /* We got a star: quickly find if/where we match the next char */
got_star:
  bm = m;                       /* Next try rollback here */
  while ((ch = *m++))
    switch (ch)      
    {          
      case '?':
        if (!*s++)
          return 1;
      case '*':    
        bm = m;
        continue;               /* while */
      case '\\':                           
        if (*m == '?' || *m == '*')
          ch = *m++;
      default:      
        goto break_while;       /* C is structured ? */
    };             
break_while:
  if (!ch)
    return 0;                   /* mask ends with '*', we got it */
  ch = tolower(ch);
  while (tolower(*s++) != ch)
    if (!*s)
      return 1;
  bs = s;                       /* Next try start from here */
  
  /* Check the rest of the "chunk" */
  while ((ch = *m++))
  {
    switch (ch)
    {         
      case '*':
        goto got_star;
      case '\\':
        if (*m == '?' || *m == '*')
          ch = *m++;
      default:
        if (tolower(*s) != tolower(ch))
        {
          m = bm;
          s = bs;           
          goto got_star;
        };
      case '?':
        if (!*s++)
          return 1;
    };
  };
  if (*s)
  {
    m = bm;
    s = bs;
    goto got_star;
  };
  return 0;
}

const char *gtime(const time_t *clock)
{
  strcpy(gtime_str, asctime(gmtime(clock)));
  gtime_str[24] = ' ';
  strcat(gtime_str, "GMT");  
  return gtime_str; 
}

/**************************************************************************************************
 * string_chain_traverse
 **************************************************************************************************
 *   Loops through the entire string_chain, sending the strings in it as P10 commands
 *   to the server.
 **************************************************************************************************
 * Params:
 *   [IN] void *ptr  : The string_chain to run through
 **************************************************************************************************/
void string_chain_traverse(void *ptr)
{
  string_chain *chain = (string_chain *)ptr;
  while (chain)
  {
    com_send(irc, chain->str);
    chain = chain->next;
  }
}

/**************************************************************************************************
 * string_chain_free
 **************************************************************************************************
 *   Loops through the entire string_chain, freeing the resources.
 **************************************************************************************************
 * Params:
 *   [IN] void *ptr  : The string_chain to run through
 **************************************************************************************************/
void string_chain_free(void *ptr)
{
  string_chain *next, *chain = (string_chain *)ptr;
  while (chain)
  {
    next = chain->next;
    xfree(chain->str);
    xfree(chain);
    chain = next;
  }
}

/**************************************************************************************************
 * uppercase
 **************************************************************************************************
 *  Converts a string into uppercase
 **************************************************************************************************
 * Params:
 *   [IN] char *str  : The string you would like to convert to uppercase.
 * Return:
 *  [OUT] char*      : The converted string.
 **************************************************************************************************/
char *uppercase(char *str)
{
  char *p = str;

  while (*p) 
  {
    *p = toupper(*p);
    *p++;
  }

  return str;
}
