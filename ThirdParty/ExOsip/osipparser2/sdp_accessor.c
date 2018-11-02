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
#include <osipparser2/osip_port.h>
#include <osipparser2/sdp_message.h>

int
sdp_message_v_version_set (sdp_message_t * sdp, char *v_version)
{
  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  sdp->v_version = v_version;
  return OSIP_SUCCESS;
}

char *
sdp_message_v_version_get (sdp_message_t * sdp)
{
  if (sdp == NULL)
    return NULL;
  return sdp->v_version;
}

int
sdp_message_o_origin_set (sdp_message_t * sdp, char *username, char *sess_id, char *sess_version, char *nettype, char *addrtype, char *addr)
{
  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  sdp->o_username = username;
  sdp->o_sess_id = sess_id;
  sdp->o_sess_version = sess_version;
  sdp->o_nettype = nettype;
  sdp->o_addrtype = addrtype;
  sdp->o_addr = addr;
  return OSIP_SUCCESS;
}

char *
sdp_message_o_username_get (sdp_message_t * sdp)
{
  if (sdp == NULL)
    return NULL;
  return sdp->o_username;
}

char *
sdp_message_o_sess_id_get (sdp_message_t * sdp)
{
  if (sdp == NULL)
    return NULL;
  return sdp->o_sess_id;
}

char *
sdp_message_o_sess_version_get (sdp_message_t * sdp)
{
  if (sdp == NULL)
    return NULL;
  return sdp->o_sess_version;
}

char *
sdp_message_o_nettype_get (sdp_message_t * sdp)
{
  if (sdp == NULL)
    return NULL;
  return sdp->o_nettype;
}

char *
sdp_message_o_addrtype_get (sdp_message_t * sdp)
{
  if (sdp == NULL)
    return NULL;
  return sdp->o_addrtype;
}

char *
sdp_message_o_addr_get (sdp_message_t * sdp)
{
  if (sdp == NULL)
    return NULL;
  return sdp->o_addr;
}

int
sdp_message_s_name_set (sdp_message_t * sdp, char *name)
{
  if (sdp == NULL)
    return -1;
  sdp->s_name = name;
  return OSIP_SUCCESS;
}

char *
sdp_message_s_name_get (sdp_message_t * sdp)
{
  if (sdp == NULL)
    return NULL;
  return sdp->s_name;
}

int
sdp_message_i_info_set (sdp_message_t * sdp, int pos_media, char *info)
{
  sdp_media_t *med;

  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  if (pos_media == -1) {
    sdp->i_info = info;
    return OSIP_SUCCESS;
  }
  med = osip_list_get (&sdp->m_medias, pos_media);
  if (med == NULL)
    return OSIP_UNDEFINED_ERROR;
  med->i_info = info;
  return OSIP_SUCCESS;
}

char *
sdp_message_i_info_get (sdp_message_t * sdp, int pos_media)
{
  sdp_media_t *med;

  if (sdp == NULL)
    return NULL;
  if (pos_media == -1) {
    return sdp->i_info;
  }
  med = osip_list_get (&sdp->m_medias, pos_media);
  if (med == NULL)
    return NULL;
  return med->i_info;
}

int
sdp_message_u_uri_set (sdp_message_t * sdp, char *uri)
{
  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  sdp->u_uri = uri;
  return OSIP_SUCCESS;
}

char *
sdp_message_u_uri_get (sdp_message_t * sdp)
{
  if (sdp == NULL)
    return NULL;
  return sdp->u_uri;
}

int
sdp_message_e_email_add (sdp_message_t * sdp, char *email)
{
  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  osip_list_add (&sdp->e_emails, email, -1);
  return OSIP_SUCCESS;
}

char *
sdp_message_e_email_get (sdp_message_t * sdp, int pos)
{
  if (sdp == NULL)
    return NULL;
  if (osip_list_size (&sdp->e_emails) > pos)
    return (char *) osip_list_get (&sdp->e_emails, pos);
  return NULL;
}

int
sdp_message_p_phone_add (sdp_message_t * sdp, char *phone)
{
  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  osip_list_add (&sdp->p_phones, phone, -1);
  return OSIP_SUCCESS;
}

char *
sdp_message_p_phone_get (sdp_message_t * sdp, int pos)
{
  if (sdp == NULL)
    return NULL;
  if (osip_list_size (&sdp->p_phones) > pos)
    return (char *) osip_list_get (&sdp->p_phones, pos);
  return NULL;
}

int
sdp_message_c_connection_add (sdp_message_t * sdp, int pos_media, char *nettype, char *addrtype, char *addr, char *addr_multicast_ttl, char *addr_multicast_int)
{
  int i;
  sdp_media_t *med;
  sdp_connection_t *conn;

  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  if ((pos_media != -1) && (osip_list_size (&sdp->m_medias) < pos_media + 1))
    return -1;
  i = sdp_connection_init (&conn);
  if (i != 0)
    return i;
  conn->c_nettype = nettype;
  conn->c_addrtype = addrtype;
  conn->c_addr = addr;
  conn->c_addr_multicast_ttl = addr_multicast_ttl;
  conn->c_addr_multicast_int = addr_multicast_int;
  if (pos_media == -1) {
    sdp->c_connection = conn;
    return OSIP_SUCCESS;
  }
  med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos_media);
  osip_list_add (&med->c_connections, conn, -1);
  return OSIP_SUCCESS;
}

/* this method should be internal only... */
sdp_connection_t *
sdp_message_connection_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_media_t *med;

  if (sdp == NULL)
    return NULL;
  if (pos_media == -1)          /* pos is useless in this case: 1 global "c=" is allowed */
    return sdp->c_connection;
  med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos_media);
  if (med == NULL)
    return NULL;
  return (sdp_connection_t *) osip_list_get (&med->c_connections, pos);
}

char *
sdp_message_c_nettype_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_connection_t *conn = sdp_message_connection_get (sdp, pos_media, pos);

  if (conn == NULL)
    return NULL;
  return conn->c_nettype;
}

char *
sdp_message_c_addrtype_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_connection_t *conn = sdp_message_connection_get (sdp, pos_media, pos);

  if (conn == NULL)
    return NULL;
  return conn->c_addrtype;
}

char *
sdp_message_c_addr_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_connection_t *conn = sdp_message_connection_get (sdp, pos_media, pos);

  if (conn == NULL)
    return NULL;
  return conn->c_addr;
}

char *
sdp_message_c_addr_multicast_ttl_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_connection_t *conn = sdp_message_connection_get (sdp, pos_media, pos);

  if (conn == NULL)
    return NULL;
  return conn->c_addr_multicast_ttl;
}

char *
sdp_message_c_addr_multicast_int_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_connection_t *conn = sdp_message_connection_get (sdp, pos_media, pos);

  if (conn == NULL)
    return NULL;
  return conn->c_addr_multicast_int;
}

int
sdp_message_b_bandwidth_add (sdp_message_t * sdp, int pos_media, char *bwtype, char *bandwidth)
{
  int i;
  sdp_media_t *med;
  sdp_bandwidth_t *band;

  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  if ((pos_media != -1) && (osip_list_size (&sdp->m_medias) < pos_media + 1))
    return OSIP_UNDEFINED_ERROR;
  i = sdp_bandwidth_init (&band);
  if (i != 0)
    return i;
  band->b_bwtype = bwtype;
  band->b_bandwidth = bandwidth;
  if (pos_media == -1) {
    osip_list_add (&sdp->b_bandwidths, band, -1);
    return OSIP_SUCCESS;
  }
  med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos_media);
  osip_list_add (&med->b_bandwidths, band, -1);
  return OSIP_SUCCESS;
}

sdp_bandwidth_t *
sdp_message_bandwidth_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_media_t *med;

  if (sdp == NULL)
    return NULL;
  if (pos_media == -1)
    return (sdp_bandwidth_t *) osip_list_get (&sdp->b_bandwidths, pos);
  med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos_media);
  if (med == NULL)
    return NULL;
  return (sdp_bandwidth_t *) osip_list_get (&med->b_bandwidths, pos);
}

char *
sdp_message_b_bwtype_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_bandwidth_t *band = sdp_message_bandwidth_get (sdp, pos_media, pos);

  if (band == NULL)
    return NULL;
  return band->b_bwtype;
}

char *
sdp_message_b_bandwidth_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_bandwidth_t *band = sdp_message_bandwidth_get (sdp, pos_media, pos);

  if (band == NULL)
    return NULL;
  return band->b_bandwidth;
}

int
sdp_message_t_time_descr_add (sdp_message_t * sdp, char *start, char *stop)
{
  int i;
  sdp_time_descr_t *td;

  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  i = sdp_time_descr_init (&td);
  if (i != 0)
    return i;
  td->t_start_time = start;
  td->t_stop_time = stop;
  osip_list_add (&sdp->t_descrs, td, -1);
  return OSIP_SUCCESS;
}

char *
sdp_message_t_start_time_get (sdp_message_t * sdp, int pos_td)
{
  sdp_time_descr_t *td;

  if (sdp == NULL)
    return NULL;
  td = (sdp_time_descr_t *) osip_list_get (&sdp->t_descrs, pos_td);
  if (td == NULL)
    return NULL;
  return td->t_start_time;
}

char *
sdp_message_t_stop_time_get (sdp_message_t * sdp, int pos_td)
{
  sdp_time_descr_t *td;

  if (sdp == NULL)
    return NULL;
  td = (sdp_time_descr_t *) osip_list_get (&sdp->t_descrs, pos_td);
  if (td == NULL)
    return NULL;
  return td->t_stop_time;
}

int
sdp_message_r_repeat_add (sdp_message_t * sdp, int pos_time_descr, char *field)
{
  sdp_time_descr_t *td;

  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  td = (sdp_time_descr_t *) osip_list_get (&sdp->t_descrs, pos_time_descr);
  if (td == NULL)
    return OSIP_UNDEFINED_ERROR;
  osip_list_add (&td->r_repeats, field, -1);
  return OSIP_SUCCESS;
}

char *
sdp_message_r_repeat_get (sdp_message_t * sdp, int pos_time_descr, int pos_repeat)
{
  sdp_time_descr_t *td;

  if (sdp == NULL)
    return NULL;
  td = (sdp_time_descr_t *) osip_list_get (&sdp->t_descrs, pos_time_descr);
  if (td == NULL)
    return NULL;
  return (char *) osip_list_get (&td->r_repeats, pos_repeat);
}

int
sdp_message_z_adjustments_set (sdp_message_t * sdp, char *field)
{
  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  sdp->z_adjustments = field;
  return OSIP_SUCCESS;
}

char *
sdp_message_z_adjustments_get (sdp_message_t * sdp)
{
  if (sdp == NULL)
    return NULL;
  return sdp->z_adjustments;
}

int
sdp_message_k_key_set (sdp_message_t * sdp, int pos_media, char *keytype, char *keydata)
{
  sdp_key_t *key;
  sdp_media_t *med;
  int i;

  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  if ((pos_media != -1) && (osip_list_size (&sdp->m_medias) < pos_media + 1))
    return OSIP_UNDEFINED_ERROR;
  i = sdp_key_init (&key);
  if (i != 0)
    return i;
  key->k_keytype = keytype;
  key->k_keydata = keydata;
  if (pos_media == -1) {
    sdp->k_key = key;
    return OSIP_SUCCESS;
  }
  med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos_media);
  med->k_key = key;
  return OSIP_SUCCESS;
}

char *
sdp_message_k_keytype_get (sdp_message_t * sdp, int pos_media)
{
  sdp_media_t *med;

  if (sdp == NULL)
    return NULL;
  if (pos_media == -1) {
    if (sdp->k_key == NULL)
      return NULL;
    return sdp->k_key->k_keytype;
  }
  if ((pos_media != -1) && (osip_list_size (&sdp->m_medias) < pos_media + 1))
    return NULL;
  med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos_media);
  if (med->k_key == NULL)
    return NULL;
  return med->k_key->k_keytype;
}

char *
sdp_message_k_keydata_get (sdp_message_t * sdp, int pos_media)
{
  sdp_media_t *med;

  if (sdp == NULL)
    return NULL;
  if (pos_media == -1) {
    if (sdp->k_key == NULL)
      return NULL;
    return sdp->k_key->k_keydata;
  }
  if ((pos_media != -1) && (osip_list_size (&sdp->m_medias) < pos_media + 1))
    return NULL;
  med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos_media);
  if (med->k_key == NULL)
    return NULL;
  return med->k_key->k_keydata;
}

int
sdp_message_a_attribute_add (sdp_message_t * sdp, int pos_media, char *att_field, char *att_value)
{
  int i;
  sdp_media_t *med;
  sdp_attribute_t *attr;

  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  if ((pos_media != -1) && (osip_list_size (&sdp->m_medias) < pos_media + 1))
    return OSIP_UNDEFINED_ERROR;
  i = sdp_attribute_init (&attr);
  if (i != 0)
    return i;
  attr->a_att_field = att_field;
  attr->a_att_value = att_value;
  if (pos_media == -1) {
    osip_list_add (&sdp->a_attributes, attr, -1);
    return OSIP_SUCCESS;
  }
  med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos_media);
  osip_list_add (&med->a_attributes, attr, -1);
  return OSIP_SUCCESS;
}

int
sdp_message_a_attribute_del (sdp_message_t * sdp, int pos_media, char *att_field)
{
  int i;
  sdp_media_t *med;
  sdp_attribute_t *attr;

  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  if ((pos_media != -1) && (osip_list_size (&sdp->m_medias) < pos_media + 1))
    return OSIP_UNDEFINED_ERROR;
  if (pos_media == -1) {
    for (i = 0; i < osip_list_size (&sdp->a_attributes);) {
      attr = osip_list_get (&sdp->a_attributes, i);
      if (strcmp (attr->a_att_field, att_field) == 0) {
        osip_list_remove (&sdp->a_attributes, i);
        sdp_attribute_free (attr);
      }
      else
        i++;
    }
    return OSIP_SUCCESS;
  }
  med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos_media);
  if (med == NULL)
    return OSIP_UNDEFINED_ERROR;
  for (i = 0; i < osip_list_size (&med->a_attributes);) {
    attr = osip_list_get (&med->a_attributes, i);
    if (strcmp (attr->a_att_field, att_field) == 0) {
      osip_list_remove (&med->a_attributes, i);
      sdp_attribute_free (attr);
    }
    else
      i++;
  }
  return OSIP_SUCCESS;
}

int
sdp_message_a_attribute_del_at_index (sdp_message_t * sdp, int pos_media, char *att_field, int pos_attr)
{
  int i;
  sdp_media_t *med;
  sdp_attribute_t *attr;

  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  if ((pos_media != -1) && (osip_list_size (&sdp->m_medias) < pos_media + 1))
    return OSIP_UNDEFINED_ERROR;
  if (pos_media == -1) {
    if (pos_attr == -1) {
      for (i = 0; i < osip_list_size (&sdp->a_attributes);) {
        attr = osip_list_get (&sdp->a_attributes, i);
        if (strcmp (attr->a_att_field, att_field) == 0) {
          osip_list_remove (&sdp->a_attributes, i);
          sdp_attribute_free (attr);
        }
        else
          i++;
      }
    }
    else if ((attr = osip_list_get (&sdp->a_attributes, pos_attr)) != NULL) {
      osip_list_remove (&sdp->a_attributes, pos_attr);
      sdp_attribute_free (attr);
    }
    return OSIP_SUCCESS;
  }
  med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos_media);
  if (med == NULL)
    return OSIP_UNDEFINED_ERROR;
  for (i = 0; i < osip_list_size (&med->a_attributes);) {
    if (pos_attr == -1) {
      attr = osip_list_get (&med->a_attributes, i);
      if (strcmp (attr->a_att_field, att_field) == 0) {
        osip_list_remove (&med->a_attributes, i);
        sdp_attribute_free (attr);
      }
      else
        i++;
    }
    else if ((attr = osip_list_get (&med->a_attributes, pos_attr)) != NULL) {
      osip_list_remove (&med->a_attributes, pos_attr);
      sdp_attribute_free (attr);
    }
  }
  return OSIP_SUCCESS;
}


sdp_attribute_t *
sdp_message_attribute_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_media_t *med;

  if (sdp == NULL)
    return NULL;
  if (pos_media == -1)
    return (sdp_attribute_t *) osip_list_get (&sdp->a_attributes, pos);
  med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos_media);
  if (med == NULL)
    return NULL;
  return (sdp_attribute_t *) osip_list_get (&med->a_attributes, pos);
}

char *
sdp_message_a_att_field_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_attribute_t *attr = sdp_message_attribute_get (sdp, pos_media, pos);

  if (attr == NULL)
    return NULL;
  return attr->a_att_field;
}

char *
sdp_message_a_att_value_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_attribute_t *attr = sdp_message_attribute_get (sdp, pos_media, pos);

  if (attr == NULL)
    return NULL;
  return attr->a_att_value;
}

int
sdp_message_endof_media (sdp_message_t * sdp, int i)
{
  if (sdp == NULL)
    return OSIP_BADPARAMETER;
  if (i == -1)
    return OSIP_SUCCESS;
  if (!osip_list_eol (&sdp->m_medias, i))
    return OSIP_SUCCESS;        /* not end of list */
  return OSIP_UNDEFINED_ERROR;  /* end of list */
}

int
sdp_message_m_media_add (sdp_message_t * sdp, char *media, char *port, char *number_of_port, char *proto)
{
  int i;
  sdp_media_t *med;

  i = sdp_media_init (&med);
  if (i != 0)
    return i;
  med->m_media = media;
  med->m_port = port;
  med->m_number_of_port = number_of_port;
  med->m_proto = proto;
  osip_list_add (&sdp->m_medias, med, -1);
  return OSIP_SUCCESS;
}

char *
sdp_message_m_media_get (sdp_message_t * sdp, int pos_media)
{
  sdp_media_t *med = osip_list_get (&sdp->m_medias, pos_media);

  if (med == NULL)
    return NULL;
  return med->m_media;
}

char *
sdp_message_m_port_get (sdp_message_t * sdp, int pos_media)
{
  sdp_media_t *med = osip_list_get (&sdp->m_medias, pos_media);

  if (med == NULL)
    return NULL;
  return med->m_port;
}

char *
sdp_message_m_number_of_port_get (sdp_message_t * sdp, int pos_media)
{
  sdp_media_t *med = osip_list_get (&sdp->m_medias, pos_media);

  if (med == NULL)
    return NULL;
  return med->m_number_of_port;
}

int
sdp_message_m_port_set (sdp_message_t * sdp, int pos_media, char *port)
{
  sdp_media_t *med = osip_list_get (&sdp->m_medias, pos_media);

  if (med == NULL)
    return OSIP_BADPARAMETER;
  if (med->m_port)
    osip_free (med->m_port);
  med->m_port = port;
  return OSIP_SUCCESS;
}

char *
sdp_message_m_proto_get (sdp_message_t * sdp, int pos_media)
{
  sdp_media_t *med = osip_list_get (&sdp->m_medias, pos_media);

  if (med == NULL)
    return NULL;
  return med->m_proto;
}

int
sdp_message_m_payload_add (sdp_message_t * sdp, int pos_media, char *payload)
{
  sdp_media_t *med = osip_list_get (&sdp->m_medias, pos_media);

  if (med == NULL)
    return OSIP_BADPARAMETER;
  osip_list_add (&med->m_payloads, payload, -1);
  return OSIP_SUCCESS;
}

char *
sdp_message_m_payload_get (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_media_t *med = osip_list_get (&sdp->m_medias, pos_media);

  if (med == NULL)
    return NULL;
  return (char *) osip_list_get (&med->m_payloads, pos);
}

int
sdp_message_m_payload_del (sdp_message_t * sdp, int pos_media, int pos)
{
  sdp_media_t *med = osip_list_get (&sdp->m_medias, pos_media);
  char *payload;

  if (med == NULL)
    return OSIP_BADPARAMETER;
  if ((payload = osip_list_get (&med->m_payloads, pos)) == NULL)
    return OSIP_UNDEFINED_ERROR;
  osip_list_remove (&med->m_payloads, pos);
  osip_free (payload);
  return OSIP_SUCCESS;
}
