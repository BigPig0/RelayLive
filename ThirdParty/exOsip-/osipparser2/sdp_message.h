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


#ifndef _SDP_H_
#define _SDP_H_

#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

#include <osipparser2/osip_list.h>


/**
 * @file sdp_message.h
 * @brief oSIP SDP parser Routines
 *
 * This is the SDP accessor and parser related API.
 */

/**
 * @defgroup oSIP_SDP oSIP SDP parser Handling
 * @ingroup osip2_sdp
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Structure for referencing bandwidth header.
 * @var sdp_bandwidth_t
 */
  typedef struct sdp_bandwidth sdp_bandwidth_t;

/**
 * SDP bandwidth definition.
 * @struct sdp_bandwidth
 */
  struct sdp_bandwidth {
    char *b_bwtype;                             /**< bandwidth type */
    char *b_bandwidth;                          /**< bandwidth value */
  };

/**
 * Allocate a bandwidth element.
 * @param elem The element to work on.
 */
  int sdp_bandwidth_init (sdp_bandwidth_t ** elem);
/**
 * Free a bandwidth element.
 * @param elem The element to work on.
 */
  void sdp_bandwidth_free (sdp_bandwidth_t * elem);

/**
 * Structure for referencing time description header.
 * @var sdp_time_descr_t
 */
  typedef struct sdp_time_descr sdp_time_descr_t;

/**
 * SDP Time description definition.
 * @struct sdp_time_descr
 */
  struct sdp_time_descr {
    char *t_start_time;                            /**< start time */
    char *t_stop_time;                             /**< stop time */
    osip_list_t r_repeats;                /**< repeat headers */
  };

/**
 * Allocate a time description element.
 * @param elem The element to work on.
 */
  int sdp_time_descr_init (sdp_time_descr_t ** elem);
/**
 * Free a time description element.
 * @param elem The element to work on.
 */
  void sdp_time_descr_free (sdp_time_descr_t * elem);

/**
 * Structure for referencing key header.
 * @var sdp_key_t
 */
  typedef struct sdp_key sdp_key_t;

/**
 * SDP key definition.
 * @struct sdp_key
 */
  struct sdp_key {
    char *k_keytype;            /**< Key Type (prompt, clear, base64, uri) */
    char *k_keydata;            /**< key data */
  };

/**
 * Allocate a key element.
 * @param elem The element to work on.
 */
  int sdp_key_init (sdp_key_t ** elem);
/**
 * Free a key element.
 * @param elem The element to work on.
 */
  void sdp_key_free (sdp_key_t * elem);

/**
 * Structure for referencing an attribute header.
 * @var sdp_attribute_t
 */
  typedef struct sdp_attribute sdp_attribute_t;

/**
 * SDP attribute definition.
 * @struct sdp_attribute
 */
  struct sdp_attribute {
    char *a_att_field;                          /**< attribute field */
    char *a_att_value;                          /**< attribute value (optional) */
  };

/**
 * Allocate an attribute element.
 * @param elem The element to work on.
 */
  int sdp_attribute_init (sdp_attribute_t ** elem);
/**
 * Free a attribute element.
 * @param elem The element to work on.
 */
  void sdp_attribute_free (sdp_attribute_t * elem);


/**
 * Structure for referencing a connection header.
 * @var sdp_connection_t
 */
  typedef struct sdp_connection sdp_connection_t;

/**
 * SDP connection definition.
 * @struct sdp_connection
 */
  struct sdp_connection {
    char *c_nettype;                             /**< Network Type */
    char *c_addrtype;                            /**< Network Address Type */
    char *c_addr;                                /**< Address */
    char *c_addr_multicast_ttl;
                                                                 /**< TTL value for multicast address */
    char *c_addr_multicast_int;
                                                                 /**< Number of multicast address */
  };

/**
 * Allocate a connection element.
 * @param elem The element to work on.
 */
  int sdp_connection_init (sdp_connection_t ** elem);
/**
 * Free a connection element.
 * @param elem The element to work on.
 */
  void sdp_connection_free (sdp_connection_t * elem);

/**
 * Structure for referencing a media header.
 * @var sdp_media_t
 */
  typedef struct sdp_media sdp_media_t;

/**
 * SDP media definition.
 * @struct sdp_media
 */
  struct sdp_media {
    char *m_media;                              /**< media type */
    char *m_port;                               /**< port number */
    char *m_number_of_port;             /**< number of port */
    char *m_proto;                              /**< protocol to be used */
    osip_list_t m_payloads;            /**< list of payloads (as strings) */

    char *i_info;                               /**< information header */
    osip_list_t c_connections;
                                                           /**< list of sdp_connection_t * */
    osip_list_t b_bandwidths;
                                                           /**< list of sdp_bandwidth_t * */
    osip_list_t a_attributes;
                                                           /**< list of sdp_attribute_t * */
    sdp_key_t *k_key;                           /**< key informations */
  };

/**
 * Allocate a media element.
 * @param elem The element to work on.
 */
  int sdp_media_init (sdp_media_t ** elem);
/**
 * Free a media element.
 * @param elem The element to work on.
 */
  void sdp_media_free (sdp_media_t * elem);

/**
 * Structure for referencing a SDP packet.
 * @var sdp_message_t
 */
  typedef struct sdp_message sdp_message_t;

/**
 * SDP message definition.
 * @struct sdp_message
 */
  struct sdp_message {
    char *v_version;                            /**< version header */
    char *o_username;                           /**< Username */
    char *o_sess_id;                            /**< Identifier for session */
    char *o_sess_version;               /**< Version of session */
    char *o_nettype;                            /**< Network type */
    char *o_addrtype;                           /**< Address type */
    char *o_addr;                               /**< Address */
    char *s_name;                               /**< Subject header */
    char *i_info;                               /**< Information header */
    char *u_uri;                                /**< Uri header */
    osip_list_t e_emails;              /**< list of mail address */
    osip_list_t p_phones;              /**< list of phone numbers * */
    sdp_connection_t *c_connection;
                                                                          /**< Connection information */
    osip_list_t b_bandwidths;
                                                           /**< list of bandwidth info (sdp_bandwidth_t) */
    osip_list_t t_descrs;              /**< list of time description (sdp_time_descr_t) */
    char *z_adjustments;                /**< Time adjustment header */
    sdp_key_t *k_key;                           /**< Key information header */
    osip_list_t a_attributes;
                                                           /**< list of global attributes (sdp_attribute_t) */
    osip_list_t m_medias;              /**< list of supported media (sdp_media_t) */
  };



/**
 * Allocate a SDP packet.
 * @param sdp The element to work on.
 */
  int sdp_message_init (sdp_message_t ** sdp);
/**
 * Parse a SDP packet.
 * @param sdp The element to work on.
 * @param buf The buffer to parse.
 */
  int sdp_message_parse (sdp_message_t * sdp, const char *buf);
/**
 * Get a string representation of a SDP packet.
 * @param sdp The element to work on.
 * @param dest The resulting new allocated buffer.
 */
  int sdp_message_to_str (sdp_message_t * sdp, char **dest);
/**
 * Free a SDP packet.
 * @param sdp The element to work on.
 */
  void sdp_message_free (sdp_message_t * sdp);
/**
 * Clone a SDP packet.
 * @param sdp The element to work on.
 * @param dest The cloned element.
 */
  int sdp_message_clone (sdp_message_t * sdp, sdp_message_t ** dest);

/**
 * Set the version in a SDP packet.
 * @param sdp The element to work on.
 * @param value The token value.
 */
  int sdp_message_v_version_set (sdp_message_t * sdp, char *value);
/**
 * Get the version ('v' field) of a SDP packet.
 * @param sdp The element to work on.
 */
  char *sdp_message_v_version_get (sdp_message_t * sdp);
/**
 * Set the origin field in a SDP packet.
 * @param sdp The element to work on.
 * @param username The token value.
 * @param sess_id The token value.
 * @param sess_version The token value.
 * @param nettype The token value.
 * @param addrtype The token value.
 * @param addr The token value.
 */
  int sdp_message_o_origin_set (sdp_message_t * sdp, char *username, char *sess_id, char *sess_version, char *nettype, char *addrtype, char *addr);
/**
 * Get the username ('o' field) of a SDP packet.
 * @param sdp The element to work on.
 */
  char *sdp_message_o_username_get (sdp_message_t * sdp);
/**
 * Get the session id ('o' field) of a SDP packet.
 * @param sdp The element to work on.
 */
  char *sdp_message_o_sess_id_get (sdp_message_t * sdp);
/**
 * Get the session version ('o' field) of a SDP packet.
 * @param sdp The element to work on.
 */
  char *sdp_message_o_sess_version_get (sdp_message_t * sdp);
/**
 * Get the nettype ('o' field) of a SDP packet.
 * @param sdp The element to work on.
 */
  char *sdp_message_o_nettype_get (sdp_message_t * sdp);
/**
 * Get the addrtype ('o' field) of a SDP packet.
 * @param sdp The element to work on.
 */
  char *sdp_message_o_addrtype_get (sdp_message_t * sdp);
/**
 * Get the addr ('o' field) of a SDP packet.
 * @param sdp The element to work on.
 */
  char *sdp_message_o_addr_get (sdp_message_t * sdp);
/**
 * Set the session name in a SDP packet.
 * @param sdp The element to work on.
 * @param value The token value.
 */
  int sdp_message_s_name_set (sdp_message_t * sdp, char *value);
/**
 * Get the session name ('s' field) of a SDP packet.
 * @param sdp The element to work on.
 */
  char *sdp_message_s_name_get (sdp_message_t * sdp);
/**
 * Set the version in a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param value The token value.
 */
  int sdp_message_i_info_set (sdp_message_t * sdp, int pos_media, char *value);
/**
 * Get the session info ('i' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 */
  char *sdp_message_i_info_get (sdp_message_t * sdp, int pos_media);
/**
 * Set the session info in a SDP packet.
 * @param sdp The element to work on.
 * @param value The token value.
 */
  int sdp_message_u_uri_set (sdp_message_t * sdp, char *value);
/**
 * Get the uri ('u' field) of a SDP packet.
 * @param sdp The element to work on.
 */
  char *sdp_message_u_uri_get (sdp_message_t * sdp);
/**
 * Set the version in a SDP packet.
 * @param sdp The element to work on.
 * @param value The token value.
 */
  int sdp_message_e_email_add (sdp_message_t * sdp, char *value);
/**
 * OBSOLETE: see sdp_message_e_email_get
 * @def sdp_e_email_get
 */
#define sdp_e_email_get sdp_message_e_email_get
/**
 * Get one of the email ('e' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos the index of the email line.
 */
  char *sdp_message_e_email_get (sdp_message_t * sdp, int pos);
/**
 * Set the version in a SDP packet.
 * @param sdp The element to work on.
 * @param value The token value.
 */
  int sdp_message_p_phone_add (sdp_message_t * sdp, char *value);
/**
 * Get one of the phone ('p' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos the index of the phone line.
 */
  char *sdp_message_p_phone_get (sdp_message_t * sdp, int pos);
/**
 * Set the version in a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param nettype The token value.
 * @param addrtype The token value.
 * @param addr The token value.
 * @param addr_multicast_ttl The token value.
 * @param addr_multicast_int The token value.
 */
  int sdp_message_c_connection_add (sdp_message_t * sdp, int pos_media, char *nettype, char *addrtype, char *addr, char *addr_multicast_ttl, char *addr_multicast_int);
#ifndef DOXYGEN
/* this method should be internal only... */
  sdp_connection_t *sdp_message_connection_get (sdp_message_t * sdp, int pos_media, int pos);
#endif
/**
 * Get the network type ('c' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param pos The index in the connection element list..
 */
  char *sdp_message_c_nettype_get (sdp_message_t * sdp, int pos_media, int pos);
/**
 * Get the address type ('c' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param pos The index in the connection element list..
 */
  char *sdp_message_c_addrtype_get (sdp_message_t * sdp, int pos_media, int pos);
/**
 * Get the address ('c' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param pos The index in the connection element list..
 */
  char *sdp_message_c_addr_get (sdp_message_t * sdp, int pos_media, int pos);
/**
 * Get the multicast ttl ('c' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param pos The index in the connection element list..
 */
  char *sdp_message_c_addr_multicast_ttl_get (sdp_message_t * sdp, int pos_media, int pos);
/**
 * Get the multicast int info ('c' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param pos The index in the connection element list..
 */
  char *sdp_message_c_addr_multicast_int_get (sdp_message_t * sdp, int pos_media, int pos);
/**
 * Set the version in a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param bwtype The token value.
 * @param bandwidth The token value.
 */
  int sdp_message_b_bandwidth_add (sdp_message_t * sdp, int pos_media, char *bwtype, char *bandwidth);
/**
 * Get the bandwidth ('b' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param pos The index in the bandwidth element list..
 */
  sdp_bandwidth_t *sdp_message_bandwidth_get (sdp_message_t * sdp, int pos_media, int pos);
/**
 * Get the bandwidth type ('b' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param pos The index in the bandwidth element list..
 */
  char *sdp_message_b_bwtype_get (sdp_message_t * sdp, int pos_media, int pos);
/**
 * Get the bandwidth value ('b' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param pos The index in the bandwidth element list..
 */
  char *sdp_message_b_bandwidth_get (sdp_message_t * sdp, int pos_media, int pos);
/**
 * Set the version in a SDP packet.
 * @param sdp The element to work on.
 * @param start The token value.
 * @param stop The token value.
 */
  int sdp_message_t_time_descr_add (sdp_message_t * sdp, char *start, char *stop);
/**
 * Get the start time value ('t' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_td The time description line number.
 */
  char *sdp_message_t_start_time_get (sdp_message_t * sdp, int pos_td);
/**
 * Get the stop time value ('t' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_td The time description line number.
 */
  char *sdp_message_t_stop_time_get (sdp_message_t * sdp, int pos_td);
/**
 * Set the repeat information ('r' field) in a SDP packet.
 * @param sdp The element to work on.
 * @param pos_time_descr index of t field.
 * @param value The token value.
 */
  int sdp_message_r_repeat_add (sdp_message_t * sdp, int pos_time_descr, char *value);
/**
 * Get the repeat information ('r' field) in a SDP packet.
 * @param sdp The element to work on.
 * @param pos_time_descr index of t field.
 * @param pos_repeat index of element in the 'r' field.
 */
  char *sdp_message_r_repeat_get (sdp_message_t * sdp, int pos_time_descr, int pos_repeat);
/**
 * Set the adjustments ('z' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param value The token value.
 */
  int sdp_message_z_adjustments_set (sdp_message_t * sdp, char *value);
/**
 * Get the adjustments ('z' field) of a SDP packet.
 * @param sdp The element to work on.
 */
  char *sdp_message_z_adjustments_get (sdp_message_t * sdp);
/**
 * Add a key in a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media index of m field.
 * @param keytype The token value.
 * @param keydata The token value.
 */
  int sdp_message_k_key_set (sdp_message_t * sdp, int pos_media, char *keytype, char *keydata);
/**
 * Get the key type ('k' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 */
  char *sdp_message_k_keytype_get (sdp_message_t * sdp, int pos_media);
/**
 * Get the key value ('k' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 */
  char *sdp_message_k_keydata_get (sdp_message_t * sdp, int pos_media);
/**
 * Set the version in a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The line number.
 * @param att_field The token value.
 * @param att_value The token value.
 */
  int sdp_message_a_attribute_add (sdp_message_t * sdp, int pos_media, char *att_field, char *att_value);
/**
 * delete all attribute fields specified by att_field.
 * @param sdp The element to work on.
 * @param pos_media The line number.
 * @param att_field The value to remove.
 */
  int sdp_message_a_attribute_del (sdp_message_t * sdp, int pos_media, char *att_field);
/**
 * delete one specific attribute fields specified by att_field.
 * @param sdp The element to work on.
 * @param pos_media The line number.
 * @param att_field The value to remove.
 * @param pos_attr The index of attribute to remove.
 */
  int sdp_message_a_attribute_del_at_index (sdp_message_t * sdp, int pos_media, char *att_field, int pos_attr);
 /**
 * Get one of the attribute ('a' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param pos The attribute line number.
 */
  sdp_attribute_t *sdp_message_attribute_get (sdp_message_t * sdp, int pos_media, int pos);
/**
 * Get the attribute name ('a' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param pos The attribute line number.
 */
  char *sdp_message_a_att_field_get (sdp_message_t * sdp, int pos_media, int pos);
/**
 * Get the attribute value ('a' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The media line number.
 * @param pos The attribute line number.
 */
  char *sdp_message_a_att_value_get (sdp_message_t * sdp, int pos_media, int pos);
/**
 * Check if there is more media lines a SDP packet.
 * @param sdp The element to work on.
 * @param pos The attribute line number.
 */
  int sdp_message_endof_media (sdp_message_t * sdp, int pos);
/**
 * Add a media line in a SDP packet.
 * @param sdp The element to work on.
 * @param media The token value.
 * @param port The token value.
 * @param number_of_port The token value.
 * @param proto The token value.
 */
  int sdp_message_m_media_add (sdp_message_t * sdp, char *media, char *port, char *number_of_port, char *proto);
/**
 * Get the media type ('m' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The line number.
 */
  char *sdp_message_m_media_get (sdp_message_t * sdp, int pos_media);
/**
 * Get the port number ('m' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The line number.
 */
  char *sdp_message_m_port_get (sdp_message_t * sdp, int pos_media);
/**
 * Set the port number ('m' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The line number.
 * @param port The new port to set (must be allocated with osip_malloc)
 */
  int sdp_message_m_port_set (sdp_message_t * sdp, int pos_media, char *port);
 /**
 * Get the number of port ('m' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The line number.
 */
  char *sdp_message_m_number_of_port_get (sdp_message_t * sdp, int pos_media);
/**
 * Get the protocol ('m' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The line number.
 */
  char *sdp_message_m_proto_get (sdp_message_t * sdp, int pos_media);
/**
 * Set the payload in a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The line number.
 * @param payload The token value.
 */
  int sdp_message_m_payload_add (sdp_message_t * sdp, int pos_media, char *payload);
/**
 * Get one of the payload number ('m' field) of a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The line number.
 * @param pos The i th payload element.
 */
  char *sdp_message_m_payload_get (sdp_message_t * sdp, int pos_media, int pos);
/**
 * Remove a payload in a SDP packet.
 * @param sdp The element to work on.
 * @param pos_media The line number.
 * @param pos The position of the payload in the media line.
 */
  int sdp_message_m_payload_del (sdp_message_t * sdp, int pos_media, int pos);


/** @} */


#ifdef __cplusplus
}
#endif
#endif
