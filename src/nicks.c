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
/* $Id: nicks.c,v 1.3 2003/01/17 18:32:40 mr Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "setup.h"
#include "dbase.h"
#include "server.h"
#include "misc_func.h"
#include "chanserv.h"
#include "log.h"
#include "dcc.h"

dbase_nicks **nicks       = NULL;
dbase_nicks **nicks_num   = NULL;
long          nicks_count = 0;


/********************************************************************
  NICKS_SEARCH_*
     Leder i nicks arrayen efter den afgivne person

  Parameter og return:
    [in]  char *nick   = personens nick
el. [in]  long numeric = personenes numeric
    [out] long  return
                  >= 0 = index i nicks arrayen til den fundne person
                   < 0 = Fejl - ikke fundet
                        (Internt: (return+1)*-1 = index til nærmeste )

********************************************************************/
long nicks_search_nick(const char *nick)
{
  return nicks_internal_search(0, nicks_count-1, nick);
}

long nicks_search_numeric(const char *numeric)
{
  return nicks_internal_num_search(0, nicks_count-1, numeric);
}


/********************************************************************
  NICKS_INTERNAL_SEARCH
     Bruges internt af nicks_search til at køre en bintreesearch
     på nicks arrayen

  Parameter og return:
    [in]  long  low    = \ Der bliver kun ledt efter kanalen i
    [in]  long  high   = / arrayen mellem indexene low og high
    [in]  char *nick   = personens nick
    [out] long  return
                  >= 0 = index i nicks arrayen til den fundne person
                   < 0 = Fejl - ikke fundet
                        (Internt: (return+1)*-1 = index til nærmeste )
   
********************************************************************/
long nicks_internal_search(long low, long high, const char *nick)
{
  int res;
  long mid = high - ((high - low) / 2);
  if (low > high) return -1-low;
  res = strcasecmp(nick, nicks[mid]->nick);
  if (res < 0) return nicks_internal_search(low, mid-1, nick);
  else if (res > 0) return nicks_internal_search(mid+1, high, nick);
  else return mid;
}

/********************************************************************
  NICKS_INTERNAL_NUM_SEARCH
     Bruges internt af nicks_search til at køre en bintreesearch
     på nicks arrayen
        
  Parameter og return:
    [in]  long  low    = \ Der bliver kun ledt efter kanalen i
    [in]  long  high   = / arrayen mellem indexene low og high
    [in]  long numeric = numeric på den person der skal findes
    [out] long  return
                  >= 0 = index i nicks arrayen til den fundne person
                   < 0 = Fejl - ikke fundet
                        (Internt: (return+1)*-1 = index til nærmeste )
   
********************************************************************/
long nicks_internal_num_search(long low, long high, const char *numeric)
{
  int res;
  long mid = high - ((high - low) / 2);
  if (low > high) return -1-low;
  res = strcmp(numeric, nicks_num[mid]->numeric);
  if (res < 0) return nicks_internal_num_search(low, mid-1, numeric);
  else if (res > 0) return nicks_internal_num_search(mid+1, high, numeric);
  else return mid;
}

/********************************************************************
  NICKS_ADD
    Tilføjer en person til nicks arrayen ved at oprette en pointer
    til en ny dbase_nicks, hvortil informationerne fra data kopieres
    (der bliver oprettet nye pointere til pointer-elementer).
    Sørg for at ALLE elementer i data er valid.

    OBS: Hvis der eksisterer en person med samme navn vil denne
         blive OVERSKREVET !!

    nicks_add finder automatisk det sted i arrayen hvor data skal
    tilføjes så arrayen hele tiden er sorteret alfabetisk efter nick
    og numeric.

  Parameter og return:
    [in]  dbase_nicks *data    = struct med de data der skal tilføjen
    [out] long      return
                       >= 0 = OK
                        < 0 = Fejl

********************************************************************/
long nicks_add(dbase_nicks *info, char *modes)
{
  long nr, num;
  if ((nr = nicks_search_nick(info->nick)) < 0)
  {
    nr += 1;
    nr *= -1;
    nicks = (dbase_nicks **)realloc(nicks, (nicks_count+1) * sizeof(dbase_nicks*));
    if (nr < (nicks_count)) memmove(&nicks[nr+1], &nicks[nr], (nicks_count - nr) * sizeof(dbase_nicks*));
    nicks[nr] = (dbase_nicks*)malloc(sizeof(dbase_nicks));
  }
  else
  {
    nicks_remove(nicks[nr]->numeric);
    return nicks_add(info, modes);
  }
  if ((num = nicks_search_numeric(info->numeric)) < 0)
  {
    num += 1;
    num *= -1;
    nicks_num = (dbase_nicks**)realloc(nicks_num, (nicks_count+1) * sizeof(dbase_nicks*));
    if (num < (nicks_count)) memmove(&nicks_num[num+1], &nicks_num[num], (nicks_count - num) * sizeof(dbase_nicks*));
  }
  else
  {
    nicks_remove(nicks_num[num]->numeric);
    return nicks_add(info, modes);
  }

  nicks_count++;

  nicks_num[num] = nicks[nr];

  nicks[nr]->nick = (char *)malloc(sizeof(char)*(strlen(info->nick)+1));
  strcpy(nicks[nr]->nick, info->nick);

  nicks[nr]->numeric = (char *)malloc(sizeof(char)*(strlen(info->numeric)+1));
  strcpy(nicks[nr]->numeric, info->numeric);

  nicks[nr]->username = (char *)malloc(sizeof(char)*(strlen(info->username)+1));
  strcpy(nicks[nr]->username, info->username);

  nicks[nr]->host = (char *)malloc(sizeof(char)*(strlen(info->host)+1));
  strcpy(nicks[nr]->host, info->host);

  nicks[nr]->userinfo = (char *)malloc(sizeof(char)*(strlen(info->userinfo)+1));
  strcpy(nicks[nr]->userinfo, info->userinfo);

  nicks[nr]->away = NULL;

  nicks[nr]->channels = (dbase_channels_nicks **)calloc(0, sizeof(dbase_channels_nicks*));

  nicks[nr]->IP = info->IP;
  nicks[nr]->timestamp = info->timestamp;
  nicks[nr]->hopcount = info->hopcount;
  nicks[nr]->channels_count = 0;
  nicks[nr]->modes = 0;  
  nicks[nr]->nickserv = NULL;

  nicks[nr]->ignored = 0;
  nicks[nr]->ignore_lines = 0;
  nicks[nr]->ignore_ts[0] = 0;
  nicks[nr]->ignore_ts[1] = 0;

  nicks_setmode(nicks[nr]->numeric, modes);

  return num;
}

/********************************************************************
  NICKS_REMOVE
    Fjerner en person fra nicks arrayen og free'er alle resourcer
    som er associeret med den personen
        
  Parameter og return:
    [in]  long  index   = personens numeric
    [in]  long  numeric = personens numeric
    [out] long  return 
                   < 0  = Fejl
                  >= 0  = OK
   
********************************************************************/
long nicks_remove(const char *numeric)
{
  long nr, index, i;
  if ((index = nicks_search_numeric(numeric)) < 0) return index;
  if ((nr = nicks_search_nick(nicks_num[index]->nick)) < 0) return nr;

  for (i = nicks[nr]->channels_count; i > 0; i--)
  {
    if (channels_userpart(-1, nicks[nr]->channels[i-1]->channel->name, nicks[nr]->numeric) < 0)
    {
      debug_out("We're fucked !!!! - %lu %s\n", nicks[nr]->channels_count, nicks[nr]->channels[0]->channel->name);
      log_command(LOG_SERVICES, NULL, "", "BUG! nicks_remove - %lu %s", nicks[nr]->channels_count, nicks[nr]->channels[0]->channel->name);
    }
  }

  if (nicks[nr]->channels_count)
  {
    debug_out("WARNING !!!! Memory leak - not all channels was parted in memory (%lu)!!!!\n", nicks[nr]->channels_count);
  }
  
  dcc_free(nicks[nr]);

  if (nicks[nr])
  {
    if (nicks[nr]->nickserv) nicks[nr]->nickserv->entry = NULL;
    xfree(nicks[nr]->nick);
    xfree(nicks[nr]->numeric);
    xfree(nicks[nr]->username);
    xfree(nicks[nr]->host);
    xfree(nicks[nr]->userinfo);
    xfree(nicks[nr]->away);
    xfree(nicks[nr]);
  }

  nicks_count--;
  memmove(&nicks[nr], &nicks[nr+1], (nicks_count - nr) * sizeof(dbase_nicks*));
  nicks = (dbase_nicks**)realloc(nicks, nicks_count * sizeof(dbase_nicks*));

  memmove(&nicks_num[index], &nicks_num[index+1], (nicks_count - index) * sizeof(dbase_nicks*));
  nicks_num = (dbase_nicks**)realloc(nicks_num, nicks_count * sizeof(dbase_nicks*));

  return index;
}

/********************************************************************
  NICKS_RENICK
    Ændre personens nick

  Parameter og return:
    [in]  long  numeric = personens numeric
    [in]  char *modes   = personens modes ændring
    [out] long  return
                   < 0  = Fejl
                  >= 0  = OK

********************************************************************/
long nicks_renick(const char *numeric, const char *newnick)
{
  long index, nr, to;
  if ((index = nicks_search_numeric(numeric)) < 0) return index;
  if ((nr = nicks_search_nick(nicks_num[index]->nick)) < 0) return nr;
  if ((to = nicks_search_nick(newnick)) >= 0) return -1-to;
  to += 1;
  to *= -1;

  if (to > nr) to--;

  nicks_num[index]->nick = (char *)realloc(nicks_num[index]->nick, sizeof(char)*(strlen(newnick)+1));
  strcpy(nicks_num[index]->nick, newnick);

  if (to > nr) memmove(&nicks[nr], &nicks[nr+1], (to - nr) * sizeof(dbase_nicks*));
  else if (nr > to) memmove(&nicks[to+1], &nicks[to], (nr - to) * sizeof(dbase_nicks*));
  nicks[to] = nicks_num[index];

  return index;
}

/********************************************************************
  NICKS_SETMODE
    Ændre personens modes
        
  Parameter og return:
    [in]  long  numeric = personens numeric
    [in]  char *modes   = personens modes ændring
    [out] long  return 
                   < 0  = Fejl
                  >= 0  = OK
   
********************************************************************/
long nicks_setmode(const char *numeric, char *modes)
{
  long index;
  unsigned int i, check = 1;
  if (!modes) return 0;
  if ((index = nicks_search_numeric(numeric)) < 0) return index;
  for (i = 0; i < strlen(modes); i++)
  {
    if (modes[i] == '+') check = 1;
    else if (modes[i] == '-') check = 0;
    else if ((modes[i] >= 'a') && (modes[i] <= 'z'))
    {
      if (check) bitadd(nicks_num[index]->modes, modes[i]-'a');
      else bitdel(nicks_num[index]->modes, modes[i]-'a');
    }   
  }
  return index;
}

/********************************************************************
  NICKS_SETAWAY
    Sætter personens away message
        
  Parameter og return:
    [in]  long  numeric  = personenes numeric
    [in]  char *awaymsg  = away message
    [out] long  return 
                   < 0  = Fejl
                  >= 0  = OK
   
********************************************************************/
long nicks_setaway(const char *numeric, const char *awaymsg)
{
  long index;
  if ((index = nicks_search_numeric(numeric)) < 0) return index;
  if (awaymsg)
  {
  nicks_num[index]->away = (char *)realloc(nicks_num[index]->away, sizeof(char)*(strlen(awaymsg)+1));
  strcpy(nicks_num[index]->away, awaymsg);
  }
  else
  {
    free(nicks_num[index]->away);
    nicks_num[index]->away = NULL;
  }
  return index;
}

long nicks_chan_search(dbase_channels_nicks **arr, long low, long high, const char *name)
{
  int res;
  long mid = high - ((high - low) / 2);
  if (low > high) return -1-low;
  res = strcasecmp(name, arr[mid]->channel->name);
  if (res < 0) return nicks_chan_search(arr, low, mid-1, name);
  else if (res > 0) return nicks_chan_search(arr, mid+1, high, name);
  else return mid;
}

long nicks_join_channel(const char *numeric, dbase_channels_nicks *cn)
{
  long index, nr;
  index = nicks_search_numeric(numeric);
  if (index < 0) return index;
  cn->nick = nicks_num[index];
  if ((nr = nicks_chan_search(nicks_num[index]->channels, 0, nicks_num[index]->channels_count-1, cn->channel->name)) < 0)
  {
    nr += 1;
    nr *= -1;
    nicks_num[index]->channels = (dbase_channels_nicks **)realloc(nicks_num[index]->channels, (nicks_num[index]->channels_count+1) * sizeof(dbase_channels_nicks*));
    if (nr < (nicks_num[index]->channels_count++))
      memmove(&nicks_num[index]->channels[nr+1], &nicks_num[index]->channels[nr], (nicks_num[index]->channels_count - nr - 1) * sizeof(dbase_channels_nicks*));
    nicks_num[index]->channels[nr] = cn;
  }
  return nr;
}

long nicks_part_channel(const char *numeric, dbase_channels_nicks *cn)
{
  long index, nr;
  index = nicks_search_numeric(numeric);
  if (index < 0) return index;  
  if ((nr = nicks_chan_search(nicks_num[index]->channels, 0, nicks_num[index]->channels_count-1, cn->channel->name)) < 0)
    return nr;
  
  memmove(&nicks_num[index]->channels[nr], &nicks_num[index]->channels[nr+1], (nicks_num[index]->channels_count - nr - 1) * sizeof(dbase_channels_nicks*));
  nicks_num[index]->channels = (dbase_channels_nicks **)realloc(nicks_num[index]->channels, (--nicks_num[index]->channels_count) * sizeof(dbase_channels_nicks*));
  
  return nr;
}


/********************************************************************
  NICKS_GETINFO
    returnerer oplysningerne om den fundne person til info
        
  Parameter og return:
    [in]  long  numeric  = personenes numeric (eller <0)
    [in]  char *nick     = hvis numeric<0 findes personen med nicket nick
    [out] dbase_nicks *info = Pointer til den struct der skal modtage info'en
    [out] long  return 
                   < 0  = Fejl - informationer i info er ugyldige
                  >= 0  = OK
   
********************************************************************/
dbase_nicks *nicks_getinfo(const char *numeric, const char *nick, int nr)
{
  long index;
  if ((nr >= 0) && (nr < nicks_count))
  {
    return nicks_num[nr];
  }
  else if (numeric)
  {
    if ((index = nicks_search_numeric(numeric)) < 0) return NULL;
    return nicks_num[index];
  }
  else if (nick)
  {
    if ((index = nicks_search_nick(nick)) < 0) return NULL;
    return nicks[index];
  }
  return NULL;
}

/********************************************************************
  NICKS_GETNICK
    returnerer nicket til det angivne numeric
        
  Parameter og return:
    [in]  long  numeric  = personenes numeric
    [out] char *return   = personens nick eller NULL hvis fejl
   
********************************************************************/
const char *nicks_getnick(const char *numeric)
{
  long index;
  if ((index = nicks_search_numeric(numeric)) < 0) return NULL;
  return nicks_num[index]->nick;
}

const char *nicks_getnum(const char *nick)
{
  long index;
  if ((index = nicks_search_nick(nick)) < 0) return NULL;
  return nicks[index]->numeric;
}

/********************************************************************
  NICKS_GETCOUNT
     Returnerer antallet af nicks i databasen
        
  Parameter og return:
    [out] long return = Antallet af nicks i databasen
   
********************************************************************/
long nicks_getcount(void)
{
  return nicks_count;
}

/********************************************************************
  DBASE_CLEAR
    Sletter alle informationer i databaserne

********************************************************************/
void dbase_clear(void)
{
  debug_out(" | |==> Cleaning Nick database...\n");
  while (nicks_count)
    nicks_remove(nicks_num[nicks_count-1]->numeric);
  channels_cleanup();
  chanserv_dbase_cleanup();
  debug_out(" | \\==> Databases successfully removed from memory...\n");

}

