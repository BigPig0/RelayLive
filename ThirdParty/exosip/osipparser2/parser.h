/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001-2015 Aymeric MOIZARD amoizard@antisip.com
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef _MSG_H_
#define _MSG_H_

#ifndef DOXYGEN

#ifndef MINISIZE
#define NUMBER_OF_HEADERS 33
#else
#define NUMBER_OF_HEADERS 22
#endif

/* internal type for parser's config */
typedef struct ___osip_message_config_t {
  char *hname;
  int (*setheader) (osip_message_t *, const char *);
  int ignored_when_invalid;
} __osip_message_config_t;

int __osip_message_call_method (int i, osip_message_t * dest, const char *hvalue);
int __osip_message_is_known_header (const char *hname);

int __osip_find_next_occurence (const char *str, const char *buf, const char **index_of_str, const char *end_of_buf);
int __osip_find_next_crlf (const char *start_of_header, const char **end_of_header);
int __osip_find_next_crlfcrlf (const char *start_of_part, const char **end_of_part);

int __osip_quoted_string_set (const char *name, const char *str, char **result, const char **next);
int __osip_token_set (const char *name, const char *str, char **result, const char **next);


int __osip_generic_param_parseall (osip_list_t * gen_params, const char *params);
#endif

#endif
