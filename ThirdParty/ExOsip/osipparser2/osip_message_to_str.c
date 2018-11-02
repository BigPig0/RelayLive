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

#define MIME_MAX_BOUNDARY_LEN 70

extern const char *osip_protocol_version;

static int strcat_simple_header (char **_string, size_t * malloc_size, char **_message, void *ptr_header, char *header_name, size_t size_of_header, int (*xxx_to_str) (void *, char **), char **next);
static int strcat_headers_one_per_line (char **_string, size_t * malloc_size, char **_message, osip_list_t * headers, char *header, size_t size_of_header, int (*xxx_to_str) (void *, char **), char **next);


static int
__osip_message_startline_to_strreq (osip_message_t * sip, char **dest)
{
  const char *sip_version;
  char *tmp;
  char *rquri;
  int i;

  *dest = NULL;
  if ((sip == NULL) || (sip->req_uri == NULL) || (sip->sip_method == NULL))
    return OSIP_BADPARAMETER;

  i = osip_uri_to_str (sip->req_uri, &rquri);
  if (i != 0)
    return i;

  if (sip->sip_version == NULL)
    sip_version = osip_protocol_version;
  else
    sip_version = sip->sip_version;

  *dest = (char *) osip_malloc (strlen (sip->sip_method)
                                + strlen (rquri) + strlen (sip_version) + 3);
  if (*dest == NULL) {
    osip_free (rquri);
    return OSIP_NOMEM;
  }
  tmp = *dest;

  tmp = osip_str_append (tmp, sip->sip_method);
  *tmp = ' ';
  tmp++;
  tmp = osip_str_append (tmp, rquri);
  *tmp = ' ';
  tmp++;
  strcpy (tmp, sip_version);

  osip_free (rquri);
  return OSIP_SUCCESS;
}

static int
__osip_message_startline_to_strresp (osip_message_t * sip, char **dest)
{
  char *tmp;
  const char *sip_version;
  char status_code[5];

  *dest = NULL;
  if ((sip == NULL) || (sip->reason_phrase == NULL)
      || (sip->status_code < 100) || (sip->status_code > 699))
    return OSIP_BADPARAMETER;

  if (sip->sip_version == NULL)
    sip_version = osip_protocol_version;
  else
    sip_version = sip->sip_version;

  sprintf (status_code, "%u", sip->status_code);

  *dest = (char *) osip_malloc (strlen (sip_version)
                                + 3 + strlen (sip->reason_phrase) + 4);
  if (*dest == NULL)
    return OSIP_NOMEM;
  tmp = *dest;

  tmp = osip_str_append (tmp, sip_version);
  *tmp = ' ';
  tmp++;

  tmp = osip_strn_append (tmp, status_code, 3);
  *tmp = ' ';
  tmp++;
  strcpy (tmp, sip->reason_phrase);

  return OSIP_SUCCESS;
}

static int
__osip_message_startline_to_str (osip_message_t * sip, char **dest)
{

  if (sip->sip_method != NULL)
    return __osip_message_startline_to_strreq (sip, dest);
  if (sip->status_code != 0)
    return __osip_message_startline_to_strresp (sip, dest);

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, TRACE_LEVEL1, NULL, "ERROR method has no value or status code is 0!\n"));
  return OSIP_BADPARAMETER;     /* should never come here */
}

char *
osip_message_get_reason_phrase (const osip_message_t * sip)
{
  return sip->reason_phrase;
}

int
osip_message_get_status_code (const osip_message_t * sip)
{
  return sip->status_code;
}

char *
osip_message_get_method (const osip_message_t * sip)
{
  return sip->sip_method;
}

char *
osip_message_get_version (const osip_message_t * sip)
{
  return sip->sip_version;
}

osip_uri_t *
osip_message_get_uri (const osip_message_t * sip)
{
  return sip->req_uri;
}

static int
strcat_simple_header (char **_string, size_t * malloc_size, char **_message, void *ptr_header, char *header_name, size_t size_of_header, int (*xxx_to_str) (void *, char **), char **next)
{
  char *string;
  char *message;
  char *tmp;
  int i;

  string = *_string;
  message = *_message;

  if (ptr_header != NULL) {
    if (*malloc_size < message - string + 100 + size_of_header)
      /* take some memory avoid to osip_realloc too much often */
    {                           /* should not happen often */
      size_t size = message - string;

      *malloc_size = message - string + size_of_header + 100;
      string = osip_realloc (string, *malloc_size);
      if (string == NULL) {
        osip_free (*_string);   /* pointer for string */
        *_string = NULL;
        *_message = NULL;
        return OSIP_NOMEM;
      }
      *_string = string;
      message = string + size;
    }
    message = osip_strn_append (message, header_name, size_of_header);

    i = xxx_to_str (ptr_header, &tmp);
    if (i != 0) {
      *_string = string;
      *_message = message;
      *next = NULL;
      return i;
    }
    if (*malloc_size < message - string + strlen (tmp) + 100) {
      size_t size = message - string;

      *malloc_size = message - string + strlen (tmp) + 100;
      string = osip_realloc (string, *malloc_size);
      if (string == NULL) {
        osip_free (*_string);   /* pointer for string */
        *_string = NULL;
        *_message = NULL;
        return OSIP_NOMEM;
      }
      *_string = string;
      message = string + size;
    }

    message = osip_str_append (message, tmp);
    osip_free (tmp);
    message = osip_strn_append (message, CRLF, 2);
  }
  *_string = string;
  *_message = message;
  *next = message;
  return OSIP_SUCCESS;
}

static int
strcat_headers_one_per_line (char **_string, size_t * malloc_size, char **_message, osip_list_t * headers, char *header, size_t size_of_header, int (*xxx_to_str) (void *, char **), char **next)
{
  char *string;
  char *message;
  char *tmp;
  int i;
  osip_list_iterator_t it;
  void *elt = osip_list_get_first(headers, &it);

  string = *_string;
  message = *_message;

  while (elt != OSIP_SUCCESS) {

    if (*malloc_size < message - string + 100 + size_of_header)
      /* take some memory avoid to osip_realloc too much often */
    {                           /* should not happen often */
      size_t size = message - string;

      *malloc_size = message - string + size_of_header + 100;
      string = osip_realloc (string, *malloc_size);
      if (string == NULL) {
        osip_free (*_string);   /* pointer for string */
        *_string = NULL;
        *_message = NULL;
        return OSIP_NOMEM;
      }
      *_string = string;
      message = string + size;
    }
    osip_strncpy (message, header, size_of_header);
    i = xxx_to_str (elt, &tmp);
    if (i != 0) {
      *_string = string;
      *_message = message;
      *next = NULL;
      return i;
    }
    message = message + strlen (message);

    if (*malloc_size < message - string + strlen (tmp) + 100) {
      size_t size = message - string;

      *malloc_size = message - string + strlen (tmp) + 100;
      string = osip_realloc (string, *malloc_size);
      if (string == NULL) {
        osip_free (*_string);   /* pointer for string */
        *_string = NULL;
        *_message = NULL;
        return OSIP_NOMEM;
      }
      *_string = string;
      message = string + size;
    }
    message = osip_str_append (message, tmp);
    osip_free (tmp);
    message = osip_strn_append (message, CRLF, 2);
    elt = osip_list_get_next(&it);
  }
  *_string = string;
  *_message = message;
  *next = message;
  return OSIP_SUCCESS;
}

 /* return values:
    1: structure and buffer "message" are identical.
    2: buffer "message" is not up to date with the structure info (call osip_message_to_str to update it).
    -1 on error.
  */
int
osip_message_get__property (const osip_message_t * sip)
{
  if (sip == NULL)
    return OSIP_BADPARAMETER;
  return sip->message_property;
}

int
osip_message_force_update (osip_message_t * sip)
{
  if (sip == NULL)
    return OSIP_BADPARAMETER;
  sip->message_property = 2;
  return OSIP_SUCCESS;
}

static int
_osip_message_realloc (char **message, char **dest, size_t needed, size_t * malloc_size)
{
  size_t size = *message - *dest;

  if (*malloc_size < (size_t) (size + needed + 100)) {
    *malloc_size = size + needed + 100;
    *dest = osip_realloc (*dest, *malloc_size);
    if (*dest == NULL)
      return OSIP_NOMEM;
    *message = *dest + size;
  }

  return OSIP_SUCCESS;
}

static int
_osip_message_to_str (osip_message_t * sip, char **dest, size_t * message_length, int sipfrag)
{
  size_t malloc_size;
  size_t total_length = 0;

  /* Added at SIPit day1 */
  char *start_of_bodies;
  char *content_length_to_modify = NULL;

  char *message;
  char *next;
  char *tmp;
  int pos;
  int i;
  char *boundary = NULL;

  malloc_size = SIP_MESSAGE_MAX_LENGTH;

  *dest = NULL;
  if (sip == NULL)
    return OSIP_BADPARAMETER;

  {
    if (1 == osip_message_get__property (sip)) {        /* message is already available in "message" */

      *dest = osip_malloc (sip->message_length + 1);
      if (*dest == NULL)
        return OSIP_NOMEM;
      memcpy (*dest, sip->message, sip->message_length);
      (*dest)[sip->message_length] = '\0';
      if (message_length != NULL)
        *message_length = sip->message_length;
      return OSIP_SUCCESS;
    }
    else {
      /* message should be rebuilt: delete the old one if exists. */
      osip_free (sip->message);
      sip->message = NULL;
    }
  }

  message = (char *) osip_malloc (SIP_MESSAGE_MAX_LENGTH);      /* ???? message could be > 4000  */
  if (message == NULL)
    return OSIP_NOMEM;
  *dest = message;

  /* add the first line of message */
  i = __osip_message_startline_to_str (sip, &tmp);
  if (i != 0) {
    if (!sipfrag) {
      osip_free (*dest);
      *dest = NULL;
      return i;
    }

    /* A start-line isn't required for message/sipfrag parts. */
  }
  else {
    size_t message_len = strlen(tmp);
    if (_osip_message_realloc (&message, dest, message_len + 3, &malloc_size) < 0) {
      osip_free (tmp);
      *dest = NULL;
      return OSIP_NOMEM;
    }

    message = osip_str_append (message, tmp);
    osip_free (tmp);
    message = osip_strn_append (message, CRLF, 2);
  }

  {
    struct to_str_table {
      char header_name[30];
      int header_length;
      osip_list_t *header_list;
      void *header_data;
      int (*to_str) (void *, char **);
    }
#ifndef MINISIZE
    table[25] =
#else
    table[16] =
#endif
    {
      {
      "Via: ", 5, NULL, NULL, (int (*)(void *, char **)) &osip_via_to_str}, {
      "Record-Route: ", 14, NULL, NULL, (int (*)(void *, char **)) &osip_record_route_to_str}, {
      "Route: ", 7, NULL, NULL, (int (*)(void *, char **)) &osip_route_to_str}, {
      "From: ", 6, NULL, NULL, (int (*)(void *, char **)) &osip_from_to_str}, {
      "To: ", 4, NULL, NULL, (int (*)(void *, char **)) &osip_to_to_str}, {
      "Call-ID: ", 9, NULL, NULL, (int (*)(void *, char **)) &osip_call_id_to_str}, {
      "CSeq: ", 6, NULL, NULL, (int (*)(void *, char **)) &osip_cseq_to_str}, {
      "Contact: ", 9, NULL, NULL, (int (*)(void *, char **)) &osip_contact_to_str}, {
      "Authorization: ", 15, NULL, NULL, (int (*)(void *, char **)) &osip_authorization_to_str}, {
      "WWW-Authenticate: ", 18, NULL, NULL, (int (*)(void *, char **)) &osip_www_authenticate_to_str}, {
      "Proxy-Authenticate: ", 20, NULL, NULL, (int (*)(void *, char **)) &osip_www_authenticate_to_str}, {
      "Proxy-Authorization: ", 21, NULL, NULL, (int (*)(void *, char **)) &osip_authorization_to_str}, {
      "Call-Info: ", 11, NULL, NULL, (int (*)(void *, char **)) &osip_call_info_to_str}, {
      "Content-Type: ", 14, NULL, NULL, (int (*)(void *, char **)) &osip_content_type_to_str}, {
      "Mime-Version: ", 14, NULL, NULL, (int (*)(void *, char **)) &osip_content_length_to_str},
#ifndef MINISIZE
      {
      "Allow: ", 7, NULL, NULL, (int (*)(void *, char **)) &osip_allow_to_str}, {
      "Content-Encoding: ", 18, NULL, NULL, (int (*)(void *, char **)) &osip_content_encoding_to_str}, {
      "Alert-Info: ", 12, NULL, NULL, (int (*)(void *, char **)) &osip_call_info_to_str}, {
      "Error-Info: ", 12, NULL, NULL, (int (*)(void *, char **)) &osip_call_info_to_str}, {
      "Accept: ", 8, NULL, NULL, (int (*)(void *, char **)) &osip_accept_to_str}, {
      "Accept-Encoding: ", 17, NULL, NULL, (int (*)(void *, char **)) &osip_accept_encoding_to_str}, {
      "Accept-Language: ", 17, NULL, NULL, (int (*)(void *, char **)) &osip_accept_language_to_str}, {
      "Authentication-Info: ", 21, NULL, NULL, (int (*)(void *, char **)) &osip_authentication_info_to_str}, {
      "Proxy-Authentication-Info: ", 27, NULL, NULL, (int (*)(void *, char **)) &osip_authentication_info_to_str},
#endif
      { {
      '\0'}, 0, NULL, NULL, NULL}
    };
    table[0].header_list = &sip->vias;
    table[1].header_list = &sip->record_routes;
    table[2].header_list = &sip->routes;
    table[3].header_data = sip->from;
    table[4].header_data = sip->to;
    table[5].header_data = sip->call_id;
    table[6].header_data = sip->cseq;
    table[7].header_list = &sip->contacts;
    table[8].header_list = &sip->authorizations;
    table[9].header_list = &sip->www_authenticates;
    table[10].header_list = &sip->proxy_authenticates;
    table[11].header_list = &sip->proxy_authorizations;
    table[12].header_list = &sip->call_infos;
    table[13].header_data = sip->content_type;
    table[14].header_data = sip->mime_version;
#ifndef MINISIZE
    table[15].header_list = &sip->allows;
    table[16].header_list = &sip->content_encodings;
    table[17].header_list = &sip->alert_infos;
    table[18].header_list = &sip->error_infos;
    table[19].header_list = &sip->accepts;
    table[20].header_list = &sip->accept_encodings;
    table[21].header_list = &sip->accept_languages;
    table[22].header_list = &sip->authentication_infos;
    table[23].header_list = &sip->proxy_authentication_infos;
#endif

    pos = 0;
    while (table[pos].header_name[0] != '\0') {
      if (table[13].header_list == NULL)
        i = strcat_simple_header (dest, &malloc_size, &message, table[pos].header_data, table[pos].header_name, table[pos].header_length, ((int (*)(void *, char **))
                                                                                                                                           table[pos].to_str), &next);
      i = strcat_headers_one_per_line (dest, &malloc_size, &message, table[pos].header_list, table[pos].header_name, table[pos].header_length, ((int (*)(void *, char **))
                                                                                                                                                table[pos].to_str), &next);
      if (i != 0) {
        osip_free (*dest);
        *dest = NULL;
        return i;
      }
      message = next;

      pos++;
    }
  }

  {
    osip_list_iterator_t it;
    osip_header_t *header = (osip_header_t *) osip_list_get_first(&sip->headers, &it);  
    while (header != OSIP_SUCCESS) {
      
      size_t header_len = 0;
      
      i = osip_header_to_str (header, &tmp);
      if (i != 0) {
	osip_free (*dest);
	*dest = NULL;
	return i;
      }
      
      header_len = strlen (tmp);
      
      if (_osip_message_realloc (&message, dest, header_len + 3, &malloc_size) < 0) {
	osip_free (tmp);
	*dest = NULL;
	return OSIP_NOMEM;
      }
      
      message = osip_str_append (message, tmp);
      osip_free (tmp);
      message = osip_strn_append (message, CRLF, 2);
      
      header = (osip_header_t *) osip_list_get_next(&it);
    }
  }
  
  /* we have to create the body before adding the contentlength */
  /* add enough lenght for "Content-Length: " */

  if (_osip_message_realloc (&message, dest, 16, &malloc_size) < 0)
    return OSIP_NOMEM;

  if (sipfrag && osip_list_eol (&sip->bodies, 0)) {
    /* end of headers */
    osip_strncpy (message, CRLF, 2);
    message = message + 2;

    /* same remark as at the beginning of the method */
    sip->message_property = 1;
    sip->message = osip_strdup (*dest);
    sip->message_length = message - *dest;
    if (message_length != NULL)
      *message_length = message - *dest;

    return OSIP_SUCCESS;        /* it's all done */
  }

  osip_strncpy (message, "Content-Length: ", 16);
  message = message + 16;

  /* SIPit Day1
     ALWAYS RECALCULATE?
     if (sip->contentlength!=NULL)
     {
     i = osip_content_length_to_str(sip->contentlength, &tmp);
     if (i!=0) {
     osip_free(*dest);
     *dest = NULL;
     return i;
     }
     osip_strncpy(message,tmp,strlen(tmp));
     osip_free(tmp);
     }
     else
     { */
  if (osip_list_eol (&sip->bodies, 0))  /* no body */
    message = osip_strn_append (message, "0", 1);
  else {
    /* BUG: p130 (rfc2543bis-04)
       "No SP after last token or quoted string"

       In fact, if extra spaces exist: the stack can't be used
       to make user-agent that wants to make authentication...
       This should be changed...
     */

    content_length_to_modify = message;
    message = osip_str_append (message, "     ");
  }
  /*  } */

  message = osip_strn_append (message, CRLF, 2);


  /* end of headers */
  message = osip_strn_append (message, CRLF, 2);

  start_of_bodies = message;
  total_length = start_of_bodies - *dest;

  if (osip_list_eol (&sip->bodies, 0)) {
    /* same remark as at the beginning of the method */
    sip->message_property = 1;
    sip->message = osip_strdup (*dest);
    sip->message_length = total_length;
    if (message_length != NULL)
      *message_length = total_length;

    return OSIP_SUCCESS;        /* it's all done */
  }

  if (sip->mime_version != NULL && sip->content_type && sip->content_type->type && !osip_strcasecmp (sip->content_type->type, "multipart")) {
    osip_generic_param_t *ct_param = NULL;

    /* find the boundary */
    i = osip_generic_param_get_byname (&sip->content_type->gen_params, "boundary", &ct_param);
    if ((i >= 0) && ct_param && ct_param->gvalue) {
      size_t len = strlen (ct_param->gvalue);

      if (len > MIME_MAX_BOUNDARY_LEN) {
        osip_free (*dest);
        *dest = NULL;
        return OSIP_SYNTAXERROR;
      }

      boundary = osip_malloc (len + 5);
      if (boundary == NULL) {
        osip_free (*dest);
        *dest = NULL;
        return OSIP_NOMEM;
      }

      osip_strncpy (boundary, CRLF, 2);
      osip_strncpy (boundary + 2, "--", 2);

      if (ct_param->gvalue[0] == '"' && ct_param->gvalue[len - 1] == '"')
        osip_strncpy (boundary + 4, ct_param->gvalue + 1, len - 2);
      else
        osip_strncpy (boundary + 4, ct_param->gvalue, len);
    }
  }

  {
    osip_list_iterator_t it;
    osip_body_t *body = (osip_body_t *) osip_list_get_first(&sip->bodies, &it);  
    while (body != OSIP_SUCCESS) {
      size_t body_length;

      if (boundary) {
	/* Needs at most 77 bytes,
	   last realloc allocate at least 100 bytes extra */
	message = osip_str_append (message, boundary);
	message = osip_strn_append (message, CRLF, 2);
      }
      
      i = osip_body_to_str (body, &tmp, &body_length);
      if (i != 0) {
	osip_free (*dest);
	*dest = NULL;
	if (boundary)
	  osip_free (boundary);
	return i;
      }
      
      if (malloc_size < message - *dest + 100 + body_length) {
	size_t size = message - *dest;
	int offset_of_body;
	int offset_content_length_to_modify = 0;
	
	offset_of_body = (int) (start_of_bodies - *dest);
	if (content_length_to_modify != NULL)
	  offset_content_length_to_modify = (int) (content_length_to_modify - *dest);
	malloc_size = message - *dest + body_length + 100;
	*dest = osip_realloc (*dest, malloc_size);
	if (*dest == NULL) {
	  osip_free (tmp);        /* fixed 09/Jun/2005 */
	  if (boundary)
	    osip_free (boundary);
	  return OSIP_NOMEM;
	}
	start_of_bodies = *dest + offset_of_body;
	if (content_length_to_modify != NULL)
	  content_length_to_modify = *dest + offset_content_length_to_modify;
	message = *dest + size;
      }
      
      memcpy (message, tmp, body_length);
      message[body_length] = '\0';
      osip_free (tmp);
      message = message + body_length;
      
      body = (osip_body_t *) osip_list_get_next(&it);
    }
  }

  if (boundary) {
    /* Needs at most 79 bytes,
       last realloc allocate at least 100 bytes extra */
    message = osip_str_append (message, boundary);
    message = osip_strn_append (message, "--", 2);
    message = osip_strn_append (message, CRLF, 2);

    osip_free (boundary);
    boundary = NULL;
  }

  if (content_length_to_modify == NULL) {
    osip_free (*dest);
    *dest = NULL;
    return OSIP_SYNTAXERROR;
  }

  /* we NOW have the length of bodies: */
  {
    size_t size = message - start_of_bodies;
    char tmp2[15];

    total_length += size;
    snprintf (tmp2, 15, "%i", (int) size);
    /* do not use osip_strncpy here! */
    strncpy (content_length_to_modify + 5 - strlen (tmp2), tmp2, strlen (tmp2));
  }

  /* same remark as at the beginning of the method */
  sip->message_property = 1;
  sip->message = osip_malloc (total_length + 1);
  if (sip->message != NULL) {
    memcpy (sip->message, *dest, total_length);
    sip->message[total_length] = '\0';
    sip->message_length = total_length;
    if (message_length != NULL)
      *message_length = total_length;
  }
  return OSIP_SUCCESS;
}

int
osip_message_to_str (osip_message_t * sip, char **dest, size_t * message_length)
{
  return _osip_message_to_str (sip, dest, message_length, 0);
}

int
osip_message_to_str_sipfrag (osip_message_t * sip, char **dest, size_t * message_length)
{
  return _osip_message_to_str (sip, dest, message_length, 1);
}
