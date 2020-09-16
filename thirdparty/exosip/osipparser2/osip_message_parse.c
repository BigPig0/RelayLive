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

#include <osipparser2/internal.h>

#include <osipparser2/osip_port.h>
#include <osipparser2/osip_parser.h>
#include "parser.h"

static void osip_util_replace_all_lws (char *sip_message);
static int osip_message_set__header (osip_message_t * sip, const char *hname, const char *hvalue);
static int msg_headers_parse (osip_message_t * sip, const char *start_of_header, const char **body);
static int msg_osip_body_parse (osip_message_t * sip, const char *start_of_buf, const char **next_body, size_t length);


static int
__osip_message_startline_parsereq (osip_message_t * dest, const char *buf, const char **headers)
{
  const char *p1;
  const char *p2;
  char *requesturi;
  int i;

  dest->sip_method = NULL;
  dest->status_code = 0;
  dest->reason_phrase = NULL;

  *headers = buf;

  /* The first token is the method name: */
  p2 = strchr (buf, ' ');
  if (p2 == NULL)
    return OSIP_SYNTAXERROR;
  if (*(p2 + 1) == '\0' || *(p2 + 2) == '\0')
    return OSIP_SYNTAXERROR;
  if (p2 - buf == 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "No space allowed here\n"));
    return OSIP_SYNTAXERROR;
  }
  dest->sip_method = (char *) osip_malloc (p2 - buf + 1);
  if (dest->sip_method == NULL)
    return OSIP_NOMEM;
  osip_strncpy (dest->sip_method, buf, p2 - buf);

  /* The second token is a sip-url or a uri: */
  p1 = strchr (p2 + 2, ' ');    /* no space allowed inside sip-url */
  if (p1 == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Uncompliant request-uri\n"));
    osip_free (dest->sip_method);
    dest->sip_method = NULL;
    return OSIP_SYNTAXERROR;
  }
  if (p1 - p2 < 2) {
    osip_free (dest->sip_method);
    dest->sip_method = NULL;
    return OSIP_SYNTAXERROR;
  }

  requesturi = (char *) osip_malloc (p1 - p2);
  if (requesturi == NULL) {
    osip_free (dest->sip_method);
    dest->sip_method = NULL;
    return OSIP_NOMEM;
  }
  osip_clrncpy (requesturi, p2 + 1, (p1 - p2 - 1));

  i = osip_uri_init (&(dest->req_uri));
  if (i != 0) {
    osip_free (requesturi);
    requesturi = NULL;
    osip_free (dest->sip_method);
    dest->sip_method = NULL;
    return OSIP_NOMEM;
  }
  i = osip_uri_parse (dest->req_uri, requesturi);
  osip_free (requesturi);
  if (i != 0) {
    osip_free (dest->sip_method);
    dest->sip_method = NULL;
    osip_uri_free (dest->req_uri);
    dest->req_uri = NULL;
    return OSIP_SYNTAXERROR;
  }

  /* find the the version and the beginning of headers */
  {
    const char *hp = p1;

    hp++;                       /* skip space */
    if (*hp == '\0' || *(hp + 1) == '\0' || *(hp + 2) == '\0' || *(hp + 3) == '\0' || *(hp + 4) == '\0' || *(hp + 5) == '\0' || *(hp + 6) == '\0') {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Uncomplete request line\n"));
      osip_free (dest->sip_method);
      dest->sip_method = NULL;
      osip_uri_free (dest->req_uri);
      dest->req_uri = NULL;
      return OSIP_SYNTAXERROR;
    }
    if (((hp[0] != 'S') && (hp[0] != 's')) || ((hp[1] != 'I') && (hp[1] != 'i')) || ((hp[2] != 'P') && (hp[2] != 'p')) || (hp[3] != '/')) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "No crlf found/No SIP/2.0 found\n"));
      osip_free (dest->sip_method);
      dest->sip_method = NULL;
      osip_uri_free (dest->req_uri);
      dest->req_uri = NULL;
      return OSIP_SYNTAXERROR;
    }
    hp = hp + 4;                /* SIP/ */

    while ((*hp != '\r') && (*hp != '\n')) {
      if (*hp) {
        if ((*hp >= '0') && (*hp <= '9'))
          hp++;
        else if (*hp == '.')
          hp++;
        else {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "incorrect sip version string\n"));
          osip_free (dest->sip_method);
          dest->sip_method = NULL;
          osip_uri_free (dest->req_uri);
          dest->req_uri = NULL;
          return OSIP_SYNTAXERROR;
        }
      }
      else {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "No crlf found\n"));
        osip_free (dest->sip_method);
        dest->sip_method = NULL;
        osip_uri_free (dest->req_uri);
        dest->req_uri = NULL;
        return OSIP_SYNTAXERROR;
      }
    }
    if (hp - p1 < 2) {
      osip_free (dest->sip_method);
      dest->sip_method = NULL;
      osip_uri_free (dest->req_uri);
      dest->req_uri = NULL;
      return OSIP_SYNTAXERROR;
    }

    dest->sip_version = (char *) osip_malloc (hp - p1);
    if (dest->sip_version == NULL) {
      osip_free (dest->sip_method);
      dest->sip_method = NULL;
      osip_uri_free (dest->req_uri);
      dest->req_uri = NULL;
      return OSIP_NOMEM;
    }

    osip_strncpy (dest->sip_version, p1 + 1, (hp - p1 - 1));

    if (0 != osip_strcasecmp (dest->sip_version, "SIP/2.0")) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Wrong version number\n"));
    }

    hp++;
    if ((*hp) && ('\r' == hp[-1]) && ('\n' == hp[0]))
      hp++;
    (*headers) = hp;
  }
  return OSIP_SUCCESS;
}

static int
__osip_message_startline_parseresp (osip_message_t * dest, const char *buf, const char **headers)
{
  const char *statuscode;
  const char *reasonphrase;

  dest->req_uri = NULL;
  dest->sip_method = NULL;

  *headers = buf;

  statuscode = strchr (buf, ' ');       /* search for first SPACE */
  if (statuscode == NULL)
    return OSIP_SYNTAXERROR;
  dest->sip_version = (char *) osip_malloc (statuscode - (*headers) + 1);
  if (dest->sip_version == NULL)
    return OSIP_NOMEM;
  osip_strncpy (dest->sip_version, *headers, statuscode - (*headers));

  reasonphrase = strchr (statuscode + 1, ' ');
  if (reasonphrase == NULL) {
    osip_free (dest->sip_version);
    dest->sip_version = NULL;
    return OSIP_SYNTAXERROR;
  }
  /* dest->status_code = (char *) osip_malloc (reasonphrase - statuscode); */
  /* osip_strncpy (dest->status_code, statuscode + 1, reasonphrase - statuscode - 1); */
  if (sscanf (statuscode + 1, "%d", &dest->status_code) != 1) {
    /* Non-numeric status code */
    return OSIP_SYNTAXERROR;
  }

  if (dest->status_code == 0)
    return OSIP_SYNTAXERROR;

  {
    const char *hp = reasonphrase;

    while ((*hp != '\r') && (*hp != '\n')) {
      if (*hp)
        hp++;
      else {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "No crlf found\n"));
        return OSIP_SYNTAXERROR;
      }
    }
    dest->reason_phrase = (char *) osip_malloc (hp - reasonphrase);
    if (dest->reason_phrase == NULL) {
      osip_free (dest->sip_version);
      dest->sip_version = NULL;
      return OSIP_NOMEM;
    }

    osip_strncpy (dest->reason_phrase, reasonphrase + 1, hp - reasonphrase - 1);

    hp++;
    if ((*hp) && ('\r' == hp[-1]) && ('\n' == hp[0]))
      hp++;
    (*headers) = hp;
  }
  return OSIP_SUCCESS;
}

static int
__osip_message_startline_parse (osip_message_t * dest, const char *buf, const char **headers)
{

  if (0 == strncmp ((const char *) buf, (const char *) "SIP/", 4))
    return __osip_message_startline_parseresp (dest, buf, headers);
  else
    return __osip_message_startline_parsereq (dest, buf, headers);
}

int
__osip_find_next_occurence (const char *str, const char *buf, const char **index_of_str, const char *end_of_buf)
{
  int i;

  *index_of_str = NULL;         /* AMD fix */
  if ((NULL == str) || (NULL == buf))
    return OSIP_BADPARAMETER;
  /* TODO? we may prefer strcasestr instead of strstr? */
  for (i = 0; i < 1000; i++) {
    *index_of_str = strstr (buf, str);
    if (NULL == (*index_of_str)) {
      /* if '\0' (when binary data is used) is located before the separator,
         then we have to continue searching */
      const char *ptr = buf + strlen (buf);

      if (end_of_buf - ptr > 0) {
        buf = ptr + 1;
        continue;
      }
      return OSIP_SYNTAXERROR;
    }
    return OSIP_SUCCESS;
  }
  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_BUG, NULL, "This was probably an infinite loop?\n"));
  return OSIP_SYNTAXERROR;
}

/* This method replace all LWS with SP located before the
   initial CRLFCRLF found or the end of the string.
*/
static void
osip_util_replace_all_lws (char *sip_message)
{
  /* const char *end_of_message; */
  char *tmp;

  if (sip_message == NULL)
    return;

  /* end_of_message = sip_message + strlen (sip_message); */

  tmp = sip_message;
  for (; tmp[0] != '\0'; tmp++) {
    if (('\0' == tmp[0])
        || ('\0' == tmp[1]) || ('\0' == tmp[2]) || ('\0' == tmp[3]))
      return;

    if ((('\r' == tmp[0]) && ('\n' == tmp[1])
         && ('\r' == tmp[2]) && ('\n' == tmp[3]))
        || (('\r' == tmp[0]) && ('\r' == tmp[1]))
        || (('\n' == tmp[0]) && ('\n' == tmp[1])))
      return;                   /* end of message */

    if ((('\r' == tmp[0]) && ('\n' == tmp[1])
         && ((' ' == tmp[2]) || ('\t' == tmp[2])))
        || (('\r' == tmp[0])
            && ((' ' == tmp[1]) || ('\t' == tmp[1])))
        || (('\n' == tmp[0]) && ((' ' == tmp[1]) || ('\t' == tmp[1])))) {
      /* replace line end and TAB symbols by SP */
      tmp[0] = ' ';
      tmp[1] = ' ';
      tmp = tmp + 2;
      /* replace all following TAB symbols */
      for (; ('\t' == tmp[0] || ' ' == tmp[0]);) {
        tmp[0] = ' ';
        tmp++;
      }
    }
  }
}

int
__osip_find_next_crlf (const char *start_of_header, const char **end_of_header)
{
  const char *soh = start_of_header;

  *end_of_header = NULL;        /* AMD fix */

  while (('\r' != *soh) && ('\n' != *soh)) {
    if (*soh)
      soh++;
    else {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Final CRLF is missing\n"));
      return OSIP_SYNTAXERROR;
    }
  }

  if (('\r' == soh[0]) && ('\n' == soh[1]))
    /* case 1: CRLF is the separator
       case 2 or 3: CR or LF is the separator */
    soh = soh + 1;


  /* VERIFY if TMP is the end of header or LWS.            */
  /* LWS are extra SP, HT, CR and LF contained in headers. */
  if ((' ' == soh[1]) || ('\t' == soh[1])) {
    /* From now on, incoming message that potentially
       contains LWS must be processed with
       -> void osip_util_replace_all_lws(char *)
       This is because the parser methods does not
       support detection of LWS inside. */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_BUG, NULL, "Message that contains LWS must be processed with osip_util_replace_all_lws(char *tmp) before being parsed.\n"));
    return -2;
  }

  *end_of_header = soh + 1;
  return OSIP_SUCCESS;
}

int
__osip_find_next_crlfcrlf (const char *start_of_part, const char **end_of_part)
{
  const char *start_of_line;
  const char *end_of_line;
  int i;

  start_of_line = start_of_part;

  for (;;) {
    i = __osip_find_next_crlf (start_of_line, &end_of_line);
    if (i == -2) {
    }
    else if (i != 0) {          /* error case??? no end of mesage found */
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Final CRLF is missing\n"));
      return i;
    }
    if ('\0' == end_of_line[0]) {       /* error case??? no end of message found */
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Final CRLF is missing\n"));
      return OSIP_SYNTAXERROR;
    }
    else if ('\r' == end_of_line[0]) {
      if ('\n' == end_of_line[1])
        end_of_line++;
      *end_of_part = end_of_line + 1;
      return OSIP_SUCCESS;
    }
    else if ('\n' == end_of_line[0]) {
      *end_of_part = end_of_line + 1;
      return OSIP_SUCCESS;
    }
    start_of_line = end_of_line;
  }
}

static int
osip_message_set__header (osip_message_t * sip, const char *hname, const char *hvalue)
{
  int my_index;

  if (hname == NULL)
    return OSIP_SYNTAXERROR;

  /* some headers are analysed completely      */
  /* this method is used for selective parsing */
  my_index = __osip_message_is_known_header (hname);
  if (my_index >= 0) {          /* ok */
    int ret;

    ret = __osip_message_call_method (my_index, sip, hvalue);
    if (ret != 0)
      return ret;
    return OSIP_SUCCESS;
  }
  /* unknownheader */
  if (osip_message_set_header (sip, hname, hvalue) != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "Could not set unknown header\n"));
    return OSIP_SUCCESS;
  }

  return OSIP_SUCCESS;
}

int
osip_message_set_multiple_header (osip_message_t * sip, char *hname, char *hvalue)
{
  int i;
  char *ptr, *p;                /* current location of the search */
  char *comma;                  /* This is the separator we are elooking for */
  char *beg;                    /* beg of a header */
  char *end;                    /* end of a header */
  int inquotes, inuri;          /* state for inside/outside of double-qoutes or URI */
  size_t hname_len;

  /* Find header based upon lowercase comparison */
  osip_tolower (hname);

  if (hvalue == NULL) {
    i = osip_message_set__header (sip, hname, hvalue);
    if (i != 0)
      return i;
    return OSIP_SUCCESS;
  }

  ptr = hvalue;
  comma = strchr (ptr, ',');

  hname_len = strlen (hname);

  if (comma == NULL || (hname_len == 4 && strncmp (hname, "date", 4) == 0)
      || (hname_len == 1 && strncmp (hname, "t", 1) == 0)
      || (hname_len == 2 && strncmp (hname, "to", 2) == 0)
      || (hname_len == 1 && strncmp (hname, "f", 1) == 0)
      || (hname_len == 4 && strncmp (hname, "from", 4) == 0)
      || (hname_len == 1 && strncmp (hname, "i", 1) == 0)
      || (hname_len == 7 && strncmp (hname, "call-id", 7) == 0)
      || (hname_len == 4 && strncmp (hname, "cseq", 4) == 0)
      || (hname_len == 1 && strncmp (hname, "s", 1) == 0)
      || (hname_len == 7 && strncmp (hname, "subject", 7) == 0)
      || (hname_len == 7 && strncmp (hname, "expires", 7) == 0)
      || (hname_len == 6 && strncmp (hname, "server", 6) == 0)
      || (hname_len == 10 && strncmp (hname, "user-agent", 10) == 0)
      || (hname_len == 16 && strncmp (hname, "www-authenticate", 16) == 0)
      || (hname_len == 19 && strncmp (hname, "authentication-info", 19) == 0)
      || (hname_len == 18 && strncmp (hname, "proxy-authenticate", 18) == 0)
      || (hname_len == 19 && strncmp (hname, "proxy-authorization", 19) == 0)
      || (hname_len == 25 && strncmp (hname, "proxy-authentication-info", 25) == 0)
      || (hname_len == 12 && strncmp (hname, "organization", 12) == 0)
      || (hname_len == 13 && strncmp (hname, "authorization", 13) == 0)
      || (hname_len == 1 && strncmp (hname, "r", 1) == 0) /* refer-to */
      || (hname_len == 8 && strncmp (hname, "refer-to", 8) == 0)
      || (hname_len == 1 && strncmp (hname, "b", 1) == 0) /* referred-by */
      || (hname_len == 11 && strncmp (hname, "referred-by", 11) == 0))
    /* there is no multiple header! likely      */
    /* to happen most of the time...            */
    /* or hname is a TEXT-UTF8-TRIM and may     */
    /* contain a comma. this is not a separator */
    /* THIS DOES NOT WORK FOR UNKNOWN HEADER!!!! */
  {
    i = osip_message_set__header (sip, hname, hvalue);
    if (i != 0)
      return i;
    return OSIP_SUCCESS;
  }

  beg = hvalue;
  inquotes = 0;
  inuri = 0;
  /* Seach for a comma that is not within quotes or a URI */
  for (;; ptr++)
  {
    switch (*ptr)
    {
    case '"':
      /* Check that the '"' is not escaped */
      for (i = 0, p = ptr; p >= beg && *p == '\\'; p--, i++);
      if (i % 2 == 0)
        inquotes = !inquotes; /* the '"' was not escaped */
      break;

    case '<':
      if (!inquotes)
      {
        if (!inuri)
        {
          if((osip_strncasecmp(ptr+1, "sip:", 4) == 0
              || osip_strncasecmp(ptr+1, "sips:", 5) == 0
              || osip_strncasecmp(ptr+1, "http:", 5) == 0
              || osip_strncasecmp(ptr+1, "https:", 6) == 0
              || osip_strncasecmp(ptr+1, "tel:", 4) == 0)
              && strchr(ptr, '>'))
            inuri = 1;
        }
	/*
	  else {
	  if we found such sequence: "<sip:" "<sip:" ">"
	  It might be a valid header containing data and not URIs.
	  Thus, we ignore inuri
	  }
	*/
      }
      break;

    case '>':
      if (!inquotes)
      {
        if (inuri)
          inuri = 0;
      }
      break;

    case '\0':
      /* we discard any validation we tried: no valid uri detected */
      inquotes=0;
      inuri=0;
    case ',':
      if (!inquotes && !inuri)
      {
        char *avalue;

        if (beg[0] == '\0')
          return OSIP_SUCCESS; /* empty header */

        end = ptr;
        if (end - beg + 1 < 2)
	  {
	    beg=end+1;
	    break; /* skip empty header */
	  }
        avalue = (char *) osip_malloc (end - beg + 1);
        if (avalue==NULL)
          return OSIP_NOMEM;
        osip_clrncpy (avalue, beg, end - beg);
        /* really store the header in the sip structure */
        i = osip_message_set__header (sip, hname, avalue);
        osip_free (avalue);
        if (i != 0)
          return i;
        beg = end + 1;
      }
      if (*ptr == '\0')
        return OSIP_SUCCESS;
      break;

    default:
      break;
    }
  }
}

/* set all headers */
static int
msg_headers_parse (osip_message_t * sip, const char *start_of_header, const char **body)
{
  const char *colon_index;      /* index of ':' */
  char *hname;
  char *hvalue;
  const char *end_of_header;
  int i;

  for (;;) {
    if (start_of_header[0] == '\0') {   /* final CRLF is missing */
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "SIP message does not end with CRLFCRLF\n"));
      return OSIP_SUCCESS;
    }

    i = __osip_find_next_crlf (start_of_header, &end_of_header);
    if (i == -2) {
    }
    else if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "End of header Not found\n"));
      return i;                 /* this is an error case!     */
    }

    /* the list of headers MUST always end with  */
    /* CRLFCRLF (also CRCR and LFLF are allowed) */
    if ((start_of_header[0] == '\r') || (start_of_header[0] == '\n')) {
      *body = start_of_header;
      return OSIP_SUCCESS;      /* end of header found        */
    }

    /* find the header name */
    colon_index = strchr (start_of_header, ':');
    if (colon_index == NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "End of header Not found\n"));
      return OSIP_SYNTAXERROR;  /* this is also an error case */
    }
    if (colon_index - start_of_header + 1 < 2)
      return OSIP_SYNTAXERROR;
    if (end_of_header <= colon_index) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Malformed message\n"));
      return OSIP_SYNTAXERROR;
    }
    hname = (char *) osip_malloc (colon_index - start_of_header + 1);
    if (hname == NULL)
      return OSIP_NOMEM;
    osip_clrncpy (hname, start_of_header, colon_index - start_of_header);

    {
      const char *end;

      /* END of header is (end_of_header-2) if header separation is CRLF */
      /* END of header is (end_of_header-1) if header separation is CR or LF */
      if ((end_of_header[-2] == '\r') || (end_of_header[-2] == '\n'))
        end = end_of_header - 2;
      else
        end = end_of_header - 1;
      if ((end) - colon_index < 2)
        hvalue = NULL;          /* some headers (subject) can be empty */
      else {
        hvalue = (char *) osip_malloc ((end) - colon_index + 1);
        if (hvalue == NULL) {
          osip_free (hname);
          return OSIP_NOMEM;
        }
        osip_clrncpy (hvalue, colon_index + 1, (end) - colon_index - 1);
      }
    }

    /* hvalue MAY contains multiple value. In this case, they   */
    /* are separated by commas. But, a comma may be part of a   */
    /* quoted-string ("here, and there" is an example where the */
    /* comma is not a separator!) */
    i = osip_message_set_multiple_header (sip, hname, hvalue);

    osip_free (hname);
    if (hvalue != NULL)
      osip_free (hvalue);
    if (i != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "End of header Not found\n"));
      return OSIP_SYNTAXERROR;
    }

    /* continue on the next header */
    start_of_header = end_of_header;
  }

/* Unreachable code
 OSIP_TRACE (osip_trace
	      (__FILE__, __LINE__, OSIP_BUG, NULL,
	       "This code cannot be reached\n")); */
  return OSIP_SYNTAXERROR;
}

static int
msg_osip_body_parse (osip_message_t * sip, const char *start_of_buf, const char **next_body, size_t length)
{
  const char *start_of_body;
  const char *end_of_body;
  const char *end_of_buf;
  char *tmp;
  int i;

  char *sep_boundary;
  size_t len_sep_boundary;
  osip_generic_param_t *ct_param;

  if (sip->content_type == NULL || sip->content_type->type == NULL || sip->content_type->subtype == NULL)
    return OSIP_SUCCESS;        /* no body is attached */

  if (0 != osip_strcasecmp (sip->content_type->type, "multipart")) {
    size_t osip_body_len;

    if (start_of_buf[0] == '\0')
      return OSIP_SYNTAXERROR;  /* final CRLF is missing */
    /* get rid of the first CRLF */
    if ('\r' == start_of_buf[0]) {
      if ('\n' == start_of_buf[1])
        start_of_body = start_of_buf + 2;
      else
        start_of_body = start_of_buf + 1;
    }
    else if ('\n' == start_of_buf[0])
      start_of_body = start_of_buf + 1;
    else
      return OSIP_SYNTAXERROR;  /* message does not end with CRLFCRLF, CRCR or LFLF */

    /* update length (without CRLFCRLF */
    length = length - (start_of_body - start_of_buf);   /* fixed 24 08 2004 */
    if (length <= 0)
      return OSIP_SYNTAXERROR;

    if (sip->content_length != NULL)
      osip_body_len = osip_atoi (sip->content_length->value);
    else {
      /* if content_length does not exist, set it. */
      char tmp[16];
      osip_body_len = length;
      sprintf (tmp, "%i", (int) osip_body_len);
      i = osip_message_set_content_length (sip, tmp);
      if (i != 0)
	return i;
    }

    if (length < osip_body_len) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Message was not receieved enterely. length=%i osip_body_len=%i\n", (int) length, (int) osip_body_len));
      return OSIP_SYNTAXERROR;
    }

    end_of_body = start_of_body + osip_body_len;
    tmp = osip_malloc (end_of_body - start_of_body + 2);
    if (tmp == NULL)
      return OSIP_NOMEM;
    memcpy (tmp, start_of_body, end_of_body - start_of_body);
    tmp[end_of_body - start_of_body] = '\0';

    i = osip_message_set_body (sip, tmp, end_of_body - start_of_body);
    osip_free (tmp);
    if (i != 0)
      return i;
    return OSIP_SUCCESS;
  }

  /* find the boundary */
  i = osip_generic_param_get_byname (&sip->content_type->gen_params, "boundary", &ct_param);
  if (i != 0)
    return i;

  if (ct_param == NULL)
    return OSIP_SYNTAXERROR;
  if (ct_param->gvalue == NULL)
    return OSIP_SYNTAXERROR;    /* No boundary but multiple headers??? */

  {
    const char *boundary_prefix = "\n--";

    size_t len = strlen (ct_param->gvalue);

    sep_boundary = (char *) osip_malloc (len + 4);
    if (sep_boundary == NULL)
      return OSIP_NOMEM;
    strcpy (sep_boundary, boundary_prefix);
    if (ct_param->gvalue[0] == '"' && ct_param->gvalue[len - 1] == '"')
      strncat (sep_boundary, ct_param->gvalue + 1, len - 2);
    else
      strncat (sep_boundary, ct_param->gvalue, len);
  }

  len_sep_boundary = strlen (sep_boundary);

  *next_body = NULL;
  start_of_body = start_of_buf;

  end_of_buf = start_of_buf + length;

  for (;;) {
    size_t body_len = 0;

    i = __osip_find_next_occurence (sep_boundary, start_of_body, &start_of_body, end_of_buf);
    if (i != 0) {
      osip_free (sep_boundary);
      return i;
    }

    i = __osip_find_next_occurence (sep_boundary, start_of_body + len_sep_boundary, &end_of_body, end_of_buf);
    if (i != 0) {
      osip_free (sep_boundary);
      return i;
    }

    /* this is the real beginning of body */
    start_of_body = start_of_body + len_sep_boundary + 1;
    if ('\n' == start_of_body[0] || '\r' == start_of_body[0])
      start_of_body++;

    body_len = end_of_body - start_of_body;

    /* Skip CR before end boundary. */
    if (*(end_of_body - 1) == '\r')
      body_len--;

    tmp = osip_malloc (body_len + 2);
    if (tmp == NULL) {
      osip_free (sep_boundary);
      return OSIP_NOMEM;
    }
    memcpy (tmp, start_of_body, body_len);
    tmp[body_len] = '\0';

    i = osip_message_set_body_mime (sip, tmp, body_len);
    osip_free (tmp);
    if (i != 0) {
      osip_free (sep_boundary);
      return i;
    }

    if (strncmp (end_of_body + len_sep_boundary, "--", 2) == 0) {       /* end of all bodies */
      *next_body = end_of_body;
      osip_free (sep_boundary);
      return OSIP_SUCCESS;
    }

    /* continue on the next body */
    start_of_body = end_of_body;
  }
  /* Unreachable code */
  /* osip_free (sep_boundary); */
  return OSIP_SYNTAXERROR;
}

/* osip_message_t *sip is filled while analysing buf */
static int
_osip_message_parse (osip_message_t * sip, const char *buf, size_t length, int sipfrag)
{
  int i;
  const char *next_header_index;
  char *tmp;
  char *beg;

  tmp = osip_malloc (length + 2);
  if (tmp == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Could not allocate memory.\n"));
    return OSIP_NOMEM;
  }
  beg = tmp;
  memcpy (tmp, buf, length);    /* may contain binary data */
  tmp[length] = '\0';
  /* skip initial \r\n */
  while (tmp[0] == '\r' || tmp[0] == '\n')
    tmp++;
  osip_util_replace_all_lws (tmp);
  /* parse request or status line */
  i = __osip_message_startline_parse (sip, tmp, &next_header_index);
  if (i != 0 && !sipfrag) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Could not parse start line of message.\n"));
    osip_free (beg);
    return i;
  }
  tmp = (char *) next_header_index;

  /* parse headers */
  i = msg_headers_parse (sip, tmp, &next_header_index);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "error in msg_headers_parse()\n"));
    osip_free (beg);
    return i;
  }
  tmp = (char *) next_header_index;

  if (sip->content_length != NULL && sip->content_length->value == NULL) {
    /* empty content_length header */
    osip_content_length_free(sip->content_length);
    sip->content_length=NULL;
  }

  if (sip->content_length != NULL && sip->content_length->value != NULL && atoi(sip->content_length->value) >0) {
    /* body exist */
  } else if (sip->content_length == NULL && '\r' == next_header_index[0] && '\n' == next_header_index[1] && length - (tmp - beg) - (2) >0) {
    /* body exist */
  } else if (sip->content_length == NULL && '\n' == next_header_index[0] && length - (tmp - beg) - (1) >0) {
    /* body exist */
  } else {
    if (sip->content_length == NULL)
      osip_message_set_content_length (sip, "0");
    osip_free (beg);
    return OSIP_SUCCESS;        /* no body found */
  }

  i = msg_osip_body_parse (sip, tmp, &next_header_index, length - (tmp - beg));
  osip_free (beg);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "error in msg_osip_body_parse()\n"));
    return i;
  }

  /* this is mandatory in the oSIP stack */
  if (sip->content_length == NULL)
    osip_message_set_content_length (sip, "0");

  return OSIP_SUCCESS;
}

int
osip_message_parse (osip_message_t * sip, const char *buf, size_t length)
{
  return _osip_message_parse (sip, buf, length, 0);
}

int
osip_message_parse_sipfrag (osip_message_t * sip, const char *buf, size_t length)
{
  return _osip_message_parse (sip, buf, length, 1);
}


/* This method just add a received parameter in the Via
   as requested by rfc3261 */
int
osip_message_fix_last_via_header (osip_message_t * request, const char *ip_addr, int port)
{
  osip_generic_param_t *rport;
  osip_via_t *via;

  /* get Top most Via header: */
  if (request == NULL)
    return OSIP_BADPARAMETER;
  if (MSG_IS_RESPONSE (request))
    return OSIP_SUCCESS;        /* Don't fix Via header */

  via = osip_list_get (&request->vias, 0);
  if (via == NULL || via->host == NULL)
    /* Hey, we could build it? */
    return OSIP_BADPARAMETER;

  osip_via_param_get_byname (via, "rport", &rport);
  if (rport != NULL) {
    if (rport->gvalue == NULL) {
      rport->gvalue = (char *) osip_malloc (9);
      if (rport->gvalue == NULL)
        return OSIP_NOMEM;
#if !defined __PALMOS__ && (defined WIN32 || defined _WIN32_WCE)
      _snprintf (rport->gvalue, 8, "%i", port);
#else
      snprintf (rport->gvalue, 8, "%i", port);
#endif
    }                           /* else bug? */
  }

  /* only add the received parameter if the 'sent-by' value does not contains
     this ip address */
  if (0 == strcmp (via->host, ip_addr)) /* don't need the received parameter */
    return OSIP_SUCCESS;
  osip_via_set_received (via, osip_strdup (ip_addr));
  return OSIP_SUCCESS;
}

const char *
osip_message_get_reason (int replycode)
{
  struct code_to_reason {
    int code;
    const char *reason;
  };

  static const struct code_to_reason reasons1xx[] = {
    {100, "Trying"},
    {180, "Ringing"},
    {181, "Call Is Being Forwarded"},
    {182, "Queued"},
    {183, "Session Progress"},
  };
  static const struct code_to_reason reasons2xx[] = {
    {200, "OK"},
    {202, "Accepted"},
  };
  static const struct code_to_reason reasons3xx[] = {
    {300, "Multiple Choices"},
    {301, "Moved Permanently"},
    {302, "Moved Temporarily"},
    {305, "Use Proxy"},
    {380, "Alternative Service"},
  };
  static const struct code_to_reason reasons4xx[] = {
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Timeout"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Conditional Request Failed"},
    {413, "Request Entity Too Large"},
    {414, "Request-URI Too Long"},
    {415, "Unsupported Media Type"},
    {416, "Unsupported URI Scheme"},
    {417, "Unknown Resource-Priority"},
    {420, "Bad Extension"},
    {421, "Extension Required"},
    {422, "Session Interval Too Small"},
    {423, "Interval Too Brief"},
    {469, "Bad Info Package"},
    {480, "Temporarily Unavailable"},
    {481, "Call/Transaction Does Not Exist"},
    {482, "Loop Detected"},
    {483, "Too Many Hops"},
    {484, "Address Incomplete"},
    {485, "Ambiguous"},
    {486, "Busy Here"},
    {487, "Request Terminated"},
    {488, "Not Acceptable Here"},
    {489, "Bad Event"},
    {491, "Request Pending"},
    {493, "Undecipherable"},
  };
  static const struct code_to_reason reasons5xx[] = {
    {500, "Server Internal Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Server Time-out"},
    {505, "Version Not Supported"},
    {513, "Message Too Large"},
  };
  static const struct code_to_reason reasons6xx[] = {
    {600, "Busy Everywhere"},
    {603, "Decline"},
    {604, "Does Not Exist Anywhere"},
    {606, "Not Acceptable"},
    {687, "Dialog Terminated"}
  };
  const struct code_to_reason *reasons;
  int len, i;

  switch (replycode / 100) {
  case 1:
    reasons = reasons1xx;
    len = sizeof (reasons1xx) / sizeof (*reasons);
    break;
  case 2:
    reasons = reasons2xx;
    len = sizeof (reasons2xx) / sizeof (*reasons);
    break;
  case 3:
    reasons = reasons3xx;
    len = sizeof (reasons3xx) / sizeof (*reasons);
    break;
  case 4:
    reasons = reasons4xx;
    len = sizeof (reasons4xx) / sizeof (*reasons);
    break;
  case 5:
    reasons = reasons5xx;
    len = sizeof (reasons5xx) / sizeof (*reasons);
    break;
  case 6:
    reasons = reasons6xx;
    len = sizeof (reasons6xx) / sizeof (*reasons);
    break;
  default:
    return NULL;
  }

  for (i = 0; i < len; i++)
    if (reasons[i].code == replycode)
      return reasons[i].reason;

  /* Not found. */
  return NULL;
}
