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
/* $Id: p10.c,v 1.8 2003/02/25 23:49:47 mr Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "setup.h"
#include "p10.h"
#include "misc_func.h"
#include "dbase.h"
#include "config.h"
#include "server.h"
#include "queue.h"
#include "log.h"
#include "parser.h"
#include "chanserv.h"
#include "nickserv.h"
#include "db_server.h"
#include "dcc.h"

extern char *build_date;
extern sock_info *irc;
extern int com_burst;

int connected = 0;

/*
 **************************************************************************************************
 * p10_server
 **************************************************************************************************
 *   Parser for the P10 command: S
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_server(sock_info *sock, char *from, char **params)
{
  char *name, *numeric, *desc;
  unsigned long linktime;
  name = getnext(params);
  getnext(params); /* hop count */
  getnext(params); /* start time */
  linktime = tr_atoi(getnext(params));
  getnext(params); /* protocol */
  numeric = getnext(params);
  /* getnext(params);  .11 server settings (ex: +hs) - not in .10 */
  desc = getrest(params);
  
  com_burst = 1;

  if (!from) conf->starttime -= (conf->starttime - linktime);
    
  if (from)
  {
    dbase_server *server = server_search(from);
    if (server)
      dcc_console_text('b', "[%s] server introduced by %s", name, server->name);
  }
  
  server_add(from, numeric, name, linktime, desc);
  
  return 0;
}

/*
 **************************************************************************************************
 * p10_pass
 **************************************************************************************************
 *   Parser for the P10 command: PASS
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_pass(sock_info *sock, char *from, char **params)
{
  /* TODO: Ought to check the password... but for now we don't */
  return 0;
}

/*
 **************************************************************************************************
 * p10_error
 **************************************************************************************************
 *   Parser for the P10 command: ERROR
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_error(sock_info *sock, char *from, char **params)
{
  /* Woops, error !! TODO - Something must be done!! SQUIT ?? */
  dcc_console_text('b', "[ERROR] %s", *params);
  log_command(LOG_SERVICES, NULL, "", "Got ERROR from server: %s", *params);
  return 0;
}

/*
 **************************************************************************************************
 * p10_end_of_burst
 **************************************************************************************************
 *   Parser for the P10 command: EB (end of burst)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_end_of_burst(sock_info *sock, char *from, char **params)
{
  dbase_server *server;
  if (!connected)
  {
    com_send(irc, "%s N %s 1 %ld %s %s +ikd xXxXxX %s :%s\n", conf->numeric, conf->ns->nick, time(0), conf->ns->username, conf->host, conf->ns->numeric, conf->ns->userinfo);
    com_send(irc, "%s N %s 1 %ld %s %s +ikd xXxXxX %s :%s\n", conf->numeric, conf->cs->nick, time(0), conf->cs->username, conf->host, conf->cs->numeric, conf->cs->userinfo);
    com_send(irc, "%s N %s 1 %ld %s %s +ikd xXxXxX %s :%s\n", conf->numeric, conf->os->nick, time(0), conf->os->username, conf->host, conf->os->numeric, conf->os->userinfo);
    com_send(irc, "%s N %s 1 %ld %s %s +ikd xXxXxX %s :%s\n", conf->numeric, conf->ms->nick, time(0), conf->ms->username, conf->host, conf->ms->numeric, conf->ms->userinfo);
    nickserv_dbase_init_jupes(sock);
    chanserv_dbase_burst_join(sock);
    com_send(irc, "%s EB\n", conf->numeric); /* END_OF_BURST */
    com_send(irc, "%s EA\n", conf->numeric);
    connected = 1;
  }
  server = server_search(from);
  if (server)
    dcc_console_text('b', "[%s] end of burst", server->name);
  com_burst = 0;
  return 0;
}

/*
 **************************************************************************************************
 * p10_end_of_burst_acknowledge
 **************************************************************************************************
 *   Parser for the P10 command: EA (end of burst acknowledge)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_end_of_burst_acknowledge(sock_info *sock, char *from, char **params)
{
  return 0;
}

/*
 **************************************************************************************************
 * p10_ping
 **************************************************************************************************
 *   Parser for the P10 command: G (ping)
 *    [NUM] G <sender> :[server]
 *
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_ping(sock_info *sock, char *from, char **params)
{
  char *user = getnext(params);
  char *rest = getrest(params);
  
  if (!rest)
  {
    dcc_console_text('d', "[%s] PING", user);
    return com_send(irc, "%s Z :%s\n", conf->numeric, user);
  }
  else
  {
    dcc_console_text('d', "[%s] PING %s", user, rest);
    return com_send(irc, "%s Z %s :%s\n", conf->numeric, rest, user);
  }
}

/*
 **************************************************************************************************
 * p10_squit
 **************************************************************************************************
 *   Parser for the P10 command: SQUIT
 *   [NUM] SQ [server name] [timestamp] :[reason]
 *   ABACV SQ bios.kc-consult.dk 1013131517 :fuck off ;)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_squit(sock_info *sock, char *from, char **params)
{
  dbase_server *srv; 
  char *server = getnext(params);
  char *reason;
  server_remove(server);
  
  getnext(params);
  reason = getrest(params);

  srv = server_search(from);
  if (srv)
    dcc_console_text('b', "[%s] net-break from %s (%s)", server, srv->name, reason);

   return 0;
}

/*
 **************************************************************************************************
 * p10_nick
 **************************************************************************************************
 *   Parser for the P10 command: N (nick)
 *   [NUM SERVER] N [NICK] [HOPCOUNT] [TIMESTAMP] [USERNAME] [HOST] <+modes> [BASE64 IP] [NUMERIC] :[USERINFO]
     AB           N The_Real 3        1013552696   the_real 0xc2ef4c1c.odnxx3.adsl-dhcp.tele.dk +oiwg DC70wc ABACj :The_Real
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_nick(sock_info *sock, char *from, char **params)
{
  dbase_nicks n;
  char *nick = getnext(params);
  char *hopcount = getnext(params);
  char *timestamp = getnext(params);
  if (!timestamp)
  {
    if (!com_burst)
    {
      dbase_nicks *ns = nicks_getinfo(from, NULL, -1);
      nickserv_new_nick_notice(sock, from, nick); /* Check if the nick is registered or not (nickserv.c) */
      dcc_console_text('n', "[%s!%s@%s] changed nick to %s", ns->nick, ns->username, ns->host, nick);
    }
    nicks_renick(from, nick);
  }
  else
  {
    char *modes;
    n.nick = nick;
    n.hopcount = atol(hopcount);
    n.timestamp = atol(timestamp);
    n.username = getnext(params);
    n.host = getnext(params);
    modes = getnext(params);
    if (*modes == '+')
      n.IP = str64long(getnext(params));
    else
    {
      n.IP = str64long(modes);
      modes = NULL;
    }
    n.numeric = getnext(params);
    n.userinfo = getrest(params);
    n.modes = 0;
    n.away = NULL;
    nicks_add(&n, modes);
    if (!com_burst)
    {
      char *p, num[5];
      dbase_server *server;
      com_message(sock, conf->ns->numeric, n.numeric, "O", NICKSERV_NEW_NICK_NOTICE, nick); /* Welcomes the users */
      nickserv_new_nick_notice(sock, n.numeric, n.nick); /* Checks if the nick is already registered or not (nickserv.c) */

      strncpy(num, n.numeric, 2);
      num[2] = '\0';
      server = server_search(num);
      p = num;
      if (server) p = server->name;
      dcc_console_text('n', "[%s!%s@%s] connected to %s", n.nick, n.username, n.host, p);
    }
  }
  return 0;
}

/*
 **************************************************************************************************
 * p10_burst
 **************************************************************************************************
 *   Parser for the P10 command: B (burst)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_burst(sock_info *sock, char *from, char **params)
{
  long chn;
  char *num, mode[4];
  char *chan = getnext(params);
  char *ts = getnext(params);
  unsigned long timestamp = tr_atoi(ts);
  char *next;
  
  com_burst = 1;
  
  if (connected)
  {
    /* another server is connecting to the network */
    dbase_channels *info = channels_getinfo(-1, chan);
    if (info)
    {
      if (info->createtime > timestamp)
      {
        info->createtime = timestamp;
        chanserv_dbase_resync_channel(info);
      }
    }
  }
  
  chn = channels_add(chan, timestamp);
  if (*params)
  {
    if (**params == '+')
    {
      next = getnext(params);
      channels_setmode(chn, chan, next);
      if (strchr(next, 'k')) channels_addkey(chn, chan, getnext(params));
      if (strchr(next, 'l')) channels_addlimit(chn, chan, atol(getnext(params)));
    }
  }
  if (*params)
  {
    if ((*params)[1] == '%')
    {
      next = getrest(params);
      channels_addban(chn, chan, ++next);
      return 0;
    }
  }
  num = getnext(params);
  mode[0] = '\0';
  while (num)
  {
    char *modes, *bak;
    bak = num;
    if ((num = strchr(num, ',')))
    {
      *num = '\0';
      num++;
    }
    if ((modes = strchr(bak, ':')))
    {
      *modes = '\0';
      modes++;                  
      mode[0] = '+'; mode[2] = '\0'; mode[2] = '\0'; mode[3] = '\0';        
      if (modes[0] != '\0') 
      {
        mode[1] = modes[0];
        if (modes[1] != '\0') {mode[2] = modes[0]; }
      }
    }    
    channels_userjoin(chn, chan, mode, bak);
  }
  if (*params)
  {
    if ((*params)[1] == '%')
    {
      next = getrest(params);
      channels_addban(chn, chan, ++next);
    }
  }
  return 0;
}
 
/*
 **************************************************************************************************
 * p10_join
 **************************************************************************************************
 *   Parser for the P10 command: J (join)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_join(sock_info *sock, char *from, char **params)
{
  dbase_nicks *ns = nicks_getinfo(from, NULL, -1);
  char *p, *c = getnext(params);
  char mask[BUFFER_SIZE]; /* Allocate to size only? dunno */ 

  if (!ns) return 0;
  
  p = c;  
  while (p)
  {
    p = strchr(c, ',');
    if (p) *p++ = '\0';

    if (!strcmp(c, "0")) 
    {
      while (ns->channels_count > 0)
        channels_userpart(-1, ns->channels[0]->channel->name, ns->numeric);
    
      dcc_console_text('j', "[%s!%s@%s] parted all channels", ns->nick, ns->username, ns->host);
    }
    else
    {
      channels_userjoin(-1, c, "", from);
      
      dcc_console_text('j', "[%s!%s@%s] joined %s", ns->nick, ns->username, ns->host, c);
    
      snprintf(mask, BUFFER_SIZE, "*!*%s@%s", ns->username, ns->host); /* optimize? */
      chanserv_dbase_check_enforced_ban_kick(c, mask, ns->numeric);
    }
    c = p;
  }
  
  return 0;
}

/*
 **************************************************************************************************
 * p10_create
 **************************************************************************************************
 *   Parser for the P10 command: C (create)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_create(sock_info *sock, char *from, char **params)
{
  dbase_nicks *ns = nicks_getinfo(from, NULL, -1);
  char *p;
  char *c = getnext(params);
  char *ts = getnext(params);
  
  while ((p = strchr(c, ',')))
  {
    *p++ = '\0';
    if (*c == '#') channels_userjoin(channels_add(c, atol(ts)), c, "+o", from);
    else channels_userjoin(channels_add(c, atol(ts)), c, "", from);
    if (ns)
      dcc_console_text('j', "[%s!%s@%s] created %s", ns->nick, ns->username, ns->host, c);
    c = p;
  }
  if (*c == '#') channels_userjoin(channels_add(c, atol(ts)), c, "+o", from);
  else channels_userjoin(channels_add(c, atol(ts)), c, "", from);
  if (ns)
    dcc_console_text('j', "[%s!%s@%s] created %s", ns->nick, ns->username, ns->host, c);
  return 0;
}

/*
 **************************************************************************************************
 * p10_part
 **************************************************************************************************
 *   Parser for the P10 command: L (part)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_part(sock_info *sock, char *from, char **params)
{
  dbase_nicks *ns = nicks_getinfo(from, NULL, -1);
  char *p;
  char *c = getnext(params);
  while ((p = strchr(c, ',')))
  {
    p[0] = '\0';
    p++;
    channels_userpart(-1, c, from);
    if (ns)
      dcc_console_text('j', "[%s!%s@%s] left %s", ns->nick, ns->username, ns->host, c);
    c = p;
  }
  channels_userpart(-1, c, from);
    if (ns)
      dcc_console_text('j', "[%s!%s@%s] left %s", ns->nick, ns->username, ns->host, c);
  return 0;
}

/*
 **************************************************************************************************
 * p10_quit
 **************************************************************************************************
 *   Parser for the P10 command: Q (quit)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_quit(sock_info *sock, char *from, char **params)
{
  char *reason = getrest(params);
  dbase_nicks *ns = nicks_getinfo(from, NULL, -1);
  if (ns)
    dcc_console_text('n', "[%s!%s@%s] quit (%s)", ns->nick, ns->username, ns->host, reason);
  nicks_remove(from);
  return 0;
}

/*
 **************************************************************************************************
 * p10_notice
 **************************************************************************************************
 *   Parser for the P10 command: O (notice)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_notice(sock_info *sock, char *from, char **params)
{
  char *to = getnext(params);
  return parser_commands(sock, to, params, from, MODE_NOTICE);
}

/*
 **************************************************************************************************
 * p10_invite
 **************************************************************************************************
 *   Parser for the P10 command: I (invite)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_invite(sock_info *sock, char *from, char **params)
{
  char *nick = getrest(params);
  char *chan = getrest(params);
  dbase_nicks *ns = nicks_getinfo(from, NULL, -1);
  if (ns)
    dcc_console_text('d', "[%s!%s@%s] has invited %s to %s", ns->nick, ns->username, ns->host, nick, chan);
  return 0;
/*  
  char *chan;
  getnext(params);*/ /* = ??? */
/*
  chan = getnext(params);
  return com_message(sock, from, "You have invited me to join %s. Please contact IRCops for help instead :-)", chan);
*/
}

/*
 **************************************************************************************************
 * p10_mode
 **************************************************************************************************
 *   Parser for the P10 command: M (mode)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_mode(sock_info *sock, char *from, char **params)
{
  dbase_nicks *ns = nicks_getinfo(from, NULL, -1);
  dbase_server *server = server_search(from);
  char minus = '+';
  const char *num, *tmp2;
  char *who = getnext(params);
  char *modestr = getnext(params);
  char *tmp;
  char mode[3];

  if (*who == '#')
  {
    char dcc_param[500], dcc_who[200], *dcc_mode = modestr;

    dcc_param[0] = '\0';
    if (ns)
      snprintf(dcc_who, 200, "%s!%s@%s", ns->nick, ns->username, ns->host);
    else if (server)
      strcpy(dcc_who, server->name);

    while (*modestr)
    {
      switch (*modestr)
      {
        case '+':
        case '-':
           minus = *modestr;
           break;
        case 'b':
          tmp = getnext(params);

          if (minus == '-') 
            channels_remban(-1, who, tmp);
          else 
            channels_addban(-1, who, tmp);

          strcat(dcc_param, " ");
          strcat(dcc_param, tmp);

          break;
        case 'k':
          tmp = getnext(params);

          if (minus == '-') 
            channels_remkey(-1, who, tmp);
          else 
            channels_addkey(-1, who, tmp);

          strcat(dcc_param, " ");
          strcat(dcc_param, tmp);

          break;
        case 'l':
          if (minus == '-') 
            channels_remlimit(-1, who);
          else
          {
            tmp = getnext(params);
            channels_addlimit(-1, who, tr_atoi(tmp));

            strcat(dcc_param, " ");
            strcat(dcc_param, tmp);
          }
          break;
        case 'i':
        case 'm':
        case 'n':
        case 'p':
        case 's':
        case 't': 
          mode[0] = minus;
          mode[1] = *modestr;
          mode[2] = '\0';
          channels_setmode(-1, who, mode);
          break;
        case 'o':
        case 'v':
          mode[0] = minus;
          mode[1] = *modestr;
          mode[2] = '\0';
  
          tmp = getnext(params);
          channels_usermode(-1, who, mode, tmp);
          
          strcat(dcc_param, " ");
          if ((tmp2 = nicks_getnick(tmp)))
            strcat(dcc_param, tmp2);
          else
            strcat(dcc_param, tmp);
          break;
      default:
         /* Unknown mode !! */
         log_command(LOG_SERVICES, NULL, "", "Setting unknown channel-mode: %c", *modestr);
        break;
      }
      modestr++;
    }
    dcc_console_text('m', "[%s] sets mode %s %s%s", dcc_who, who, dcc_mode, dcc_param);
    
  }
  else
  { /* User-mode */
    char *p = modestr;
    int add = 1;
    if (ns)
      dcc_console_text('u', "[%s!%s@%s] sets mode %s %s", ns->nick, ns->username, ns->host, who, modestr);
    else if (server)
      dcc_console_text('u', "[%s] sets mode %s %s", server->name, who, modestr);
    num = nicks_getnum(who);
    nicks_setmode(num, modestr);
    while (*p)
    {
      if (*p == '+') add = 1;
      else if (*p == '-') add = 0;
      else if (*p == 'o') 
      {
        char n[5];
        strncpy(n, num, 2);
        n[2] = '\0';
        
        server = server_search(n);
        ns = nicks_getinfo(from, NULL, -1);
        
        if ((add) && (server) && (ns))
          dcc_console_text('o', "[%s!%s@%s] opered up on %s", ns->nick, ns->username, ns->host, server->name);
        break;
      }
      p++;
    }
  }
  return 0;
}

/*
 **************************************************************************************************
 * p10_away
 **************************************************************************************************
 *   Parser for the P10 command: A (away)
 *   [NUM Sender] A :[away msg]
 *   [NUM Sender] A
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_away(sock_info *sock, char *from, char **params)
{  
  nicks_setaway(from, getrest(params));
  return 0;
}

/*
 **************************************************************************************************
 * p10_topic
 **************************************************************************************************
 *   Parser for the P10 command: T (topic)
 *   [NUM Sender] T [channel] :[new topic]
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_topic(sock_info *sock, char *from, char **params)
{  
  dbase_nicks *ns = nicks_getinfo(from, NULL, -1);
  char *chan = getnext(params);
  char *topic = getrest(params);
  channels_settopic(-1, chan, topic);

  if (ns)
    dcc_console_text('t', "[%s!%s@%s] sets topic on %s: %s", ns->nick, ns->username, ns->host, chan, topic);
  
  return 0;
}

/*
 **************************************************************************************************
 * p10_kick
 **************************************************************************************************
 *   Parser for the P10 command: K (kick)
 *   [NUM Sender] K [channel] [NUM victim] :[reason]
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_kick(sock_info *sock, char *from, char **params)
{
  char *chan = getnext(params);
  char *num = getnext(params);
  char *reason = getrest(params);
  dbase_nicks *ns = nicks_getinfo(from, NULL, -1);
  dbase_nicks *ns2 = nicks_getinfo(num, NULL, -1);
  channels_userpart(-1, chan, num);
  
  if ((ns) && (ns2))
    dcc_console_text('j', "[%s!%s@%s] has kicked %s!%s@%s from %s (%s)", ns->nick, ns->username, ns->host, ns2->nick, ns2->username, ns2->host, chan, reason);
  
  return 0;
}

/*
 **************************************************************************************************
 * p10_privmsg
 **************************************************************************************************
 *   Parser for the P10 command: P (privmsg)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_privmsg(sock_info *sock, char *from, char **params)
{
  char *to = getnext(params);
  return parser_commands(sock, to, params, from, MODE_PRIVMSG);
}

/*
 **************************************************************************************************
 * p10_kill
 **************************************************************************************************
 *   Parser for the P10 command: D (kill)
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_kill(sock_info *sock, char *from, char **params)
{
  nicks_remove(getnext(params));
  return 0;
}

/*
 **************************************************************************************************
 * p10_wallops
 **************************************************************************************************
 *   Parser for the P10 command: WA (wallops)
 *   [NUM Sender] WA :[wallops message]
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_wallops(sock_info *sock, char *from, char **params)
{
  char *msg = getrest(params);
  char frombuf[BUFFER_SIZE], buf[2*BUFFER_SIZE];
  dbase_nicks *nick;
  dbase_server *serv;
  
  if ((nick = nicks_getinfo(from, NULL, -1)))
    snprintf(frombuf, BUFFER_SIZE, "%s!%s@%s", nick->nick, nick->username, nick->host);
  else if ((serv = server_search(from)))
    strcpy(frombuf, serv->name);
  else
  {
    if (!strcmp(from, conf->os->numeric))
      snprintf(frombuf, BUFFER_SIZE, "%s!%s@%s", conf->os->nick, conf->os->username, conf->host);
    else if (!strcmp(from, conf->cs->numeric))
      snprintf(frombuf, BUFFER_SIZE, "%s!%s@%s", conf->cs->nick, conf->cs->username, conf->host);
    else if (!strcmp(from, conf->ns->numeric))
      snprintf(frombuf, BUFFER_SIZE, "%s!%s@%s", conf->ns->nick, conf->ns->username, conf->host);
    else if (!strcmp(from, conf->ms->numeric))
      snprintf(frombuf, BUFFER_SIZE, "%s!%s@%s", conf->ms->nick, conf->ms->username, conf->host);
    else
      strcpy(frombuf, "[unknown]");
  }
  
  dcc_console_text('c', "[%s] WALLOPS: %s", frombuf, msg);

  strcpy(frombuf, queue_escape_string(frombuf));
  
  snprintf(buf, 2*BUFFER_SIZE, "INSERT INTO log_wallops (wa_date, nick, msg) VALUES ('%lu', '%s', '%s')", (unsigned long)time(0), frombuf, queue_escape_string(msg));
  queue_add(buf);
  
  return 0;
}

/*
 **************************************************************************************************
 * p10_version
 **************************************************************************************************
 *   Parser for the P10 command: V (version)
 *   [NUM Sender] V :[NUM Server]
 **************************************************************************************************
 * Params:
 *   std. for parser functions
 **************************************************************************************************
 */
int p10_version (sock_info *sock, char *from, char **params)
{
  dbase_nicks *nick;
  if (!(nick = nicks_getinfo(from, NULL, -1))) return 0;
  com_send(irc, ":%s 351 %s u2.services %s :%s IRC Services (%s)\n", conf->host, nick->nick, conf->host, NETWORK_NAME, build_date);
  return 0;  
}












