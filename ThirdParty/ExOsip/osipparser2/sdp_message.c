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

#include <osipparser2/osip_const.h>
#include <osipparser2/sdp_message.h>
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_port.h>

#define ERR_ERROR   -1          /* bad header */
#define ERR_DISCARD  0          /* wrong header */
#define WF           1          /* well formed header */

static int sdp_message_parse_v (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_o (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_s (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_i (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_u (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_e (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_p (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_c (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_b (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_t (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_r (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_z (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_k (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_a (sdp_message_t * sdp, char *buf, char **next);
static int sdp_message_parse_m (sdp_message_t * sdp, char *buf, char **next);

static int sdp_append_media (char **string, int *size, char *tmp, sdp_media_t * media, char **next_tmp);
static int sdp_append_attribute (char **string, int *size, char *tmp, sdp_attribute_t * attribute, char **next_tmp);
static int sdp_append_key (char **string, int *size, char *tmp, sdp_key_t * key, char **next_tmp);
static int sdp_append_time_descr (char **string, int *size, char *tmp, sdp_time_descr_t * time_descr, char **next_tmp);
static int sdp_append_bandwidth (char **string, int *size, char *tmp, sdp_bandwidth_t * bandwidth, char **next_tmp);
static int sdp_append_connection (char **string, int *size, char *tmp, sdp_connection_t * conn, char **next_tmp);

static char *__osip_sdp_append_string (char **string, int *size, char *cur, char *string_osip_to_append);

int
sdp_bandwidth_init (sdp_bandwidth_t ** b)
{
  *b = (sdp_bandwidth_t *) osip_malloc (sizeof (sdp_bandwidth_t));
  if (*b == NULL)
    return OSIP_NOMEM;
  (*b)->b_bwtype = NULL;
  (*b)->b_bandwidth = NULL;
  return OSIP_SUCCESS;
}

void
sdp_bandwidth_free (sdp_bandwidth_t * b)
{
  if (b == NULL)
    return;
  osip_free (b->b_bwtype);
  osip_free (b->b_bandwidth);
  osip_free (b);
}

int
sdp_time_descr_init (sdp_time_descr_t ** td)
{
  *td = (sdp_time_descr_t *) osip_malloc (sizeof (sdp_time_descr_t));
  if (*td == NULL)
    return OSIP_NOMEM;
  (*td)->t_start_time = NULL;
  (*td)->t_stop_time = NULL;
  osip_list_init (&(*td)->r_repeats);
  return OSIP_SUCCESS;
}

void
sdp_time_descr_free (sdp_time_descr_t * td)
{
  if (td == NULL)
    return;
  osip_free (td->t_start_time);
  osip_free (td->t_stop_time);
  osip_list_ofchar_free (&td->r_repeats);
  osip_free (td);
}

int
sdp_key_init (sdp_key_t ** key)
{
  *key = (sdp_key_t *) osip_malloc (sizeof (sdp_key_t));
  if (*key == NULL)
    return OSIP_NOMEM;
  (*key)->k_keytype = NULL;
  (*key)->k_keydata = NULL;
  return OSIP_SUCCESS;
}

void
sdp_key_free (sdp_key_t * key)
{
  if (key == NULL)
    return;
  osip_free (key->k_keytype);
  osip_free (key->k_keydata);
  osip_free (key);
}

int
sdp_attribute_init (sdp_attribute_t ** attribute)
{
  *attribute = (sdp_attribute_t *) osip_malloc (sizeof (sdp_attribute_t));
  if (*attribute == NULL)
    return OSIP_NOMEM;
  (*attribute)->a_att_field = NULL;
  (*attribute)->a_att_value = NULL;
  return OSIP_SUCCESS;
}

void
sdp_attribute_free (sdp_attribute_t * attribute)
{
  if (attribute == NULL)
    return;
  osip_free (attribute->a_att_field);
  osip_free (attribute->a_att_value);
  osip_free (attribute);
}

int
sdp_connection_init (sdp_connection_t ** connection)
{
  *connection = (sdp_connection_t *) osip_malloc (sizeof (sdp_connection_t));
  if (*connection == NULL)
    return OSIP_NOMEM;
  (*connection)->c_nettype = NULL;
  (*connection)->c_addrtype = NULL;
  (*connection)->c_addr = NULL;
  (*connection)->c_addr_multicast_ttl = NULL;
  (*connection)->c_addr_multicast_int = NULL;
  return OSIP_SUCCESS;
}

void
sdp_connection_free (sdp_connection_t * connection)
{
  if (connection == NULL)
    return;
  osip_free (connection->c_nettype);
  osip_free (connection->c_addrtype);
  osip_free (connection->c_addr);
  osip_free (connection->c_addr_multicast_ttl);
  osip_free (connection->c_addr_multicast_int);
  osip_free (connection);
}

int
sdp_media_init (sdp_media_t ** media)
{
  int i;

  *media = (sdp_media_t *) osip_malloc (sizeof (sdp_media_t));
  if (*media == NULL)
    return OSIP_NOMEM;
  (*media)->m_media = NULL;
  (*media)->m_port = NULL;
  (*media)->m_number_of_port = NULL;
  (*media)->m_proto = NULL;
  i = osip_list_init (&(*media)->m_payloads);
  if (i != 0) {
    osip_free (*media);
    *media = NULL;
    return OSIP_NOMEM;
  }
  (*media)->i_info = NULL;
  i = osip_list_init (&(*media)->c_connections);
  if (i != 0) {
    osip_list_ofchar_free (&(*media)->m_payloads);
    osip_free (*media);
    *media = NULL;
    return OSIP_NOMEM;
  }
  i = osip_list_init (&(*media)->b_bandwidths);
  if (i != 0) {
    osip_list_ofchar_free (&(*media)->m_payloads);
    osip_list_special_free (&(*media)->c_connections, (void (*)(void *)) &sdp_connection_free);
    osip_free (*media);
    *media = NULL;
    return OSIP_NOMEM;
  }
  i = osip_list_init (&(*media)->a_attributes);
  if (i != 0) {
    osip_list_ofchar_free (&(*media)->m_payloads);
    osip_list_special_free (&(*media)->c_connections, (void (*)(void *)) &sdp_connection_free);
    osip_list_special_free (&(*media)->b_bandwidths, (void (*)(void *)) &sdp_bandwidth_free);
    osip_free (*media);
    *media = NULL;
    return OSIP_NOMEM;
  }
  (*media)->k_key = NULL;
  return OSIP_SUCCESS;
}

void
sdp_media_free (sdp_media_t * media)
{
  if (media == NULL)
    return;
  osip_free (media->m_media);
  osip_free (media->m_port);
  osip_free (media->m_number_of_port);
  osip_free (media->m_proto);
  osip_list_ofchar_free (&media->m_payloads);
  osip_free (media->i_info);
  osip_list_special_free (&media->c_connections, (void (*)(void *)) &sdp_connection_free);
  osip_list_special_free (&media->b_bandwidths, (void (*)(void *)) &sdp_bandwidth_free);
  osip_list_special_free (&media->a_attributes, (void (*)(void *)) &sdp_attribute_free);
  sdp_key_free (media->k_key);
  osip_free (media);
}

/* to be changed to sdp_message_init(sdp_message_t **dest) */
int
sdp_message_init (sdp_message_t ** sdp)
{
  int i;

  (*sdp) = (sdp_message_t *) osip_malloc (sizeof (sdp_message_t));
  if (*sdp == NULL)
    return OSIP_NOMEM;

  (*sdp)->v_version = NULL;
  (*sdp)->o_username = NULL;
  (*sdp)->o_sess_id = NULL;
  (*sdp)->o_sess_version = NULL;
  (*sdp)->o_nettype = NULL;
  (*sdp)->o_addrtype = NULL;
  (*sdp)->o_addr = NULL;
  (*sdp)->s_name = NULL;
  (*sdp)->i_info = NULL;
  (*sdp)->u_uri = NULL;

  i = osip_list_init (&(*sdp)->e_emails);
  if (i != 0) {
    osip_list_ofchar_free (&(*sdp)->e_emails);
    osip_free (*sdp);
    *sdp = NULL;
    return OSIP_NOMEM;
  }

  i = osip_list_init (&(*sdp)->p_phones);
  if (i != 0) {
    osip_list_ofchar_free (&(*sdp)->e_emails);
    osip_free (*sdp);
    *sdp = NULL;
    return OSIP_NOMEM;
  }

  (*sdp)->c_connection = NULL;

  i = osip_list_init (&(*sdp)->b_bandwidths);
  if (i != 0) {
    osip_list_ofchar_free (&(*sdp)->e_emails);
    osip_list_ofchar_free (&(*sdp)->p_phones);
    osip_free (*sdp);
    *sdp = NULL;
    return OSIP_NOMEM;
  }

  i = osip_list_init (&(*sdp)->t_descrs);
  if (i != 0) {
    osip_list_ofchar_free (&(*sdp)->e_emails);
    osip_list_ofchar_free (&(*sdp)->p_phones);
    osip_list_special_free (&(*sdp)->b_bandwidths, (void (*)(void *)) &sdp_bandwidth_free);
    osip_free (*sdp);
    *sdp = NULL;
    return OSIP_NOMEM;
  }

  (*sdp)->z_adjustments = NULL;
  (*sdp)->k_key = NULL;

  i = osip_list_init (&(*sdp)->a_attributes);
  if (i != 0) {
    osip_list_ofchar_free (&(*sdp)->e_emails);
    osip_list_ofchar_free (&(*sdp)->p_phones);
    osip_list_special_free (&(*sdp)->b_bandwidths, (void (*)(void *)) &sdp_bandwidth_free);
    osip_list_special_free (&(*sdp)->t_descrs, (void (*)(void *)) &sdp_time_descr_free);
    osip_free (*sdp);
    *sdp = NULL;
    return OSIP_NOMEM;
  }

  i = osip_list_init (&(*sdp)->m_medias);
  if (i != 0) {
    osip_list_ofchar_free (&(*sdp)->e_emails);
    osip_list_ofchar_free (&(*sdp)->p_phones);
    osip_list_special_free (&(*sdp)->b_bandwidths, (void (*)(void *)) &sdp_bandwidth_free);
    osip_list_special_free (&(*sdp)->t_descrs, (void (*)(void *)) &sdp_time_descr_free);
    osip_list_special_free (&(*sdp)->a_attributes, (void (*)(void *)) &sdp_attribute_free);
    osip_free (*sdp);
    *sdp = NULL;
    return OSIP_NOMEM;
  }
  return OSIP_SUCCESS;
}

/* append string_osip_to_append to string at position cur
   size is the current allocated size of the element
*/
static char *
__osip_sdp_append_string (char **string, int *size, char *cur, char *string_osip_to_append)
{
  int length = (int) strlen (string_osip_to_append);

  if (cur - (*string) + length +1 > *size) {
    int length2;
    length2 = (int) (cur - *string);
    (*string) = osip_realloc ((*string), *size + length + 500);
    *size = *size + length + 500; /* optimize: avoid too much realloc */
    cur = (*string) + length2;  /* the initial allocation may have changed! */
  }
  osip_strncpy (cur, string_osip_to_append, length);
  return cur + strlen (cur);
}

static int
sdp_message_parse_v (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  if (equal == buf)
    return ERR_DISCARD;

  /* check if header is "v" */
  if (equal[-1] != 'v')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /*v=\r ?? bad header */
  sdp->v_version = osip_malloc (crlf - (equal + 1) + 1);
  if (sdp->v_version == NULL)
    return OSIP_NOMEM;
  osip_strncpy (sdp->v_version, equal + 1, crlf - (equal + 1));

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_o (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;
  char *tmp;
  char *tmp_next;
  int i;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "o" */
  if (equal[-1] != 'o')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* o=\r ?? bad header */

  tmp = equal + 1;
  /* o=username sess-id sess-version nettype addrtype addr */

  /* useranme can contain any char (ascii) except "space" and CRLF */
#ifdef FIREFLY_BUG_SUPPORT
  if (tmp[0] == ' ') {
    sdp->o_username = osip_strdup ("firefly");
    tmp++;
  }
  else {
    i = __osip_set_next_token (&(sdp->o_username), tmp, ' ', &tmp_next);
    if (i != 0)
      return -1;
    tmp = tmp_next;
  }
#else
  i = __osip_set_next_token (&(sdp->o_username), tmp, ' ', &tmp_next);
  if (i != 0)
    return -1;
  tmp = tmp_next;
#endif

  /* sess_id contains only numeric characters */
  i = __osip_set_next_token (&(sdp->o_sess_id), tmp, ' ', &tmp_next);
  if (i != 0)
    return -1;
  tmp = tmp_next;

  /* sess_id contains only numeric characters */
  i = __osip_set_next_token (&(sdp->o_sess_version), tmp, ' ', &tmp_next);
  if (i != 0)
    return -1;
  tmp = tmp_next;

  /* nettype is "IN" but will surely be extented!!! assume it's some alpha-char */
  i = __osip_set_next_token (&(sdp->o_nettype), tmp, ' ', &tmp_next);
  if (i != 0)
    return -1;
  tmp = tmp_next;

  /* addrtype  is "IP4" or "IP6" but will surely be extented!!! */
  i = __osip_set_next_token (&(sdp->o_addrtype), tmp, ' ', &tmp_next);
  if (i != 0)
    return -1;
  tmp = tmp_next;

  /* addr  is "IP4" or "IP6" but will surely be extented!!! */
  i = __osip_set_next_token (&(sdp->o_addr), tmp, '\r', &tmp_next);
  if (i != 0) {                 /* could it be "\n" only??? rfc says to accept CR or LF instead of CRLF */
    i = __osip_set_next_token (&(sdp->o_addr), tmp, '\n', &tmp_next);
    if (i != 0)
      return -1;
  }

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_s (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "s" */
  if (equal[-1] != 's')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
#ifdef FIREFLY_BUG_SUPPORT
  if (crlf == equal + 1) {
    sdp->s_name = osip_strdup (" ");
    if (crlf[1] == '\n')
      *next = crlf + 2;
    else
      *next = crlf + 1;
    return WF;                  /* o=\r ?? bad header */
  }
#else
  if (crlf == equal + 1)
    return ERR_ERROR;           /* o=\r ?? bad header */
#endif

  /* s=text */

  /* text is interpreted as ISO-10646 UTF8! */
  /* using ISO 8859-1 requires "a=charset:ISO-8859-1 */
  sdp->s_name = osip_malloc (crlf - (equal + 1) + 1);
  if (sdp->s_name == NULL)
    return OSIP_NOMEM;
  osip_strncpy (sdp->s_name, equal + 1, crlf - (equal + 1));

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_i (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;
  int i;
  char *i_info;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "i" */
  if (equal[-1] != 'i')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* o=\r ?? bad header */

  /* s=text */

  /* text is interpreted as ISO-10646 UTF8! */
  /* using ISO 8859-1 requires "a=charset:ISO-8859-1 */
  i_info = osip_malloc (crlf - (equal + 1) + 1);
  if (i_info == NULL)
    return OSIP_NOMEM;
  osip_strncpy (i_info, equal + 1, crlf - (equal + 1));

  /* add the bandwidth at the correct place:
     if there is no media line yet, then the "b=" is the
     global one.
   */
  i = osip_list_size (&sdp->m_medias);
  if (i == 0)
    sdp->i_info = i_info;
  else {
    sdp_media_t *last_sdp_media = (sdp_media_t *) osip_list_get (&sdp->m_medias, i - 1);

    last_sdp_media->i_info = i_info;
  }

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_u (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "u" */
  if (equal[-1] != 'u')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* u=\r ?? bad header */

  /* u=uri */
  /* we assume this is a URI */
  sdp->u_uri = osip_malloc (crlf - (equal + 1) + 1);
  if (sdp->u_uri == NULL)
    return OSIP_NOMEM;
  osip_strncpy (sdp->u_uri, equal + 1, crlf - (equal + 1));

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_e (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;
  char *e_email;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "e" */
  if (equal[-1] != 'e')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* e=\r ?? bad header */

  /* e=email */
  /* we assume this is an EMAIL-ADDRESS */
  e_email = osip_malloc (crlf - (equal + 1) + 1);
  if (e_email == NULL)
    return OSIP_NOMEM;
  osip_strncpy (e_email, equal + 1, crlf - (equal + 1));

  osip_list_add (&sdp->e_emails, e_email, -1);

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_p (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;
  char *p_phone;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "p" */
  if (equal[-1] != 'p')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* p=\r ?? bad header */

  /* e=email */
  /* we assume this is an EMAIL-ADDRESS */
  p_phone = osip_malloc (crlf - (equal + 1) + 1);
  if (p_phone == NULL)
    return OSIP_NOMEM;
  osip_strncpy (p_phone, equal + 1, crlf - (equal + 1));

  osip_list_add (&sdp->p_phones, p_phone, -1);

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_c (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;
  char *tmp;
  char *tmp_next;
  sdp_connection_t *c_header;
  int i;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "c" */
  if (equal[-1] != 'c')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* c=\r ?? bad header */

  tmp = equal + 1;
  i = sdp_connection_init (&c_header);
  if (i != 0)
    return ERR_ERROR;
  /* c=nettype addrtype (multicastaddr | addr) */

  /* nettype is "IN" and will be extended */
  i = __osip_set_next_token (&(c_header->c_nettype), tmp, ' ', &tmp_next);
  if (i != 0) {
    sdp_connection_free (c_header);
    return -1;
  }
  tmp = tmp_next;

  /* nettype is "IP4" or "IP6" and will be extended */
  i = __osip_set_next_token (&(c_header->c_addrtype), tmp, ' ', &tmp_next);
  if (i != 0) {
    sdp_connection_free (c_header);
    return -1;
  }
  tmp = tmp_next;

  /* there we have a multicast or unicast address */
  /* multicast can be ip/ttl [/integer] */
  /* unicast is FQDN or ip (no ttl, no integer) */

  /* is MULTICAST? */
  {
    char *slash = strchr (tmp, '/');

    if (slash != NULL && slash < crlf) {        /* it's a multicast address! */
      i = __osip_set_next_token (&(c_header->c_addr), tmp, '/', &tmp_next);
      if (i != 0) {
        sdp_connection_free (c_header);
        return -1;
      }
      tmp = tmp_next;
      slash = strchr (slash + 1, '/');
      if (slash != NULL && slash < crlf) {      /* optionnal integer is there! */
        i = __osip_set_next_token (&(c_header->c_addr_multicast_ttl), tmp, '/', &tmp_next);
        if (i != 0) {
          sdp_connection_free (c_header);
          return -1;
        }
        tmp = tmp_next;
        i = __osip_set_next_token (&(c_header->c_addr_multicast_int), tmp, '\r', &tmp_next);
        if (i != 0) {
          i = __osip_set_next_token (&(c_header->c_addr_multicast_int), tmp, '\n', &tmp_next);
          if (i != 0) {
            sdp_connection_free (c_header);
            return -1;
          }
        }
      }
      else {
        i = __osip_set_next_token (&(c_header->c_addr_multicast_ttl), tmp, '\r', &tmp_next);
        if (i != 0) {
          i = __osip_set_next_token (&(c_header->c_addr_multicast_ttl), tmp, '\n', &tmp_next);
          if (i != 0) {
            sdp_connection_free (c_header);
            return -1;
          }
        }
      }
    }
    else {
      /* in this case, we have a unicast address */
      i = __osip_set_next_token (&(c_header->c_addr), tmp, '\r', &tmp_next);
      if (i != 0) {
        i = __osip_set_next_token (&(c_header->c_addr), tmp, '\n', &tmp_next);
        if (i != 0) {
          sdp_connection_free (c_header);
          return -1;
        }
      }
    }
  }

  /* add the connection at the correct place:
     if there is no media line yet, then the "c=" is the
     global one.
   */
  i = osip_list_size (&sdp->m_medias);
  if (i == 0)
    sdp->c_connection = c_header;
  else {
    sdp_media_t *last_sdp_media = (sdp_media_t *) osip_list_get (&sdp->m_medias, i - 1);

    osip_list_add (&last_sdp_media->c_connections, c_header, -1);
  }
  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_b (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;
  char *tmp;
  char *tmp_next;
  int i;
  sdp_bandwidth_t *b_header;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "b" */
  if (equal[-1] != 'b')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* b=\r ?? bad header */

  tmp = equal + 1;
  /* b = bwtype: bandwidth */
  i = sdp_bandwidth_init (&b_header);
  if (i != 0)
    return ERR_ERROR;

  /* bwtype is alpha-numeric */
  i = __osip_set_next_token (&(b_header->b_bwtype), tmp, ':', &tmp_next);
  if (i != 0) {
    sdp_bandwidth_free (b_header);
    return -1;
  }
  tmp = tmp_next;

  i = __osip_set_next_token (&(b_header->b_bandwidth), tmp, '\r', &tmp_next);
  if (i != 0) {
    i = __osip_set_next_token (&(b_header->b_bandwidth), tmp, '\n', &tmp_next);
    if (i != 0) {
      sdp_bandwidth_free (b_header);
      return -1;
    }
  }

  /* add the bandwidth at the correct place:
     if there is no media line yet, then the "b=" is the
     global one.
   */
  i = osip_list_size (&sdp->m_medias);
  if (i == 0)
    osip_list_add (&sdp->b_bandwidths, b_header, -1);
  else {
    sdp_media_t *last_sdp_media = (sdp_media_t *) osip_list_get (&sdp->m_medias, i - 1);

    osip_list_add (&last_sdp_media->b_bandwidths, b_header, -1);
  }

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_t (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;
  char *tmp;
  char *tmp_next;
  int i;
  sdp_time_descr_t *t_header;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "t" */
  if (equal[-1] != 't')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* t=\r ?? bad header */

  tmp = equal + 1;
  /* t = start_time stop_time */
  i = sdp_time_descr_init (&t_header);
  if (i != 0)
    return ERR_ERROR;

  i = __osip_set_next_token (&(t_header->t_start_time), tmp, ' ', &tmp_next);
  if (i != 0) {
    sdp_time_descr_free (t_header);
    return -1;
  }
  tmp = tmp_next;

  i = __osip_set_next_token (&(t_header->t_stop_time), tmp, '\r', &tmp_next);
  if (i != 0) {
    i = __osip_set_next_token (&(t_header->t_stop_time), tmp, '\n', &tmp_next);
    if (i != 0) {
      sdp_time_descr_free (t_header);
      return -1;
    }
  }

  /* add the new time_description header */
  osip_list_add (&sdp->t_descrs, t_header, -1);

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_r (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;
  int index;
  char *r_header;
  sdp_time_descr_t *t_descr;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "r" */
  if (equal[-1] != 'r')
    return ERR_DISCARD;

  index = osip_list_size (&sdp->t_descrs);
  if (index == 0)
    return ERR_ERROR;           /* r field can't come alone! */

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* r=\r ?? bad header */

  /* r=far too complexe and somewhat useless... I don't parse it! */
  r_header = osip_malloc (crlf - (equal + 1) + 1);
  if (r_header == NULL)
    return OSIP_NOMEM;
  osip_strncpy (r_header, equal + 1, crlf - (equal + 1));

  /* r field carry information for the last "t" field */
  t_descr = (sdp_time_descr_t *) osip_list_get (&sdp->t_descrs, index - 1);
  osip_list_add (&t_descr->r_repeats, r_header, -1);

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_z (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;
  char *z_header;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "z" */
  if (equal[-1] != 'z')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* z=\r ?? bad header */

  /* z=somewhat useless... I don't parse it! */
  z_header = osip_malloc (crlf - (equal + 1) + 1);
  if (z_header == NULL)
    return OSIP_NOMEM;
  osip_strncpy (z_header, equal + 1, crlf - (equal + 1));

  sdp->z_adjustments = z_header;

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_k (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;
  int i;
  char *colon;
  sdp_key_t *k_header;
  char *tmp;
  char *tmp_next;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "k" */
  if (equal[-1] != 'k')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* k=\r ?? bad header */

  tmp = equal + 1;

  i = sdp_key_init (&k_header);
  if (i != 0)
    return ERR_ERROR;
  /* k=key-type[:key-data] */

  /* is there any key-data? */
  colon = strchr (equal + 1, ':');
  if ((colon != NULL) && (colon < crlf)) {
    /* att-field is alpha-numeric */
    i = __osip_set_next_token (&(k_header->k_keytype), tmp, ':', &tmp_next);
    if (i != 0) {
      sdp_key_free (k_header);
      return -1;
    }
    tmp = tmp_next;

    i = __osip_set_next_token (&(k_header->k_keydata), tmp, '\r', &tmp_next);
    if (i != 0) {
      i = __osip_set_next_token (&(k_header->k_keydata), tmp, '\n', &tmp_next);
      if (i != 0) {
        sdp_key_free (k_header);
        return -1;
      }
    }
  }
  else {
    i = __osip_set_next_token (&(k_header->k_keytype), tmp, '\r', &tmp_next);
    if (i != 0) {
      i = __osip_set_next_token (&(k_header->k_keytype), tmp, '\n', &tmp_next);
      if (i != 0) {
        sdp_key_free (k_header);
        return -1;
      }
    }
  }

  /* add the key at the correct place:
     if there is no media line yet, then the "k=" is the
     global one.
   */
  i = osip_list_size (&sdp->m_medias);
  if (i == 0)
    sdp->k_key = k_header;
  else {
    sdp_media_t *last_sdp_media = (sdp_media_t *) osip_list_get (&sdp->m_medias, i - 1);

    last_sdp_media->k_key = k_header;
  }

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_a (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;
  char *tmp;
  char *tmp_next;
  int i;
  sdp_attribute_t *a_attribute;
  char *colon;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "a" */
  if (equal[-1] != 'a')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* a=\r ?? bad header */

  tmp = equal + 1;

  i = sdp_attribute_init (&a_attribute);
  if (i != 0)
    return ERR_ERROR;

  /* a=att-field[:att-value] */

  /* is there any att-value? */
  colon = strchr (equal + 1, ':');
  if ((colon != NULL) && (colon < crlf)) {
    /* att-field is alpha-numeric */
    i = __osip_set_next_token (&(a_attribute->a_att_field), tmp, ':', &tmp_next);
    if (i != 0) {
      sdp_attribute_free (a_attribute);
      return -1;
    }
    tmp = tmp_next;

    i = __osip_set_next_token (&(a_attribute->a_att_value), tmp, '\r', &tmp_next);
    if (i != 0) {
      i = __osip_set_next_token (&(a_attribute->a_att_value), tmp, '\n', &tmp_next);
      if (i != 0) {
        sdp_attribute_free (a_attribute);
        return -1;
      }
    }
  }
  else {
    i = __osip_set_next_token (&(a_attribute->a_att_field), tmp, '\r', &tmp_next);
    if (i != 0) {
      i = __osip_set_next_token (&(a_attribute->a_att_field), tmp, '\n', &tmp_next);
      if (i != 0) {
        sdp_attribute_free (a_attribute);
        return -1;
      }
    }
  }

  /* add the attribute at the correct place:
     if there is no media line yet, then the "a=" is the
     global one.
   */
  i = osip_list_size (&sdp->m_medias);
  if (i == 0)
    osip_list_add (&sdp->a_attributes, a_attribute, -1);
  else {
    sdp_media_t *last_sdp_media = (sdp_media_t *) osip_list_get (&sdp->m_medias, i - 1);

    osip_list_add (&last_sdp_media->a_attributes, a_attribute, -1);
  }

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}

static int
sdp_message_parse_m (sdp_message_t * sdp, char *buf, char **next)
{
  char *equal;
  char *crlf;
  char *tmp;
  char *tmp_next;
  int i;
  sdp_media_t *m_header;
  char *slash;
  char *space;

  *next = buf;

  equal = buf;
  while ((*equal != '=') && (*equal != '\0'))
    equal++;
  if (*equal == '\0')
    return ERR_ERROR;

  /* check if header is "m" */
  if (equal[-1] != 'm')
    return ERR_DISCARD;

  crlf = equal + 1;

  while ((*crlf != '\r') && (*crlf != '\n') && (*crlf != '\0'))
    crlf++;
  if (*crlf == '\0')
    return ERR_ERROR;
  if (crlf == equal + 1)
    return ERR_ERROR;           /* a=\r ?? bad header */

  tmp = equal + 1;

  i = sdp_media_init (&m_header);
  if (i != 0)
    return ERR_ERROR;

  /* m=media port ["/"integer] proto *(payload_number) */

  /* media is "audio" "video" "application" "data" or other... */
  i = __osip_set_next_token (&(m_header->m_media), tmp, ' ', &tmp_next);
  if (i != 0) {
    sdp_media_free (m_header);
    return -1;
  }
  tmp = tmp_next;

  slash = strchr (tmp, '/');
  space = strchr (tmp, ' ');
  if (space == NULL) {          /* not possible! */
    sdp_media_free (m_header);
    return ERR_ERROR;
  }
  if ((slash != NULL) && (slash < space)) {     /* a number of port is specified! */
    i = __osip_set_next_token (&(m_header->m_port), tmp, '/', &tmp_next);
    if (i != 0) {
      sdp_media_free (m_header);
      return -1;
    }
    tmp = tmp_next;

    i = __osip_set_next_token (&(m_header->m_number_of_port), tmp, ' ', &tmp_next);
    if (i != 0) {
      sdp_media_free (m_header);
      return -1;
    }
    tmp = tmp_next;
  }
  else {
    i = __osip_set_next_token (&(m_header->m_port), tmp, ' ', &tmp_next);
    if (i != 0) {
      sdp_media_free (m_header);
      return -1;
    }
    tmp = tmp_next;
  }

  i = __osip_set_next_token (&(m_header->m_proto), tmp, ' ', &tmp_next);
  if (i != 0) {
    /* a few stack don't add SPACE after m_proto when rejecting all payloads */
    i = __osip_set_next_token (&(m_header->m_proto), tmp, '\r', &tmp_next);
    if (i != 0) {
      i = __osip_set_next_token (&(m_header->m_proto), tmp, '\n', &tmp_next);
      if (i != 0) {
        sdp_media_free (m_header);
        return -1;
      }
    }
  }
  tmp = tmp_next;

  {
    char *str;
    int more_space_before_crlf;

    space = strchr (tmp + 1, ' ');
    if (space == NULL)
      more_space_before_crlf = 1;
    else if ((space != NULL) && (space > crlf))
      more_space_before_crlf = 1;
    else
      more_space_before_crlf = 0;
    while (more_space_before_crlf == 0) {
      i = __osip_set_next_token (&str, tmp, ' ', &tmp_next);
      if (i != 0) {
        sdp_media_free (m_header);
        return -1;
      }
      tmp = tmp_next;
      osip_list_add (&m_header->m_payloads, str, -1);

      space = strchr (tmp + 1, ' ');
      if (space == NULL)
        more_space_before_crlf = 1;
      else if ((space != NULL) && (space > crlf))
        more_space_before_crlf = 1;
      else
        more_space_before_crlf = 0;
    }
    if (tmp_next < crlf) {      /* tmp_next is still less than clrf: no space */
      i = __osip_set_next_token (&str, tmp, '\r', &tmp_next);
      if (i != 0) {
        i = __osip_set_next_token (&str, tmp, '\n', &tmp_next);
        if (i != 0) {
          sdp_media_free (m_header);
          return -1;
        }
      }
      osip_list_add (&m_header->m_payloads, str, -1);
    }
  }

  osip_list_add (&sdp->m_medias, m_header, -1);

  if (crlf[1] == '\n')
    *next = crlf + 2;
  else
    *next = crlf + 1;
  return WF;
}


int
sdp_message_parse (sdp_message_t * sdp, const char *buf)
{

  /* In SDP, headers must be in the right order */
  /* This is a simple example
     v=0
     o=user1 53655765 2353687637 IN IP4 128.3.4.5
     s=Mbone Audio
     i=Discussion of Mbone Engineering Issues
     e=mbone@somewhere.com
     c=IN IP4 224.2.0.1/127
     t=0 0
     m=audio 3456 RTP/AVP 0
     a=rtpmap:0 PCMU/8000
   */

  char *next_buf;
  char *ptr;
  int i;

  ptr = (char *) buf;
  /* mandatory */
  i = sdp_message_parse_v (sdp, ptr, &next_buf);
  if (i == -1)                  /* header is bad */
    return -1;
  else if (0 == i)              /* header is not "v" */
    return -1;
  ptr = next_buf;

  /* adtech phone use the wrong ordering and place "s" before "o" */
  i = sdp_message_parse_s (sdp, ptr, &next_buf);
  if (i == -1)                  /* header is bad */
    return -1;
  /* else if (0==i) header is not "s" */
  /* else ADTECH PHONE DETECTED */

  ptr = next_buf;



  i = sdp_message_parse_o (sdp, ptr, &next_buf);
  if (i == -1)                  /* header is bad */
    return -1;
  else if (0 == i)              /* header is not "o" */
    return -1;
  ptr = next_buf;

  i = sdp_message_parse_s (sdp, ptr, &next_buf);
  if (i == -1)                  /* header is bad */
    return -1;
  else if (0 == i) {            /* header is not "s" */
    /* return -1; */
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO4, NULL, "The \"s\" parameter is mandatory, but this packet does not contain any! - anyway, we don't mind about it.\n"));
  }
  ptr = next_buf;

  i = sdp_message_parse_i (sdp, ptr, &next_buf);
  if (i == -1)                  /* header is bad */
    return -1;
  ptr = next_buf;

  i = sdp_message_parse_u (sdp, ptr, &next_buf);
  if (i == -1)                  /* header is bad */
    return -1;
  ptr = next_buf;

  i = 1;
  while (i == 1) {
    i = sdp_message_parse_e (sdp, ptr, &next_buf);
    if (i == -1)                /* header is bad */
      return -1;
    ptr = next_buf;
  }
  i = 1;
  while (i == 1) {
    i = sdp_message_parse_p (sdp, ptr, &next_buf);
    if (i == -1)                /* header is bad */
      return -1;
    ptr = next_buf;
  }

  /* rfc2327: there should be at least of email or phone number! */
  if (osip_list_size (&sdp->e_emails) == 0 && osip_list_size (&sdp->p_phones) == 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO4, NULL, "The rfc2327 says there should be at least an email or a phone header!- anyway, we don't mind about it.\n"));
  }

  i = sdp_message_parse_c (sdp, ptr, &next_buf);
  if (i == -1)                  /* header is bad */
    return -1;
  ptr = next_buf;

  i = 1;
  while (i == 1) {
    i = sdp_message_parse_b (sdp, ptr, &next_buf);
    if (i == -1)                /* header is bad */
      return -1;
    ptr = next_buf;
  }

  /* 1 or more "t" header + 0 or more "r" header for each "t" header */
  i = sdp_message_parse_t (sdp, ptr, &next_buf);
  if (i == -1)                  /* header is bad */
    return -1;
  else if (i == ERR_DISCARD)
    return -1;                  /* t is mandatory */
  ptr = next_buf;

  if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
    return OSIP_SUCCESS;

  i = 1;
  while (i == 1) {              /* is a "r" header */
    i = sdp_message_parse_r (sdp, ptr, &next_buf);
    if (i == -1)                /* header is bad */
      return -1;
    ptr = next_buf;
    if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
      return OSIP_SUCCESS;

  }


  {
    int more_t_header = 1;

    i = sdp_message_parse_t (sdp, ptr, &next_buf);
    if (i == -1)                /* header is bad */
      return -1;
    ptr = next_buf;

    if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
      return OSIP_SUCCESS;

    while (more_t_header == 1) {
      i = 1;
      while (i == 1) {          /* is a "r" header */
        i = sdp_message_parse_r (sdp, ptr, &next_buf);
        if (i == -1)            /* header is bad */
          return -1;
        ptr = next_buf;
        if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
          return OSIP_SUCCESS;
      }

      i = sdp_message_parse_t (sdp, ptr, &next_buf);
      if (i == -1)              /* header is bad */
        return -1;
      else if (i == ERR_DISCARD)
        more_t_header = 0;
      else
        more_t_header = 1;      /* no more "t" headers */
      ptr = next_buf;
      if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
        return OSIP_SUCCESS;
    }
  }

  i = sdp_message_parse_z (sdp, ptr, &next_buf);
  if (i == -1)                  /* header is bad */
    return -1;
  ptr = next_buf;
  if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
    return OSIP_SUCCESS;

  i = sdp_message_parse_k (sdp, ptr, &next_buf);
  if (i == -1)                  /* header is bad */
    return -1;
  ptr = next_buf;
  if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
    return OSIP_SUCCESS;

  /* 0 or more "a" header */
  i = 1;
  while (i == 1) {              /* no more "a" header */
    i = sdp_message_parse_a (sdp, ptr, &next_buf);
    if (i == -1)                /* header is bad */
      return -1;
    ptr = next_buf;
    if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
      return OSIP_SUCCESS;
  }
  /* 0 or more media headers */
  {
    int more_m_header = 1;

    while (more_m_header == 1) {
      more_m_header = sdp_message_parse_m (sdp, ptr, &next_buf);
      if (more_m_header == -1)  /* header is bad */
        return -1;
      ptr = next_buf;
      if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
        return OSIP_SUCCESS;

      i = sdp_message_parse_i (sdp, ptr, &next_buf);
      if (i == -1)              /* header is bad */
        return -1;
      ptr = next_buf;
      if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
        return OSIP_SUCCESS;

      i = 1;
      while (i == 1) {
        i = sdp_message_parse_c (sdp, ptr, &next_buf);
        if (i == -1)            /* header is bad */
          return -1;
        ptr = next_buf;
        if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
          return OSIP_SUCCESS;
      }

      i = 1;
      while (i == 1) {
        i = sdp_message_parse_b (sdp, ptr, &next_buf);
        if (i == -1)            /* header is bad */
          return -1;
        ptr = next_buf;
        if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
          return OSIP_SUCCESS;
      }
      i = sdp_message_parse_k (sdp, ptr, &next_buf);
      if (i == -1)              /* header is bad */
        return -1;
      ptr = next_buf;
      if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
        return OSIP_SUCCESS;
      /* 0 or more a headers */
      i = 1;
      while (i == 1) {
        i = sdp_message_parse_a (sdp, ptr, &next_buf);
        if (i == -1)            /* header is bad */
          return -1;
        ptr = next_buf;
        if (*ptr == '\0' || (*ptr == '\r') || (*ptr == '\n'))
          return OSIP_SUCCESS;
      }
    }
  }

  return OSIP_SUCCESS;
}

static int
sdp_append_connection (char **string, int *size, char *tmp, sdp_connection_t * conn, char **next_tmp)
{
  if (conn->c_nettype == NULL)
    return -1;
  if (conn->c_addrtype == NULL)
    return -1;
  if (conn->c_addr == NULL)
    return -1;

  tmp = __osip_sdp_append_string (string, size, tmp, "c=");
  tmp = __osip_sdp_append_string (string, size, tmp, conn->c_nettype);
  tmp = __osip_sdp_append_string (string, size, tmp, " ");
  tmp = __osip_sdp_append_string (string, size, tmp, conn->c_addrtype);
  tmp = __osip_sdp_append_string (string, size, tmp, " ");
  tmp = __osip_sdp_append_string (string, size, tmp, conn->c_addr);
  if (conn->c_addr_multicast_ttl != NULL) {
    tmp = __osip_sdp_append_string (string, size, tmp, "/");
    tmp = __osip_sdp_append_string (string, size, tmp, conn->c_addr_multicast_ttl);
  }
  if (conn->c_addr_multicast_int != NULL) {
    tmp = __osip_sdp_append_string (string, size, tmp, "/");
    tmp = __osip_sdp_append_string (string, size, tmp, conn->c_addr_multicast_int);
  }
  tmp = __osip_sdp_append_string (string, size, tmp, CRLF);
  *next_tmp = tmp;
  return OSIP_SUCCESS;
}

static int
sdp_append_bandwidth (char **string, int *size, char *tmp, sdp_bandwidth_t * bandwidth, char **next_tmp)
{
  if (bandwidth->b_bwtype == NULL)
    return -1;
  if (bandwidth->b_bandwidth == NULL)
    return -1;

  tmp = __osip_sdp_append_string (string, size, tmp, "b=");
  tmp = __osip_sdp_append_string (string, size, tmp, bandwidth->b_bwtype);
  tmp = __osip_sdp_append_string (string, size, tmp, ":");
  tmp = __osip_sdp_append_string (string, size, tmp, bandwidth->b_bandwidth);
  tmp = __osip_sdp_append_string (string, size, tmp, CRLF);

  *next_tmp = tmp;
  return OSIP_SUCCESS;
}

static int
sdp_append_time_descr (char **string, int *size, char *tmp, sdp_time_descr_t * time_descr, char **next_tmp)
{
  int pos;

  if (time_descr->t_start_time == NULL)
    return -1;
  if (time_descr->t_stop_time == NULL)
    return -1;


  tmp = __osip_sdp_append_string (string, size, tmp, "t=");
  tmp = __osip_sdp_append_string (string, size, tmp, time_descr->t_start_time);
  tmp = __osip_sdp_append_string (string, size, tmp, " ");
  tmp = __osip_sdp_append_string (string, size, tmp, time_descr->t_stop_time);

  tmp = __osip_sdp_append_string (string, size, tmp, CRLF);

  pos = 0;
  while (!osip_list_eol (&time_descr->r_repeats, pos)) {
    char *str = (char *) osip_list_get (&time_descr->r_repeats, pos);

    tmp = __osip_sdp_append_string (string, size, tmp, "r=");
    tmp = __osip_sdp_append_string (string, size, tmp, str);
    tmp = __osip_sdp_append_string (string, size, tmp, CRLF);
    pos++;
  }

  *next_tmp = tmp;
  return OSIP_SUCCESS;
}

static int
sdp_append_key (char **string, int *size, char *tmp, sdp_key_t * key, char **next_tmp)
{
  if (key->k_keytype == NULL)
    return -1;

  tmp = __osip_sdp_append_string (string, size, tmp, "k=");
  tmp = __osip_sdp_append_string (string, size, tmp, key->k_keytype);
  if (key->k_keydata != NULL) {
    tmp = __osip_sdp_append_string (string, size, tmp, ":");
    tmp = __osip_sdp_append_string (string, size, tmp, key->k_keydata);
  }
  tmp = __osip_sdp_append_string (string, size, tmp, CRLF);
  *next_tmp = tmp;
  return OSIP_SUCCESS;
}

static int
sdp_append_attribute (char **string, int *size, char *tmp, sdp_attribute_t * attribute, char **next_tmp)
{
  if (attribute->a_att_field == NULL)
    return -1;

  tmp = __osip_sdp_append_string (string, size, tmp, "a=");
  tmp = __osip_sdp_append_string (string, size, tmp, attribute->a_att_field);
  if (attribute->a_att_value != NULL) {
    tmp = __osip_sdp_append_string (string, size, tmp, ":");
    tmp = __osip_sdp_append_string (string, size, tmp, attribute->a_att_value);
  }
  tmp = __osip_sdp_append_string (string, size, tmp, CRLF);

  *next_tmp = tmp;
  return OSIP_SUCCESS;
}

/* internal facility */
static int
sdp_append_media (char **string, int *size, char *tmp, sdp_media_t * media, char **next_tmp)
{
  int pos;

  if (media->m_media == NULL)
    return -1;
  if (media->m_port == NULL)
    return -1;
  if (media->m_proto == NULL)
    return -1;

  tmp = __osip_sdp_append_string (string, size, tmp, "m=");
  tmp = __osip_sdp_append_string (string, size, tmp, media->m_media);
  tmp = __osip_sdp_append_string (string, size, tmp, " ");
  tmp = __osip_sdp_append_string (string, size, tmp, media->m_port);
  if (media->m_number_of_port != NULL) {
    tmp = __osip_sdp_append_string (string, size, tmp, "/");
    tmp = __osip_sdp_append_string (string, size, tmp, media->m_number_of_port);
  }
  tmp = __osip_sdp_append_string (string, size, tmp, " ");
  tmp = __osip_sdp_append_string (string, size, tmp, media->m_proto);
  pos = 0;
  while (!osip_list_eol (&media->m_payloads, pos)) {
    char *str = (char *) osip_list_get (&media->m_payloads, pos);

    tmp = __osip_sdp_append_string (string, size, tmp, " ");
    tmp = __osip_sdp_append_string (string, size, tmp, str);
    pos++;
  }
  tmp = __osip_sdp_append_string (string, size, tmp, CRLF);

  if (media->i_info != NULL) {
    tmp = __osip_sdp_append_string (string, size, tmp, "i=");
    tmp = __osip_sdp_append_string (string, size, tmp, media->i_info);
    tmp = __osip_sdp_append_string (string, size, tmp, CRLF);
  }

  pos = 0;
  while (!osip_list_eol (&media->c_connections, pos)) {
    sdp_connection_t *conn = (sdp_connection_t *) osip_list_get (&media->c_connections, pos);
    char *next_tmp2;
    int i;

    i = sdp_append_connection (string, size, tmp, conn, &next_tmp2);
    if (i != 0)
      return -1;
    tmp = next_tmp2;
    pos++;
  }

  pos = 0;
  while (!osip_list_eol (&media->b_bandwidths, pos)) {
    sdp_bandwidth_t *band = (sdp_bandwidth_t *) osip_list_get (&media->b_bandwidths, pos);
    char *next_tmp2;
    int i;

    i = sdp_append_bandwidth (string, size, tmp, band, &next_tmp2);
    if (i != 0)
      return -1;
    tmp = next_tmp2;
    pos++;
  }

  if (media->k_key != NULL) {
    char *next_tmp2;
    int i;

    i = sdp_append_key (string, size, tmp, media->k_key, &next_tmp2);
    if (i != 0)
      return -1;
    tmp = next_tmp2;
  }

  pos = 0;
  while (!osip_list_eol (&media->a_attributes, pos)) {
    sdp_attribute_t *attr = (sdp_attribute_t *) osip_list_get (&media->a_attributes, pos);
    char *next_tmp2;
    int i;

    i = sdp_append_attribute (string, size, tmp, attr, &next_tmp2);
    if (i != 0)
      return -1;
    tmp = next_tmp2;
    pos++;
  }

  *next_tmp = tmp;
  return OSIP_SUCCESS;
}

int
sdp_message_to_str (sdp_message_t * sdp, char **dest)
{
  int size;
  int pos;
  char *tmp;
  char *string;

  *dest = NULL;
  if (!sdp || sdp->v_version == NULL)
    return -1;
  if (sdp->o_username == NULL || sdp->o_sess_id == NULL || sdp->o_sess_version == NULL || sdp->o_nettype == NULL || sdp->o_addrtype == NULL || sdp->o_addr == NULL)
    return -1;

  /* RFC says "s=" is mandatory... rfc2543 (SIP) recommends to
     accept SDP datas without s_name... as some buggy implementations
     often forget it...
   */
  /* if (sdp->s_name == NULL)
     return -1; */

  size = BODY_MESSAGE_MAX_SIZE;
  tmp = (char *) osip_malloc (size);
  if (tmp == NULL)
    return OSIP_NOMEM;
  string = tmp;

  tmp = __osip_sdp_append_string (&string, &size, tmp, "v=");
  tmp = __osip_sdp_append_string (&string, &size, tmp, sdp->v_version);
  tmp = __osip_sdp_append_string (&string, &size, tmp, CRLF);
  tmp = __osip_sdp_append_string (&string, &size, tmp, "o=");
  tmp = __osip_sdp_append_string (&string, &size, tmp, sdp->o_username);
  tmp = __osip_sdp_append_string (&string, &size, tmp, " ");
  tmp = __osip_sdp_append_string (&string, &size, tmp, sdp->o_sess_id);
  tmp = __osip_sdp_append_string (&string, &size, tmp, " ");
  tmp = __osip_sdp_append_string (&string, &size, tmp, sdp->o_sess_version);
  tmp = __osip_sdp_append_string (&string, &size, tmp, " ");
  tmp = __osip_sdp_append_string (&string, &size, tmp, sdp->o_nettype);
  tmp = __osip_sdp_append_string (&string, &size, tmp, " ");
  tmp = __osip_sdp_append_string (&string, &size, tmp, sdp->o_addrtype);
  tmp = __osip_sdp_append_string (&string, &size, tmp, " ");
  tmp = __osip_sdp_append_string (&string, &size, tmp, sdp->o_addr);
  tmp = __osip_sdp_append_string (&string, &size, tmp, CRLF);
  if (sdp->s_name != NULL) {
    tmp = __osip_sdp_append_string (&string, &size, tmp, "s=");
    tmp = __osip_sdp_append_string (&string, &size, tmp, sdp->s_name);
    tmp = __osip_sdp_append_string (&string, &size, tmp, CRLF);
  }
  if (sdp->i_info != NULL) {
    tmp = __osip_sdp_append_string (&string, &size, tmp, "i=");
    tmp = __osip_sdp_append_string (&string, &size, tmp, sdp->i_info);
    tmp = __osip_sdp_append_string (&string, &size, tmp, CRLF);
  }
  if (sdp->u_uri != NULL) {
    tmp = __osip_sdp_append_string (&string, &size, tmp, "u=");
    tmp = __osip_sdp_append_string (&string, &size, tmp, sdp->u_uri);
    tmp = __osip_sdp_append_string (&string, &size, tmp, CRLF);
  }
  pos = 0;
  while (!osip_list_eol (&sdp->e_emails, pos)) {
    char *email = (char *) osip_list_get (&sdp->e_emails, pos);

    tmp = __osip_sdp_append_string (&string, &size, tmp, "e=");
    tmp = __osip_sdp_append_string (&string, &size, tmp, email);
    tmp = __osip_sdp_append_string (&string, &size, tmp, CRLF);
    pos++;
  }
  pos = 0;
  while (!osip_list_eol (&sdp->p_phones, pos)) {
    char *phone = (char *) osip_list_get (&sdp->p_phones, pos);

    tmp = __osip_sdp_append_string (&string, &size, tmp, "p=");
    tmp = __osip_sdp_append_string (&string, &size, tmp, phone);
    tmp = __osip_sdp_append_string (&string, &size, tmp, CRLF);
    pos++;
  }
  if (sdp->c_connection != NULL) {
    char *next_tmp;
    int i;

    i = sdp_append_connection (&string, &size, tmp, sdp->c_connection, &next_tmp);
    if (i != 0) {
      osip_free (string);
      return -1;
    }
    tmp = next_tmp;
  }
  pos = 0;
  while (!osip_list_eol (&sdp->b_bandwidths, pos)) {
    sdp_bandwidth_t *header = (sdp_bandwidth_t *) osip_list_get (&sdp->b_bandwidths, pos);
    char *next_tmp;
    int i;

    i = sdp_append_bandwidth (&string, &size, tmp, header, &next_tmp);
    if (i != 0) {
      osip_free (string);
      return -1;
    }
    tmp = next_tmp;
    pos++;
  }

  pos = 0;
  while (!osip_list_eol (&sdp->t_descrs, pos)) {
    sdp_time_descr_t *header = (sdp_time_descr_t *) osip_list_get (&sdp->t_descrs, pos);
    char *next_tmp;
    int i;

    i = sdp_append_time_descr (&string, &size, tmp, header, &next_tmp);
    if (i != 0) {
      osip_free (string);
      return -1;
    }
    tmp = next_tmp;
    pos++;
  }

  if (sdp->z_adjustments != NULL) {
    tmp = __osip_sdp_append_string (&string, &size, tmp, "z=");
    tmp = __osip_sdp_append_string (&string, &size, tmp, sdp->z_adjustments);
    tmp = __osip_sdp_append_string (&string, &size, tmp, CRLF);
  }

  if (sdp->k_key != NULL) {
    char *next_tmp;
    int i;

    i = sdp_append_key (&string, &size, tmp, sdp->k_key, &next_tmp);
    if (i != 0) {
      osip_free (string);
      return -1;
    }
    tmp = next_tmp;
  }

  pos = 0;
  while (!osip_list_eol (&sdp->a_attributes, pos)) {
    sdp_attribute_t *header = (sdp_attribute_t *) osip_list_get (&sdp->a_attributes, pos);
    char *next_tmp;
    int i;

    i = sdp_append_attribute (&string, &size, tmp, header, &next_tmp);
    if (i != 0) {
      osip_free (string);
      return -1;
    }
    tmp = next_tmp;
    pos++;
  }

  pos = 0;
  while (!osip_list_eol (&sdp->m_medias, pos)) {
    sdp_media_t *header = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos);
    char *next_tmp;
    int i;

    i = sdp_append_media (&string, &size, tmp, header, &next_tmp);
    if (i != 0) {
      osip_free (string);
      return -1;
    }
    tmp = next_tmp;
    pos++;
  }
  *dest = string;
  return OSIP_SUCCESS;
}

void
sdp_message_free (sdp_message_t * sdp)
{
  if (sdp == NULL)
    return;
  osip_free (sdp->v_version);
  osip_free (sdp->o_username);
  osip_free (sdp->o_sess_id);
  osip_free (sdp->o_sess_version);
  osip_free (sdp->o_nettype);
  osip_free (sdp->o_addrtype);
  osip_free (sdp->o_addr);
  osip_free (sdp->s_name);
  osip_free (sdp->i_info);
  osip_free (sdp->u_uri);

  osip_list_ofchar_free (&sdp->e_emails);

  osip_list_ofchar_free (&sdp->p_phones);

  sdp_connection_free (sdp->c_connection);

  osip_list_special_free (&sdp->b_bandwidths, (void (*)(void *)) &sdp_bandwidth_free);

  osip_list_special_free (&sdp->t_descrs, (void (*)(void *)) &sdp_time_descr_free);

  osip_free (sdp->z_adjustments);
  sdp_key_free (sdp->k_key);

  osip_list_special_free (&sdp->a_attributes, (void (*)(void *)) &sdp_attribute_free);

  osip_list_special_free (&sdp->m_medias, (void (*)(void *)) &sdp_media_free);

  osip_free (sdp);
}

int
sdp_message_clone (sdp_message_t * sdp, sdp_message_t ** dest)
{
  int i;
  char *body;

  i = sdp_message_init (dest);
  if (i != 0)
    return -1;

  i = sdp_message_to_str (sdp, &body);
  if (i != 0)
    goto error_sc1;

  i = sdp_message_parse (*dest, body);
  osip_free (body);
  if (i != 0)
    goto error_sc1;

  return OSIP_SUCCESS;

error_sc1:
  sdp_message_free (*dest);
  return -1;
}
