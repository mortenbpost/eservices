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
/* $Id: nickserv.c,v 1.19 2003/03/01 16:52:08 mr Exp $ */

#include "setup.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#include "misc_func.h"
#include "dbase.h"
#include "config.h"
#include "server.h"
#include "queue.h"
#include "log.h"
#include "parser.h"
#include "nickserv.h"
#include "errors.h"
#include "help.h"
#include "db_server.h"
#include "chanserv.h"
#include "timer.h"
#include "operserv.h"
#include "dcc.h"

extern sock_info *irc;
  
int nickserv_list_count;
nickserv_dbase_data **nickserv_list;

nickserv_dbase_data *jupe_list[26*26];

/************************************************************************
                      MISC NICKSERV HELPER FUNCTIONS
 ************************************************************************/

/**************************************************************************************************
 * nickserv_new_nick_notice
 **************************************************************************************************
 *   Checks, when a new nick is introduces (connect and renick), if the nick is regged, and
 *   if the user is currently authed to that nick.
 **************************************************************************************************
 * Params:
 *   [IN] sock_info *sock    : The socket from which the data was recieved
 *   [IN] char *numeric      : the users numeric
 *   [IN] char *nick         : The nick
 * Return:
 *  [OUT] int return         : return 0 if success, anything else will cause the services to stop
 **************************************************************************************************/
int nickserv_new_nick_notice(sock_info *sock, char *numeric, char *nick)
{
  nickserv_dbase_data *who;
  dbase_nicks *info = nicks_getinfo(numeric, NULL, -1);
  
  /* is the nick regged */
  if ((who = nickserv_dbase_find_nick(nick)))
  { 
    /* is the user currently authed as nick */
    if (info->nickserv == who)
      return 0;

    /* else write message about the nick being regged */
    if (info->nickserv)
      com_message(sock, conf->ns->numeric, numeric, MODE_NOTICE, NICKSERV_AUTHED_REGNICK, nick, info->nickserv->nick);
    else
      com_message(sock, conf->ns->numeric, numeric, MODE_NOTICE, NICKSERV_NEW_NICK_REGNICK, nick, conf->ns->nick);
  }
  else if (!info->nickserv)
    /* write message about the nick not being regged */
    com_message(sock, conf->ns->numeric, numeric, MODE_NOTICE, NICKSERV_NEW_NICK_NOTREGNICK, nick, conf->ns->nick);
  
  return 0;
}

/**************************************************************************************************
 * nickserv_dbase_update
 **************************************************************************************************
 *   Updates the lastlogin date and user@host in the database
 **************************************************************************************************
 * Params:
 *   [IN] dbase_nicks *info  : dbase_nicks struct in which the data should be update
 **************************************************************************************************/
void nickserv_dbase_update(dbase_nicks *info)
{
  char buf[BUFFER_SIZE];
  /* is the user authed */
  if (!info->nickserv) return;
  
  /* update user@host */
  info->nickserv->userhost = (char *)realloc(info->nickserv->userhost, strlen(info->username)+strlen(info->host)+2*SIZEOF_CHAR);
  strcpy(info->nickserv->userhost, info->username);
  strcat(info->nickserv->userhost, "@");
  strcat(info->nickserv->userhost, info->host);
  /* update lastlogin */
  info->nickserv->lastlogin = time(0);
  /* save to the database */
  snprintf(buf, BUFFER_SIZE, "UPDATE nickdata SET lastlogin='%lu',userhost='%s' WHERE nick='%s'", info->nickserv->lastlogin, info->nickserv->userhost, queue_escape_string(info->nickserv->nick));
  queue_add(buf);
}

/**************************************************************************************************
 * nickserv_dbase_create
 **************************************************************************************************
 *   Wrapper for nickserv_dbase_add, which automatically crypt's the password, with a
 *   random seed.
 **************************************************************************************************
 * Params:
 *   [IN] char *nick         : the nick to add
 *   [IN] char *email        : the user email address
 *   [IN] char *passwd       : the desired password
 * Return;
 *  [OUT] nickserv_dbase_data* : the created entry
 **************************************************************************************************/
nickserv_dbase_data *nickserv_dbase_create(char *nick, char *email, char *passwd)
{
  const char *pwd;
  char sand[3];
  /* randomize the seed */
  sand[0] = 97 + rand()%25;
  sand[1] = 97 + rand()%25;
  sand[2] = '\0';
  /* crypt the password */
  pwd = ircd_crypt(passwd, sand);
  /* create the new regged nick, and return the created entry */
  return nickserv_dbase_add(nick, email, pwd, "", 0, "", BITS_NICKSERV_SECRET, time(0), 4, COMMIT_TO_MYSQL);
}

/**************************************************************************************************
 * nickserv_dbase_generate_password
 **************************************************************************************************
 *   Generates a random [length]-char long password, and stores it in [buf]
 **************************************************************************************************
 * Params:
 *   [IN] char *buf          : the buffer, in which the generated password is stored
 *   [IN] int length         : the length of the desired password
 **************************************************************************************************/
void nickserv_dbase_generate_password(char *buf, int length)
{
  int i;
  /* loop length tims */
  for (i = 0; i < length; i++)
  {
    /* random lowercase letter */
    buf[i] = 97 + rand()%25;
  }
  /* add trailing NUL */
  buf[length] = '\0';
}

/**************************************************************************************************
 * nickserv_dbase_validate_password
 **************************************************************************************************
 *   Checks if the password is correct, and updates the specified dbase_nicks struct.
 **************************************************************************************************
 * Params:
 *   [IN] char *nick         : the nick
 *   [IN] char *passwd       : the password
 *   [IN] dbase_nicks *info  : the dbase_nicks struct to link with the nickserv entry for nick
 * Return;
 *  [OUT] int                : bool values, if the password is false
 **************************************************************************************************/
int nickserv_dbase_validate_password(char *nick, char *passwd, dbase_nicks *info)
{
  nickserv_dbase_data *ns;
  /* is nick regged */
  if (!(ns = nickserv_dbase_find_nick(nick))) return 1;
  /* is it the correct password */
  if (strcmp(ns->password, ircd_crypt(passwd, ns->password)))    
    return 1;
  /* update the dbase_nicks struct */
  info->nickserv = ns;
  info->nickserv->entry = info;
  /* return that the password was correct */
  return 0;
}

/**************************************************************************************************
 * nickserv_dbase_find_nick
 **************************************************************************************************
 *   Wrapper for nickserv_dbase_internal_search function to find the struct for the nick.
 **************************************************************************************************
 * Params:
 *   [IN] char *nick         : the nick to find
 * Return;
 *  [OUT] nickserv_dbase_data* : NULL of not found, else the struct for nick
 **************************************************************************************************/
nickserv_dbase_data *nickserv_dbase_find_nick(char *nick)
{
  int res;  
  if (!nick) return NULL;
  res = nickserv_dbase_internal_search(0, nickserv_list_count-1, nick);
  if (res < 0) return NULL;
  else return nickserv_list[res];
}

/**************************************************************************************************
 * nickserv_dbase_valid_email
 **************************************************************************************************
 *   Test if the specified email is valid using the following requirements:
 *    - exactly one @ in the access
 *    - a minimum of 4 normal characters (a-z, 0-9, _ and -)
 *    - atleast 2 chars after last dot, and max 4
 *    - atleast 1 char before @
 *    - atleast 1 dot after @
 *    - no two dots after eachother
 **************************************************************************************************
 * Params:
 *   [IN] char *email        : the email to test
 * Return;
 *  [OUT] int                : bool value, if the email is valid or not
 **************************************************************************************************/
int nickserv_dbase_valid_email(char *email)
{
  int normal = 0;
  int at_sign = 0;
  int after_dot = 0;
  int dot = 0;
  char last = ' ';
  /* loop through all chars in email */
  while (*email)
  {
    if (*email == '@')
    {
      /* is this the first char (normal == 0) or has there already been a @ */
      if ((at_sign) || (!normal)) return 0;
      at_sign++;
      dot = 0;
      after_dot = 0;
    }    
    else if (*email == '.')
    {
      /* two dots in a row or first char a dot ? */
      if ((last == '.') || (!normal)) return 0;

      dot++;
      /* reset the counter for chars after last dot */
      after_dot = 0;
    }
    else if (((*email >= '0') && (*email <= '9')) ||
             ((*email >= 'a') && (*email <= 'z')) ||
             ((*email >= 'A') && (*email <= 'Z')) ||
             (*email == '-') || (*email == '_'))
             {
               normal++;
               if (dot) after_dot++;
             }
    else return 0;
    last = *email;
    /* next char */
    email++;
  }
  /* test and return */
  return ((after_dot > 1) && ((after_dot < 5)) && (at_sign) && (normal >= 4));
}

/**************************************************************************************************
 * nickserv_dbase_add
 **************************************************************************************************
 *   Create a new entry in the nickserv_list array, and saves the data to the mysql database
 *   if sql is true.
 **************************************************************************************************
 * Params:
 *   [IN] char *nick            : the nick of the entry to create
 *   [IN] char *email           : the email of the entry to create
 *   [IN] char *password        : the password of the entry to create (encrypted form)
 *   [IN] char *userhost        : the user@host of the entry to create
 *   [IN] long lastlogin        : the lastlogin time of the entry to create
 *   [IN] char *info            : the info-line of the entry to create
 *   [IN] unsigned long flags   : the flags of the entry to create
 *   [IN] long regdate          : the date of registration of the entry to create
 *   [IN] unsigned long console : the DCC-console setting of the entry to create
 *   [IN] int sql               : weather it should be saved to mysql or not
 * Return;
 *  [OUT] nickserv_dbase_data   : the created entry
 **************************************************************************************************/
 nickserv_dbase_data *nickserv_dbase_add(const char *nick, const char *email, const char *password, const char *userhost, long lastlogin, char *info, unsigned long flags, long regdate, unsigned long console, int sql)
{
  int nr;
  nickserv_dbase_data *entry;

  /* find the place in the array where the new entry should be placed */
  nr = nickserv_dbase_internal_search(0, nickserv_list_count-1, nick);
  /* ups, an entry already exists! */
  if (nr >= 0) return NULL;

  nr = (1+nr) * -1;
  /* allocate memory for the entry */
  entry = (nickserv_dbase_data *)malloc(sizeof(nickserv_dbase_data));

  /* allocate memory for the data of the entry, and copy the data */
  entry->nick = (char *)malloc(strlen(nick)+SIZEOF_CHAR);
  strcpy(entry->nick, nick);

  entry->email = (char *)malloc(strlen(email)+SIZEOF_CHAR);
  strcpy(entry->email, email);

  entry->password = (char *)malloc(strlen(password)+SIZEOF_CHAR);
  strcpy(entry->password, password);

  entry->userhost = (char *)malloc(strlen(userhost)+SIZEOF_CHAR);
  strcpy(entry->userhost, userhost);

  entry->info = (char *)malloc(strlen(info)+SIZEOF_CHAR);
  strcpy(entry->info, info);

  entry->comments = NULL;
  entry->comment_count = 0;
  
  entry->notices = NULL;
  entry->notice_count = 0;

  entry->lastlogin = lastlogin;
  entry->regdate = regdate;
  entry->flags = flags;
  entry->console = console;
  
  entry->entry = NULL;

  entry->access_count = 0;
  entry->access = NULL;

  /* rearrange the array, and insert the new entry */
  nickserv_list = (nickserv_dbase_data**)realloc(nickserv_list, (nickserv_list_count+1) * SIZEOF_VOIDP);
  if (nr < (nickserv_list_count++)) memmove(&nickserv_list[nr+1], &nickserv_list[nr], (nickserv_list_count - nr - 1) * SIZEOF_VOIDP);
  nickserv_list[nr] = entry;

  /* is sql is specified, save the new entry to mysql aswell */
  if (sql)
  {
    char buf[BUFFER_SIZE], bnick[BUFFER_SIZE];
    queue_escape_string_buf(nick, bnick);
    snprintf(buf, BUFFER_SIZE, "INSERT INTO nickdata (nick,email,password,lastlogin,userhost,info,flags,regdate,console) VALUES ('%s','%s','%s',%lu,'%s','%s',%lu,%lu,%lu)", bnick, email, entry->password, lastlogin, userhost, queue_escape_string(info), flags, regdate, console);
    queue_add(buf);
  }
  
  /* return the entry */
  return entry;
}

/**************************************************************************************************
 * nickserv_dbase_unreg
 **************************************************************************************************
 *   Unregisters the nick, and removes the nick from all channel-access-lists.
 **************************************************************************************************
 * Params:
 *   [IN] cnickserv_dbase_data *ns : the entry containing the nick to unreg.
 * Return;
 *  [OUT] int                      : bool value if failure or not
 **************************************************************************************************/
int nickserv_dbase_unreg(nickserv_dbase_data *ns)
{
  int cnt = ns->access_count;
  /* loop as long as user still have access in a channel */
  while (ns->access_count > 0)
  {
    /* is the user the owner of the channel, in that case, unreg the channel */
    if (ns->access[0]->level == 500)
      chanserv_dbase_delete(ns->access[0]->channel);
    else
      /* remove the user from the channels access */
      chanserv_dbase_access_delete(ns->access[0]->channel, ns, COMMIT_TO_MYSQL);
    /* just a little precausion, so it won't endless-loop, if access can't be removed */
    if (--cnt < 0) break;
  }
  /* delete the nick from the database */
  return nickserv_dbase_del(ns->nick, COMMIT_TO_MYSQL);
}

/**************************************************************************************************
 * nickserv_dbase_del
 **************************************************************************************************
 *   Delete the nick from the nickserv list, and removes it from the mysql database if sql = true
 **************************************************************************************************
 * Params:
 *   [IN] char *nick        : the nick to unreg.
 *   [IN] int  sql          : remove from mysql aswell ?
 * Return;
 *  [OUT] int               : bool value if failure or not
 **************************************************************************************************/
int nickserv_dbase_del(char *nick, int sql)
{
  char n[MAX_NICK_LEN];
  int index, i, j;  
  nickserv_dbase_data *entry;
  index = nickserv_dbase_internal_search(0, nickserv_list_count-1, nick);
  if (index < 0) return 0;
  entry = nickserv_list[index];
  
  /* copy the nick, since it is probably going to be free'd later */
  queue_escape_string_buf(entry->nick, n);

  /* Free the access list */
  for (i = 0; i < entry->access_count; i++)
  {
    chanserv_dbase_channel *chn = entry->access[i]->channel;
    for (j = 0; j < chn->access_count; j++)
      if (chn->access[j] == entry->access[i])
        chn->access[j] = chn->access[--chn->access_count];
    xfree(entry->access[i]);
  }

  /* Free the comment list */
  for (i = 0; i < entry->comment_count; i++)
  {
    xfree(entry->comments[i]->nick);
    xfree(entry->comments[i]->comment);
    xfree(entry->comments[i]);    
  }

  /* Free the notice list */
  for (i = 0; i < entry->notice_count; i++)
  {
    xfree(entry->notices[i]->nick);
    xfree(entry->notices[i]->comment);
    xfree(entry->notices[i]);
  }
  /* Free all entries in the struct, and the struct aswell */
  xfree(entry->comments);
  xfree(entry->notices);
  xfree(entry->access);
  xfree(entry->nick);
  xfree(entry->password);
  xfree(entry->email);
  xfree(entry->userhost);
  xfree(entry->info);
  xfree(entry);
  
  /* rearrange the array */
  memmove(&nickserv_list[index], &nickserv_list[index+1], (nickserv_list_count - index - 1) * SIZEOF_VOIDP);
  nickserv_list = (nickserv_dbase_data**)realloc(nickserv_list, (--nickserv_list_count) * SIZEOF_VOIDP);

  /* if sql = true, remove from mysql aswell */
  if (sql)
  {
    char buf[BUFFER_SIZE];
    snprintf(buf, BUFFER_SIZE, "DELETE FROM notice WHERE nick='%s'", n);
    queue_add(buf);
    snprintf(buf, BUFFER_SIZE, "DELETE FROM comment WHERE subject='%s'", n);
    queue_add(buf);
    snprintf(buf, BUFFER_SIZE, "DELETE FROM nickdata WHERE nick='%s'", n);
    queue_add(buf);
    snprintf(buf, BUFFER_SIZE, "DELETE FROM access WHERE nick='%s'", n);
    queue_add(buf);
  }
  
  return 0;
}

/**************************************************************************************************
 * nickserv_dbase_internal_search
 **************************************************************************************************
 *   A binary search rutine (recursive), finding the index of the specified nick (is present)
 *   else it returns a negative number, representing the position in which the entry should be.
 **************************************************************************************************
 * Params:
 *   [IN] long low          : Devide-and-Conquer, lower limit
 *   [IN] long high         : Devide-and-Conquer, upper limit
 *   [IN] char *nick        : the nick to find between lower and upper limit 
 * Return;
 *  [OUT] int               : if >= 0, index where nick was found, if < 0, negative index of 
 *                           where it shold have been (-1-index)
 **************************************************************************************************/
int nickserv_dbase_internal_search(long low, long high, const char *nick)
{
  int res;
  long mid = high - ((high - low) / 2);
  if (low > high) return -1-low;
  res = strcasecmp(nick, nickserv_list[mid]->nick);
  if (res < 0) return nickserv_dbase_internal_search(low, mid-1, nick);
  else if (res > 0) return nickserv_dbase_internal_search(mid+1, high, nick);
  else return mid;
}

/**************************************************************************************************
 * nickserv_dbase_cleanup
 **************************************************************************************************
 *   Removes the entire internal database from memory
 **************************************************************************************************
 **************************************************************************************************/
void nickserv_dbase_cleanup(void)
{
  int i, j;
  debug_out(" | |==> Cleaning NickServ database...\n");
  
  /* loop through the entire nickserv_list array, and free all memory */
  for (i = 0; i < nickserv_list_count; i++)
  {
    xfree(nickserv_list[i]->nick);
    xfree(nickserv_list[i]->email);
    xfree(nickserv_list[i]->password);
    xfree(nickserv_list[i]->userhost);
    xfree(nickserv_list[i]->info);
    
    for (j = 0; j < nickserv_list[i]->access_count; j++)
      xfree(nickserv_list[i]->access[j]);
    xfree(nickserv_list[i]->access);
    
    for (j = 0; j < nickserv_list[i]->comment_count; j++)
    {
      xfree(nickserv_list[i]->comments[j]->nick);
      xfree(nickserv_list[i]->comments[j]->comment);
      xfree(nickserv_list[i]->comments[j]);
    }
    xfree(nickserv_list[i]->comments);
    
    for (j = 0; j < nickserv_list[i]->notice_count; j++)
    {
      xfree(nickserv_list[i]->notices[j]->nick);
      xfree(nickserv_list[i]->notices[j]->comment);
      xfree(nickserv_list[i]->notices[j]);
    }
    xfree(nickserv_list[i]->notices);
    xfree(nickserv_list[i]);
  }
  xfree(nickserv_list);
  nickserv_list_count = 0;
}

/**************************************************************************************************
 * nickserv_dbase_setbit
 **************************************************************************************************
 *   Changes the users flags
 **************************************************************************************************
 * Params:
 *   [IN] nickserv_dbase_data *nick: the ctruct inwhich the flags should be changed
 *   [IN] long bit          : the bit(s) to set
 *   [IN] int sql           : if the change should be written to mysql or not
 * Return;
 *  [OUT] int               : bool value if failure or not
 **************************************************************************************************/
int nickserv_dbase_setbit(nickserv_dbase_data *nick, unsigned long bit, int sql)
{
  /* nick = NULL, failure */
  if (!nick) return 1;

  /* set the bits, by OR'ing it to the existing flags */
  nick->flags = nick->flags | bit;

  /* if sql set, save to the database */
  if (sql)
  {
    char buf[BUFFER_SIZE];
    snprintf(buf, BUFFER_SIZE, "UPDATE nickdata SET flags=%lu WHERE nick='%s'", nick->flags, queue_escape_string(nick->nick));
    queue_add(buf);
  }
  return 0;
}

/**************************************************************************************************
 * nickserv_dbase_removebit
 **************************************************************************************************
 *   Changes the users flags
 **************************************************************************************************
 * Params:
 *   [IN] nickserv_dbase_data *nick: the ctruct inwhich the flags should be changed
 *   [IN] long bit          : the bit(s) to remove
 *   [IN] int sql           : if the change should be written to mysql or not
 * Return;
 *  [OUT] int               : bool value if failure or not
 **************************************************************************************************/
int nickserv_dbase_removebit(nickserv_dbase_data *nick, unsigned long bit, int sql)
{
  /* if nick == NULL, failure */
  if (!nick) return 1;

  /* remove the bits from the flags */
  nick->flags = nick->flags & ~bit;

  /* if sql is set, save to the database */
  if (sql)
  {
    char buf[BUFFER_SIZE];
    snprintf(buf, BUFFER_SIZE, "UPDATE nickdata SET flags=%lu WHERE nick='%s'", nick->flags, queue_escape_string(nick->nick));
    queue_add(buf);
  }
  return 0;
}

/**************************************************************************************************
 * nickserv_dbase_checkbit
 **************************************************************************************************
 *   Checks if the specified flag is set
 **************************************************************************************************
 * Params:
 *   [IN] nickserv_dbase_data *nick: the struct containing the flags to be tested
 *   [IN] long bit          : the bit(s) to to test is set
 * Return;
 *  [OUT] int               : bool value if set or not
 **************************************************************************************************/
int nickserv_dbase_checkbit(nickserv_dbase_data *nick, unsigned long bit)
{
  if (!nick) return 0;
  return (nick->flags & bit);
}

/**************************************************************************************************
 * nickserv_dbase_set_is_ok
 **************************************************************************************************
 *   Converts the specified string to a numeric representation.
 *     0: if not recognized
 *     1: if off, no or 0
 *     2: if on, yes or 1
 **************************************************************************************************
 * Params:
 *   [IN] nickserv_dbase_data *nick: the ctruct inwhich the flags should be changed
 *   [IN] long bit          : the bit(s) to set
 *   [IN] int sql           : if the change should be written to mysql or not
 * Return;
 *  [OUT] int               : 0, 1 or 2, depending on the string
 **************************************************************************************************/
int nickserv_dbase_set_is_ok(char *str)
{
  if (!str) return 0;
  
  /* uppercase the string */
  str = uppercase(str);

  /* compare to known words */
  if (!strcmp(str, "YES")) return 2;
  else if (!strcmp(str, "ON")) return 2;
  else if (!strcmp(str, "1")) return 2;
  else if (!strcmp(str, "OFF")) return 1;
  else if (!strcmp(str, "NO")) return 1;
  else if (!strcmp(str, "0")) return 1;

  /* not a known word */
  return 0;
}

/**************************************************************************************************
 * nickserv_dbase_valid_nick
 **************************************************************************************************
 *   Checks if the specified nick is valid, ie not containing any invalid characters
 **************************************************************************************************
 * Params:
 *   [IN] char *nick:       the nick to test
 * Return;
 *  [OUT] int               : bool value if 
 **************************************************************************************************/
int nickserv_dbase_valid_nick(char *nick)
{
  char *p = nick;
  int first = 1;
  if (strlen(nick) >= MAX_NICK_LEN) return 0;
  while (*p)
  {
    if ((*p < ':') && first) return 0;
    else if ((*p < '0') && (*p != '-')) return 0;
    else if ((*p > '9') && (*p < 'A')) return 0;
    else if (*p > '}') return 0;
    p++;
    first = 0;
  }
  return 1;
}

/**************************************************************************************************
 * nickserv_dbase_init_jupes
 **************************************************************************************************
 *   Introduces the juped nicks to the network when services are connecting
 **************************************************************************************************
 * Return;
 *  [OUT] int               : bool value if failure or not
 **************************************************************************************************/
int nickserv_dbase_init_jupes(sock_info *sock)
{
  int i, next = 0;
  char num[4] = "JAA";
  
  /* init the jupe_list to NULL */
  for (i = 0; i < 26*26; i++)
    jupe_list[i] = NULL;

  /* loop through all entries in nickserv_list */
  for(i = 0; i < nickserv_list_count; i++)
  {
    /* is the nick a juped nick */
    if (nickserv_list[i]->flags & BITS_NICKSERV_JUPED)
    {
      /* enough room for it ? */
      if (next >= (26*26)) return 0;

      /* calculate a numeric */
      num[1] = 'A' + (next / 26);
      num[2] = 'A' + (next % 26);
      
      /* add it ti jupe_list */
      jupe_list[next++] = nickserv_list[i];
      
      /* introduce it to the network */
      com_send(irc, "%s N %s 1 %ld juped %s +kid xXxXxX %s%s :%s\n", conf->numeric, nickserv_list[i]->nick, JUPE_TIME, NETWORK_NAME, conf->numeric, num, nickserv_list[i]->info);
    }
  }
  return 0;
}

/**************************************************************************************************
 * nickserv_dbase_email_exists
 **************************************************************************************************
 *   Checks if the specified email-address is already in use in another account
 **************************************************************************************************
 * Params:
 *   [IN] char *email       : the email to find
 * Return;
 *  [OUT] int               : bool value if it exists or not
 **************************************************************************************************/
int nickserv_dbase_email_exists(char *email)
{
  int i;
  /* loop through the entire list, and compare email-addresses */
  for (i = 0; i < nickserv_list_count; i++)
    if (!strcasecmp(nickserv_list[i]->email, email))
      /* return Found */
      return 1;

  /* return Not Found */
  return 0;
}

/**************************************************************************************************
 * nickserv_dbase_checkold
 **************************************************************************************************
 *   Checks if any nicks are too old, and expire those, unless they are jupes or tagged noexpire
 *   and automatically insert a new timer in the timer que.
 **************************************************************************************************
 * Params:
 *   [IN] void *ptr         : (nothing)
 **************************************************************************************************/
void nickserv_dbase_checkold(void *ptr)
{
  int i;
  /* crate a new timer-object */
  timer_event *te = (timer_event*)malloc(sizeof(timer_event));
  unsigned long expire = (unsigned long)time(0) - (NICKSERV_EXPIRE_TIME);

  /* loop through all entries in the nickserv_list */
  for (i = 0; i < nickserv_list_count; i++)
    /* if the entry is not marked noexpire, is not an oper and is not a juped nick */
    if (!(nickserv_list[i]->flags & (BITS_NICKSERV_NOEXPIRE | BITS_OPERSERV_OPER | BITS_NICKSERV_JUPED)))
      /* test if the entries lastlogin time is older then the expire time */
      if (nickserv_list[i]->lastlogin < expire)
      {
        /* if yes, log the expire */
        log_command(LOG_NICKSERV, NULL, "[EXPIRE]", "%s %lu", nickserv_list[i]->nick, nickserv_list[i]->lastlogin);        

        /* remove the nick from the array */
        nickserv_dbase_unreg(nickserv_list[i]);
        i--;
      }

  /* set the variables in the timer-object, and add it to the timer-queue */
  te->data = NULL;
  te->func = nickserv_dbase_checkold;
  te->free = NULL;
  timer_add(te, NICKSERV_EXPIRE_CHECK);
}

/**************************************************************************************************
 * nickserv_dbase_get_juped
 **************************************************************************************************
 *   Returns the nickserv_dbase_data struct for the jupe with index nr
 **************************************************************************************************
 * Params:
 *   [IN] int nr               : the index of the jupe which info is wanted
 * Return;
 *  [OUT] nickserv_dbase_data* : NULL if not found, else the jupe
 **************************************************************************************************/
nickserv_dbase_data *nickserv_dbase_get_juped(int nr)
{
  return jupe_list[nr];
}

/**************************************************************************************************
 * nickserv_dbase_op_on_auth
 **************************************************************************************************
 *   Checks if the user is in any channels he/she has autoop in, when the user authes
 **************************************************************************************************
 * Params:
 *   [IN] dbase_nick *nick  : the user-object
 **************************************************************************************************/
void nickserv_dbase_op_on_auth(dbase_nicks *nick)
{
  int i;
  /* if not authed, error */
  if (!nick->nickserv) return;

  /* loop through all the channel the user is in */
  for (i = 0; i < nick->channels_count; i++)
  {
    chanserv_dbase_access *acc;
    /* check to see if the user has access in the channel */
    acc = chanserv_dbase_find_access(nick->nickserv, nick->channels[i]->channel->chanserv);
    if (!acc) continue;

    /* does the user have autoop set for this channel */
    if (acc->autoop)
    {
      /* is the user already opped ? */
      if (nick->channels[i]->mode & 2) continue;
      /* Send mode-string to server */
      com_send(irc, "%s M %s +o %s\n", conf->cs->numeric, nick->channels[i]->channel->name, nick->numeric);
      /* Update the internal database to the mode-change */
      channels_usermode(-1, nick->channels[i]->channel->name, "+o", nick->numeric);
      chanserv_dbase_update_lastlogin(nick->channels[i]->channel->chanserv);
    }
  }
}

/**************************************************************************************************
 * nickserv_dbase_comment_add
 **************************************************************************************************
 *   Attach a comment to the nick
 **************************************************************************************************
 * Params:
 *   [IN] nickserv_dbase_data *who  : the nick the comment should be attached to
 *   [IN] nickserv_dbase_data *nick : the person writing the comment
 *   [IN] const char *comment       : the comment
 *   [IN] long date                 : the ctime the comment was added.
 *   [IN] int sql                   : if the comment should be saved to mysql
 * Return;
 *  [OUT] int                       : bool value if failure or not
 **************************************************************************************************/
int nickserv_dbase_comment_add(nickserv_dbase_data *who, nickserv_dbase_data *nick, const char *comment, long date, int sql)
{
  /* expand the comment array for the user, to make room for the new comment */
  who->comments = (dbase_comment**)realloc(who->comments, (++who->comment_count)*SIZEOF_VOIDP);
  who->comments[who->comment_count-1] = (dbase_comment*)malloc(sizeof(dbase_comment));
  
  /* save the person who wrote the comment */
  who->comments[who->comment_count-1]->nick = (char*)malloc(strlen(nick->nick)+SIZEOF_CHAR);
  strcpy(who->comments[who->comment_count-1]->nick, nick->nick);

  /* save the comment */
  who->comments[who->comment_count-1]->comment = (char*)malloc(strlen(comment)+SIZEOF_CHAR);
  strcpy(who->comments[who->comment_count-1]->comment, comment);
  
  /* save the time the comment was written */
  who->comments[who->comment_count-1]->date = date;
  
  /* if sql is set, save to mysql */
  if (sql)
  {
    char buf[BUFFER_SIZE], bnick[BUFFER_SIZE], bwho[BUFFER_SIZE];
    
    queue_escape_string_buf(nick->nick, bnick);
    queue_escape_string_buf(who->nick, bwho);
    snprintf(buf, BUFFER_SIZE, "INSERT INTO comment (subject, nick, comment, com_date) VALUES ('%s','%s','%s',%lu)", bwho, bnick, queue_escape_string(comment), date);
    queue_add(buf);
  }
  
  /* return success */
  return 0;
}

/**************************************************************************************************
 * nickserv_dbase_comment_remove
 **************************************************************************************************
 *   Remove a comment from a nick
 **************************************************************************************************
 * Params:
 *   [IN] nickserv_dbase_data *who  : the nick the comment should be removed from
 *   [IN] unsigned long nr          : the index of the comment to remove
 *   [IN] int sql                   : if the comment should be saved to mysql
 * Return;
 *  [OUT] int                       : bool value if failure or not
 **************************************************************************************************/
int nickserv_dbase_comment_del(nickserv_dbase_data *who, unsigned long nr, int sql)
{
  dbase_comment *comment;
  /* if nr is greater than total count, return error */
  if (nr >= who->comment_count) return 1;
  
  comment = who->comments[nr];

  /* rearrange the array */
  memmove(&who->comments[nr], &who->comments[nr+1], (who->comment_count - nr - 1)*SIZEOF_VOIDP);
  who->comments = (dbase_comment**)realloc(who->comments, (--who->comment_count)*SIZEOF_VOIDP);
  
  /* if sql is set, remove from mysql aswell */
  if (sql)
  {
    char buf[BUFFER_SIZE];
    snprintf(buf, BUFFER_SIZE, "DELETE FROM comment WHERE com_date='%lu' AND nick='%s'", comment->date, queue_escape_string(comment->nick));
    queue_add(buf);
  }
  
  /* free the entry from memory */
  xfree(comment->nick);
  xfree(comment->comment);
  xfree(comment);

  /* return success */
  return 0;
}

/**************************************************************************************************
 * nickserv_dbase_notice
 **************************************************************************************************
 *   Add a on-connect notice to the user, or sends a message if the user is online
 **************************************************************************************************
 * Params:
 *   [IN] nickserv_dbase_data *who  : the nick the notice should be added to
 *   [IN] char *msg                 : the notice
 * Return;
 *  [OUT] int                       : bool value if failure or not
 **************************************************************************************************/
int nickserv_dbase_notice(nickserv_dbase_data *who, const char *msg, ...)
{
  char bufmsg[BUFFER_SIZE], buf[BUFFER_SIZE], buf1[MAX_NICK_LEN], buf2[MAX_NICK_LEN], buf3[BUFFER_SIZE];
  time_t now = time(0);
  va_list ag;

  if (!msg) return 0;

  va_start(ag, msg);
  vsnprintf(bufmsg, BUFFER_SIZE, msg, ag);
  va_end(ag);
  
  /* is the user online */
  if (who->entry)
  {
    /* send the notice now */
    com_message(irc, conf->ns->numeric, who->entry->numeric, (who->flags & BITS_NICKSERV_PRIVMSG)?MODE_PRIVMSG:MODE_NOTICE, "Notice from services:");
    com_message(irc, conf->ns->numeric, who->entry->numeric, (who->flags & BITS_NICKSERV_PRIVMSG)?MODE_PRIVMSG:MODE_NOTICE, "[%s] %s", gtime(&now), bufmsg);
    return 0;
  }
  
  /* add the notice to the users list of pending notices */
  who->notices = (dbase_comment**)realloc(who->notices, (++who->notice_count) * SIZEOF_VOIDP);
  who->notices[who->notice_count-1] = (dbase_comment*)malloc(sizeof(dbase_comment));
  
  who->notices[who->notice_count-1]->date = now;
  who->notices[who->notice_count-1]->nick = (char*)malloc(strlen(conf->ns->nick) + SIZEOF_CHAR);
  who->notices[who->notice_count-1]->comment = (char*)malloc(strlen(bufmsg) + SIZEOF_CHAR);
  strcpy(who->notices[who->notice_count-1]->nick, conf->ns->nick);
  strcpy(who->notices[who->notice_count-1]->comment, bufmsg);
  
  /* save the notice to the database */
  snprintf(buf, BUFFER_SIZE, "INSERT INTO notice (com_date, nick, sender, notice) VALUES ('%ld', '%s', '%s', '%s')", (long)now, queue_escape_string_buf(who->nick, buf1), queue_escape_string_buf(conf->ns->nick, buf2), queue_escape_string_buf(bufmsg, buf3));
  queue_add(buf);
  
  return 0;
}

/**************************************************************************************************
 * nickserv_dbase_notice_check
 **************************************************************************************************
 *   Check for on-connect notices, and sends them to the user
 **************************************************************************************************
 * Params:
 *   [IN] dbase_nicks *from         : the user to check if he has notices
 * Return;
 *  [OUT] int                       : bool value if failure or not
 **************************************************************************************************/
int nickserv_dbase_notice_check(dbase_nicks *from)
{
  int i;
  char buf[BUFFER_SIZE], buf2[MAX_NICK_LEN];
  nickserv_dbase_data *who;
  /* is the user authed */
  if (!(who = from->nickserv)) return 0;
  
  /* does the user have any pending motices? */
  if (who->notice_count < 1) return 0;

  /* Write header */
  com_message(irc, conf->ns->numeric, from->numeric, (who->flags & BITS_NICKSERV_PRIVMSG)?MODE_PRIVMSG:MODE_NOTICE, "Pending notices from services:");
  /* loop through all the notices */
  for (i = 0; i < who->notice_count; i++)
  {
    com_message(irc, conf->ns->numeric, from->numeric, (who->flags & BITS_NICKSERV_PRIVMSG)?MODE_PRIVMSG:MODE_NOTICE, "[%s] %s", gtime((time_t*)&who->notices[i]->date), who->notices[i]->comment);
    /* free the notices from memory now that they have been shown */
    xfree(who->notices[i]->comment);
    xfree(who->notices[i]->nick);
    xfree(who->notices[i]);
  }
  
  /* reset the notice count */
  xfree(who->notices);
  who->notices = NULL;
  who->notice_count = 0;
  
  /* delete from database */  
  snprintf(buf, BUFFER_SIZE, "DELETE FROM notice WHERE nick='%s'", queue_escape_string_buf(who->nick, buf2));
  queue_add(buf);
  
  return 0;
}
