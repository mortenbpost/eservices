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
/* $Id: chanserv.c,v 1.23 2003/03/01 16:47:02 cure Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#include "setup.h"
#include "misc_func.h"
#include "dbase.h"
#include "config.h"
#include "server.h"
#include "queue.h"
#include "log.h"
#include "parser.h"
#include "chanserv.h"
#include "nickserv.h"
#include "errors.h"
#include "help.h"
#include "channels.h"
#include "operserv.h"
#include "timer.h"

/* declare the external irc socket */
extern sock_info *irc;

/* declare the array of regged channels */
int chanserv_list_count;
chanserv_dbase_channel **chanserv_list;

/************************************************************************************
                          MISC CHANSERV HELPER FUNCTIONS
 ************************************************************************************/

int chanserv_dbase_disabled(chanserv_dbase_channel *ch)
{
  return (ch->flags & BITS_CHANSERV_DISABLED);
}

chanserv_dbase_access *chanserv_dbase_has_access(char *nick, chanserv_dbase_channel *chan)
{
  dbase_nicks *info;
  nickserv_dbase_data *data;
  if (!nick) return NULL;
  if (!chan) return NULL;
  
  if ((data = nickserv_dbase_find_nick(nick)))
    return chanserv_dbase_find_access(data, chan);

  else if ((info = nicks_getinfo(NULL, nick, -1)))
    return chanserv_dbase_find_access(info->nickserv, chan);
  
  return NULL;
}

chanserv_dbase_channel *chanserv_dbase_find_chan(const char *chan)
{
  int res;
  res = chanserv_dbase_internal_search(0, chanserv_list_count-1, chan);
  if (res < 0) return NULL;
  else return chanserv_list[res];
}

chanserv_dbase_access *chanserv_dbase_find_access(nickserv_dbase_data *ns, chanserv_dbase_channel *chan)
{
  int i;
  if ((!ns) || (!chan)) return NULL;
  
  for (i = 0; i < ns->access_count; i++)
    if (ns->access[i]->channel == chan)
      return ns->access[i];
  
  return NULL;
}

int chanserv_dbase_check_access(nickserv_dbase_data *ns, chanserv_dbase_channel *chan, int level)
{
  chanserv_dbase_access *acc;
  if ((acc = chanserv_dbase_find_access(ns, chan)))
    if (acc->level >= level) return acc->level;
  return 0;
}

int chanserv_dbase_access_add(chanserv_dbase_channel *chan, nickserv_dbase_data *nick, int level, int autoop, int sql)
{
  chanserv_dbase_access *acc;
  acc = (chanserv_dbase_access *)malloc(sizeof(chanserv_dbase_access));
  if (!acc) return -1;

  acc->nick = nick;
  acc->channel = chan;
  acc->level = level % (CHANSERV_LEVEL_OWNER + 1);
  acc->autoop = autoop % 2;

  chan->access = (chanserv_dbase_access **)realloc(chan->access, (++chan->access_count)*sizeof(chanserv_dbase_access*));
  chan->access[chan->access_count-1] = acc;
  nick->access = (chanserv_dbase_access **)realloc(nick->access, (++nick->access_count)*sizeof(chanserv_dbase_access*));
  nick->access[nick->access_count-1] = acc;

  if (sql)
  {
    char buf[BUFFER_SIZE], cbuf[BUFFER_SIZE];
    int lvl = acc->level | (acc->autoop << 16);
    strcpy(cbuf, queue_escape_string(acc->channel->name));
    snprintf(buf, BUFFER_SIZE, "INSERT INTO access (channel,nick,level) VALUES ('%s','%s','%d')", cbuf, queue_escape_string(acc->nick->nick), lvl);
    queue_add(buf);
  }

  return 0;
}

int chanserv_dbase_access_delete(chanserv_dbase_channel *chan, nickserv_dbase_data *nick, int sql)
{
  chanserv_dbase_access *acc;
  int i;

  if (sql)
  {
    char buf[BUFFER_SIZE], cbuf[BUFFER_SIZE];
    strcpy(cbuf, queue_escape_string(chan->name));
    snprintf(buf, BUFFER_SIZE, "DELETE FROM access WHERE nick='%s' AND channel='%s'", queue_escape_string(nick->nick), cbuf);
    queue_add(buf);
  }

  acc = chan->access[chan->access_count-1];
  for (i = 0; i < chan->access_count; i ++)
  {
    if (chan->access[i]->nick == nick)
    {
      chan->access[i] = acc;
      break;
    }
  }

  acc = nick->access[nick->access_count-1];
  for (i = 0; i < nick->access_count; i ++)
  {
    if (nick->access[i]->channel == chan)
    {
      xfree(nick->access[i]);
      nick->access[i] = acc;
      break;
    }
  }

  chan->access = (chanserv_dbase_access **)realloc(chan->access, (--chan->access_count)*sizeof(chanserv_dbase_access*));
  nick->access = (chanserv_dbase_access **)realloc(nick->access, (--nick->access_count)*sizeof(chanserv_dbase_access*));
  return 0;
}

chanserv_dbase_channel *chanserv_dbase_create(char *chan, dbase_nicks *from)
{
  chanserv_dbase_channel *entry;
  if ((entry = chanserv_dbase_add(chan, from->nickserv->nick, "", "", 0, (unsigned long)time(0), COMMIT_TO_MYSQL)))
  {
    if (chanserv_dbase_access_add(entry, from->nickserv, CHANSERV_LEVEL_OWNER, 1, COMMIT_TO_MYSQL))
      return NULL;
  }
  return entry;
}

chanserv_dbase_channel *chanserv_dbase_add(char *chan, char *owner, char *topic, char *mode, unsigned long flags, unsigned long lastlogin, int sql)
{
  int nr;
  chanserv_dbase_channel *entry;

  nr = chanserv_dbase_internal_search(0, chanserv_list_count-1, chan);
  if (nr >= 0) return NULL;

  nr = (nr+1) * -1;
  entry = (chanserv_dbase_channel *)malloc(sizeof(chanserv_dbase_channel));

  entry->name = (char *)malloc(strlen(chan)+1);
  strcpy(entry->name, chan);

  entry->owner = (char *)malloc(strlen(owner)+1);
  strcpy(entry->owner, owner);

  entry->topic = (char *)malloc(strlen(topic)+1);
  strcpy(entry->topic, topic);

  entry->keepmode = (char *)malloc(strlen(mode)+1);
  strcpy(entry->keepmode, mode);

  entry->comments = NULL;
  entry->comment_count = 0;

  entry->flags = flags;
  entry->lastlogin = lastlogin;

  entry->access = NULL;
  entry->access_count = 0;
  
  entry->bans = NULL;
  entry->bancount = 0;

  chanserv_list = (chanserv_dbase_channel**)realloc(chanserv_list, (chanserv_list_count+1) * sizeof(chanserv_dbase_channel*));
  if (nr < (chanserv_list_count++)) memmove(&chanserv_list[nr+1], &chanserv_list[nr], (chanserv_list_count - nr - 1) * sizeof(chanserv_dbase_channel*));
  chanserv_list[nr] = entry;

  if (sql)
  {
    char buf[3*BUFFER_SIZE], obuf[BUFFER_SIZE];
    strcpy(obuf, queue_escape_string(owner));
    snprintf(buf, 3*BUFFER_SIZE, "INSERT INTO chandata (name,owner,topic,keepmode,flags,lastlogin) VALUES ('%s','%s','','',%lu,%lu)", queue_escape_string(chan), obuf, flags, lastlogin);
    queue_add(buf);
  }
  return entry;
}

int chanserv_dbase_delete(chanserv_dbase_channel *chan)
{
  dbase_channels *ch;
  char buf[BUFFER_SIZE];
  int i;

  /* write to mysql */
  snprintf(buf, BUFFER_SIZE, "DELETE FROM comment WHERE subject='%s'", queue_escape_string(chan->name));
  queue_add(buf);
  snprintf(buf, BUFFER_SIZE, "DELETE FROM access WHERE channel='%s'", queue_escape_string(chan->name));
  queue_add(buf);
  snprintf(buf, BUFFER_SIZE, "DELETE FROM chandata WHERE name='%s'", queue_escape_string(chan->name));
  queue_add(buf);

  /* make chanserv part the channel */
  chanserv_dbase_part(chan);
  
  /* remove from then channels database */
  if ((ch = channels_getinfo(-1, chan->name))) ch->chanserv = NULL;
     
  /* remove all user from the access-list */
  i = chan->access_count;
  while(chan->access_count > 0)
  {
    chanserv_dbase_access_delete(chan, chan->access[0]->nick, COMMIT_TO_MYSQL);
    /* prevent infinite loop if broken database */
    if (--i < 0) break;
  }

  /* find index in chanserv array */
  i = chanserv_dbase_internal_search(0, chanserv_list_count -1, chan->name);
  
  /* remove from the list of regged channels */
  if (i < chanserv_list_count-1)
    memmove(&chanserv_list[i], &chanserv_list[i+1], (chanserv_list_count-i-1)*sizeof(chanserv_dbase_channel*));
  chanserv_list = (chanserv_dbase_channel**)realloc(chanserv_list, sizeof(chanserv_dbase_channel*)*(--chanserv_list_count));
  
  /* free up resources */
  for (i = 0; i < chan->comment_count; i++)
  {
    xfree(chan->comments[i]->nick);
    xfree(chan->comments[i]->comment);
    xfree(chan->comments[i]);
  }
  xfree(chan->comments);  
  xfree(chan->access);
  xfree(chan->name);
  xfree(chan->owner);
  xfree(chan->topic);
  xfree(chan->keepmode);
  xfree(chan);
  
  return 0;
}


int chanserv_dbase_internal_search(long low, long high, const char *chan)
{
  int res;
  long mid = high - ((high - low) / 2);
  if (low > high) return -1-low;
  res = strcasecmp(chan, chanserv_list[mid]->name);
  if (res < 0) return chanserv_dbase_internal_search(low, mid-1, chan);
  else if (res > 0) return chanserv_dbase_internal_search(mid+1, high, chan);
  else return mid;
}

void chanserv_dbase_burst_join(sock_info *sock)
{
  int i;
  long now = time(0);
  dbase_nicks *CS = (dbase_nicks *)malloc(sizeof(dbase_nicks));
  /* Create an bogus entry in the nicks database for ChanServ.
     Make sure the nick and numeric specified is invalid, 
     ex contains spaces so it cannot be found or will return
     an error from the server if found, so users cannot make
     E kick or change mode on itself */     
  CS->nick = (char *)malloc(12);                    strcpy(CS->nick, "This is me");
  CS->username = (char *)malloc(12);                strcpy(CS->username, "chanserv");
  CS->host = (char *)malloc(strlen(conf->host)+1);  strcpy(CS->host, conf->host);
  CS->userinfo = (char *)malloc(12);                strcpy(CS->userinfo, "ChanServ");
  CS->away = NULL;
  CS->numeric = (char *)malloc(12);                 strcpy(CS->numeric, conf->numeric); strcat(CS->numeric, "XxX");
  CS->IP = 0;
  CS->timestamp = now;
  CS->modes = 0;
  CS->hopcount = 0;
  CS->channels = NULL;
  CS->channels_count = 0;
  CS->nickserv = 0;
  nicks_add(CS, "");
  
  for (i = 0; i < chanserv_list_count; i++)
  {
    if (chanserv_list[i]->flags & BITS_CHANSERV_DISABLED) continue;
      
    chanserv_dbase_join(chanserv_list[i], now, "+nt");    
  }
  /* free the CS variable */
  xfree(CS->nick);
  xfree(CS->username);
  xfree(CS->host);
  xfree(CS->userinfo);
  xfree(CS->numeric);
  xfree(CS);
}

void chanserv_dbase_cleanup(void)
{
  int i, j;
  nickserv_dbase_cleanup();
  debug_out(" | |==> Cleaning ChanServ database...\n");
  for (i = 0; i < chanserv_list_count; i++)
  {
    for (j = 0; j < chanserv_list[i]->comment_count; j++)
    {
      xfree(chanserv_list[i]->comments[j]->nick);
      xfree(chanserv_list[i]->comments[j]->comment);
      xfree(chanserv_list[i]->comments[j]);    
    }
    xfree(chanserv_list[i]->comments);
    xfree(chanserv_list[i]->name);
    xfree(chanserv_list[i]->owner);
    xfree(chanserv_list[i]->topic);
    xfree(chanserv_list[i]->keepmode);
    xfree(chanserv_list[i]->access);
    xfree(chanserv_list[i]);
  }
  xfree(chanserv_list);
  chanserv_list_count = 0;
}

void chanserv_dbase_expire(chanserv_dbase_channel *ch)
{
  char buf[BUFFER_SIZE];
  nickserv_dbase_data *nick;
  ch->flags |= BITS_CHANSERV_EXPIRED | BITS_CHANSERV_DISABLED;
  chanserv_dbase_part(ch);
  
  if ((nick = nickserv_dbase_find_nick(ch->owner)))
    nickserv_dbase_notice(nick, "%s has expired do to inactivity. Contact an oper for more information, or to request it reopened.", ch->name);

  snprintf(buf, BUFFER_SIZE, "UPDATE chandata SET flags='%lu' WHERE name='%s'", ch->flags, queue_escape_string(ch->name));
  queue_add(buf);  
}

int chanserv_dbase_update_lastlogin(chanserv_dbase_channel *ch)
{
  char buf[BUFFER_SIZE];
  ch->lastlogin = (unsigned long)time(0);
  snprintf(buf, BUFFER_SIZE, "UPDATE chandata SET lastlogin='%lu' WHERE name='%s'", ch->lastlogin, queue_escape_string(ch->name));
  queue_add(buf);
  return 0;      
}

void chanserv_dbase_check_expire(void *ptr)
{
  int i;
  timer_event *te = (timer_event*)malloc(sizeof(timer_event));
  unsigned long expire = (unsigned long)time(0) - CHANSERV_EXPIRE_TIME;
  for (i = 0; i < chanserv_list_count; i++)
  {
    if (!(chanserv_list[i]->flags & (BITS_CHANSERV_NOEXPIRE | BITS_CHANSERV_EXPIRED | BITS_CHANSERV_DISABLED)))
      if (chanserv_list[i]->lastlogin < expire)
        chanserv_dbase_expire(chanserv_list[i]);
  }
  te->data = NULL;
  te->func = chanserv_dbase_check_expire;
  te->free = NULL;
  timer_add(te, CHANSERV_EXPIRE_CHECK);
}

/************************************************************************/

int chanserv_dbase_comment_add(chanserv_dbase_channel *chan, nickserv_dbase_data *nick, const char *comment, long date, int sql)
{
  chan->comments = (dbase_comment**)realloc(chan->comments, (++chan->comment_count)*sizeof(dbase_comment*));
  chan->comments[chan->comment_count-1] = (dbase_comment*)malloc(sizeof(dbase_comment));
  
  chan->comments[chan->comment_count-1]->nick = (char*)malloc(strlen(nick->nick)+1);
  strcpy(chan->comments[chan->comment_count-1]->nick, nick->nick);
  
  chan->comments[chan->comment_count-1]->comment = (char*)malloc(strlen(comment)+1);
  strcpy(chan->comments[chan->comment_count-1]->comment, comment);
  
  chan->comments[chan->comment_count-1]->date = date;
  
  if (sql)
  {
    char buf[BUFFER_SIZE], bnick[BUFFER_SIZE], bchan[BUFFER_SIZE];
    
    strcpy(bchan, queue_escape_string(chan->name));
    strcpy(bnick, queue_escape_string(nick->nick));
    snprintf(buf, BUFFER_SIZE, "INSERT INTO comment (subject, nick, comment, com_date) VALUES ('%s','%s','%s',%lu)", bchan, bnick, queue_escape_string(comment), date);
    queue_add(buf);
  }
  
  return 0;
}

/************************************************************************/

int chanserv_dbase_comment_del(chanserv_dbase_channel *chan, unsigned long nr, int sql)
{
  dbase_comment *comment;
  if (nr >= chan->comment_count) return 1;
  
  comment = chan->comments[nr];
    
  memmove(&chan->comments[nr], &chan->comments[nr+1], (chan->comment_count - nr - 1)*(sizeof(dbase_comment*)));
  chan->comments = (dbase_comment**)realloc(chan->comments, (--chan->comment_count)*sizeof(dbase_comment*));
  
  if (sql)
  {
    char buf[BUFFER_SIZE];
    snprintf(buf, BUFFER_SIZE, "DELETE FROM comment WHERE com_date='%lu' AND nick='%s'", comment->date, queue_escape_string(comment->nick));
    queue_add(buf);
  }
  
  xfree(comment->nick);
  xfree(comment->comment);
  xfree(comment);

  return 0;
}

void chanserv_dbase_remove_enforce_ban(chanserv_dbase_channel *ch, int nr, int nocancel, int sql)
{
  if (nr >= ch->bancount) return;
    
  if (sql)
  {
    char buf[BUFFER_SIZE], mbuf[BUFFER_SIZE];
    strcpy(mbuf, queue_escape_string(ch->bans[nr]->mask));
    snprintf(buf, BUFFER_SIZE, "DELETE FROM bans WHERE chan='%s' AND mask='%s'", queue_escape_string(ch->name), mbuf);
    queue_add(buf);
  }

  xfree(ch->bans[nr]->mask);
  xfree(ch->bans[nr]->nick);
  if (!nocancel) timer_cancel(ch->bans[nr]->expire);
  xfree(ch->bans[nr]);
  
  ch->bans[nr] = ch->bans[ch->bancount-1];
  ch->bans = (chanserv_dbase_bans**)realloc(ch->bans, sizeof(chanserv_dbase_bans*)*(--ch->bancount));
}

void chanserv_dbase_remove_covered(chanserv_dbase_channel *ch, const char *banmask)
{
  int i;
  for (i = 0; i < ch->bancount; i++)
  {
    if (wildcard_compare(ch->bans[i]->mask, banmask))
    {
      chanserv_dbase_remove_enforce_ban(ch, i, 0, COMMIT_TO_MYSQL);
      i--;
    }
  }
}

void chanserv_dbase_remove_ban(void *ptr)
{
  char *p = (char*)ptr;
  char *mask, *chan;
  chanserv_dbase_channel *ch;
  
  chan = p;
  mask = strchr(chan, ' ');
  *mask++ = '\0';
  
  if ((ch = chanserv_dbase_find_chan(chan)))
  {
    int i;
    for (i=0; i < ch->bancount; i++)
    {
      if (!strcmp(ch->bans[i]->mask, mask))
      {
        chanserv_dbase_remove_enforce_ban(ch, i, 1, COMMIT_TO_MYSQL);
        break;
      }
    }
  }
  com_send(irc, "%s M %s -b %s\n", conf->cs->numeric, chan, mask);  
  channels_remban(-1, chan, mask);
}

int chanserv_dbase_add_enforce_ban(chanserv_dbase_channel *ch, const char *banmask, int period, const char *nick, int sql)
{
  unsigned long id;
  chanserv_dbase_bans *ban;
  timer_event *te = (timer_event*)malloc(sizeof(timer_event));
    
  te->data = malloc(strlen(banmask)+strlen(ch->name)+2);
  strcpy((char*)te->data, ch->name);
  strcat((char*)te->data, " ");
  strcat((char*)te->data, banmask);

  te->func = chanserv_dbase_remove_ban;
  te->free = xfree;
  
  id = timer_add(te, period);
  
  ban = (chanserv_dbase_bans*)malloc(sizeof(chanserv_dbase_bans));

  ban->mask = (char*)malloc(strlen(banmask)+1);
  strcpy(ban->mask, banmask);
  
  ban->nick = (char*)malloc(strlen(nick)+1);
  strcpy(ban->nick, nick);
  
  ban->expire = id;  
  chanserv_dbase_remove_covered(ch, banmask);
  
  ch->bans = (chanserv_dbase_bans**)realloc(ch->bans, sizeof(chanserv_dbase_bans)*(++ch->bancount));
  ch->bans[ch->bancount-1] = ban;
  
  if (sql)
  {
    char buf[BUFFER_SIZE], cbuf[BUFFER_SIZE], bbuf[BUFFER_SIZE];
    strcpy(cbuf, queue_escape_string(ch->name));
    strcpy(bbuf, queue_escape_string(banmask));
    snprintf(buf, BUFFER_SIZE, "INSERT INTO bans (chan,mask,expire,nick) VALUES ('%s','%s','%lu','%s')", cbuf, bbuf, id, queue_escape_string(nick));
    queue_add(buf);
  }
  return id;
}

int chanserv_dbase_check_enforced_ban(const char *chan, const char *mask)
{
  chanserv_dbase_channel *ch;
  if ((!chan) || (!mask)) return 0;
    
  if ((ch = chanserv_dbase_find_chan(chan)))
  {
    int i;
    for (i=0; i < ch->bancount; i++)
    {
      if (!strcmp(ch->bans[i]->mask, mask))
      {
        com_send(irc, "%s M %s +b %s\n", conf->cs->numeric, chan, mask);
        return 1;
      }
    }
  }
  return 0;
}

int chanserv_dbase_check_enforced_ban_kick(const char *chan, const char *mask, const char *numeric)
{
  chanserv_dbase_channel *ch;
  if ((!chan) || (!mask)) return 0;
    
  if ((ch = chanserv_dbase_find_chan(chan)))
  {
    int i;
    for (i=0; i < ch->bancount; i++)
    {
      if (wildcard_compare(ch->bans[i]->mask, mask))
      {
        com_send(irc, "%s M %s +b %s\n", conf->cs->numeric, chan, ch->bans[i]->mask);
        com_send(irc, "%s K %s %s :%s is banned!\n", conf->cs->numeric, chan, numeric, ch->bans[i]->mask);
        channels_addban(-1, chan, ch->bans[i]->mask);
        channels_userpart(-1, chan, numeric);
        return 1;
      }
    }
  }
  return 0;
}

void chanserv_dbase_join(chanserv_dbase_channel *chan, unsigned long burst, char *mode)
{
  dbase_channels *info;
  unsigned long t = 0;
  char num[8];
  
  strcpy(num, conf->numeric);
  strcat(num, "XxX");
  
  /* Find the channel in the database */
  if (!(info = channels_getinfo(-1, chan->name)))
  {
    /* add it if it doesn't exist */
    long nr = channels_add(chan->name, burst);
    info = channels_getinfo(nr, NULL);
  }
  if (info)
  {
    /* Link the chanserv database with the channels database */
    info->chanserv = chan;
    t = info->createtime;
    /* Have chanserv join the channel, with a bogus entry in channels database */
    channels_userjoin(-1, chan->name, "+o", num);
  }
  if (burst)
  {
    /* Send the actual burst to the server, to make ChanServ join the channel on the network */
    if (t == burst)
      com_send(irc, "%s B %s %lu %s %s:o\n", conf->numeric, chan->name, t, mode, conf->cs->numeric);
    else
      com_send(irc, "%s B %s %lu %s:o\n", conf->numeric, chan->name, t, conf->cs->numeric);
  }
  else
  {
    com_send(irc, "%s J %s\n", conf->cs->numeric, chan->name);
    com_send(irc, "%s M %s +o %s\n", conf->numeric, chan->name, conf->cs->numeric); 
  }
}

void chanserv_dbase_part(chanserv_dbase_channel *chan)
{
  dbase_channels *info;
  char num[8];
  
  strcpy(num, conf->numeric);
  strcat(num, "XxX");

  /* don't send part, if not on the channel */
  if (channels_user_find(chan->name, num) < 0) return;

  if ((info = channels_getinfo(-1, chan->name)))
  {
    info->chanserv = NULL;
  }
  channels_userpart(-1, chan->name, num);
  com_send(irc, "%s L %s\n", conf->cs->numeric, chan->name);
}

/**************************************************************************************************
 * chanserv_dbase_resync_channel
 **************************************************************************************************
 *   Loops through the entire userlist for the specified channel, creating P10 MODE commands
 *   to reop/revoice all user who currently have op/voice.
 *   This is used when a new server connects to the network and has a channel with a ealier
 *   timestamp than the rest of the network, thus overruling the channel settings and modes.
 *   the P10 commands are placed in a linked list (string_chain) and added as a timer-event
 *   to trigger immediately after recieving END_OF_BURST
 **************************************************************************************************
 * Params:
 *   [IN] dbase_channels *chan  : The channel to check
 **************************************************************************************************/
void chanserv_dbase_resync_channel(dbase_channels *chan)
{
  int i, m = 0, total = 0;
  char modes[10], nums[40]; /* can only contain 6 numeric/modes, 10 > 6, 40 > 6*6 (numeric + space) */
  string_chain *root, *chain;
  
  if (!chan) return;

  /* reset the 2 buffers */
  modes[0] = 0;
  nums[0] = 0;

  /* create the root of the string chain */
  root = (string_chain*)malloc(sizeof(string_chain));
  root->next = NULL;
  root->str = (char*)malloc(BUFFER_SIZE);
  
  chain = root;
  
  /* is this a regged active channel */
  if (!chan->chanserv)
  {
    /* ChanServ is not usual in this channel, join the channel to try to
       eliminate as many HACK(4) as possible */
    snprintf(chain->str, BUFFER_SIZE, "%s J %s\n", conf->cs->numeric, chan->name);
    
    /* attach a new field to the chain */
    chain->next = (string_chain*)malloc(sizeof(string_chain));
    chain = chain->next;
    chain->str = (char*)malloc(BUFFER_SIZE);
    chain->next = NULL;
  }
  
  /* Give ChanServ op again */
  snprintf(chain->str, BUFFER_SIZE, "%s M %s +o %s\n", conf->numeric, chan->name, conf->cs->numeric);
  /* add a new field to the chain */
  chain->next = (string_chain*)malloc(sizeof(string_chain));
  chain = chain->next;
  chain->str = (char*)malloc(BUFFER_SIZE);
  chain->next = NULL;

  /* loop through all users on the channel */
  for (i = 0; i < chan->usercount; i++)
  {
    /* Is this user +v or +o, and is from anther server than services */
    if ((chan->users[i]->mode & 3) && (strncmp(chan->users[i]->nick->numeric, conf->numeric, 2)))
    {
      /* Only allow 5-6 modes per command */
      if (m >= 5)
      {
        snprintf(chain->str, BUFFER_SIZE, "%s M %s +%s%s\n", conf->cs->numeric, chan->name, modes, nums);
        chain->next = (string_chain*)malloc(sizeof(string_chain));
        chain = chain->next;
        chain->str = (char*)malloc(BUFFER_SIZE);
        chain->next = NULL;
        modes[0] = 0;
        nums[0] = 0;
      }
      /* does the user have voice? */
      if (chan->users[i]->mode & 1)
      {
        strcat(modes, "v");
        strcat(nums, " ");
        strcat(nums, chan->users[i]->nick->numeric);
        m++;
        total++;
      }
      /* does the user have op */
      if (chan->users[i]->mode & 2)
      {
        strcat(modes, "o");
        strcat(nums, " ");
        strcat(nums, chan->users[i]->nick->numeric);
        m++;
        total++;
      }
    }
  }
  
  snprintf(chain->str, BUFFER_SIZE, "%s M %s +%s%s\n", conf->cs->numeric, chan->name, modes, nums);

  /* not a regged channel, make chanserv leave afterwards */
  if (!chan->chanserv)
  {
    chain->next = (string_chain*)malloc(sizeof(string_chain));
    chain = chain->next;
    chain->str = (char*)malloc(BUFFER_SIZE);
    snprintf(chain->str, BUFFER_SIZE, "%s P %s\n", conf->cs->numeric, chan->name);
    chain->next = NULL;
  }
  
  /* are there any users to change mode on */
  if (total > 0)
  {
    timer_event *t;
    /* yes there are, create a timer to execute right after EB */
    t = (timer_event*)malloc(sizeof(timer_event));
    t->data = (void*)root;
    t->func = string_chain_traverse;
    t->free = string_chain_free;
    timer_add(t, 0);
  }
  else /* if not, just forget the whole thing */
    string_chain_free(root);
}

