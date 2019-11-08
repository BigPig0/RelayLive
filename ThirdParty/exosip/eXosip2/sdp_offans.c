/*
  eXosip - This is the eXtended osip library.
  Copyright (C) 2001-2015 Aymeric MOIZARD amoizard@antisip.com
  
  eXosip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  eXosip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of portions of this program with the
  OpenSSL library under certain conditions as described in each
  individual source file, and distribute linked combinations
  including the two.
  You must obey the GNU General Public License in all respects
  for all of the code used other than OpenSSL.  If you modify
  file(s) with this exception, you may extend this exception to your
  version of the file(s), but you are not obligated to do so.  If you
  do not wish to do so, delete this exception statement from your
  version.  If you delete this exception statement from all source
  files in the program, then also delete it here.
*/

#include "eXosip2.h"

sdp_message_t *_eXosip_get_remote_sdp (osip_transaction_t * invite_tr);
sdp_message_t *_eXosip_get_local_sdp (osip_transaction_t * invite_tr);

sdp_message_t *
eXosip_get_remote_sdp_from_tid (struct eXosip_t *excontext, int tid)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *tr = NULL;

  if (tid > 0) {
    _eXosip_call_transaction_find (excontext, tid, &jc, &jd, &tr);
  }
  if (jc == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    return NULL;
  }
  if (tr == NULL)
    return NULL;

  return _eXosip_get_remote_sdp (tr);
}


sdp_message_t *
eXosip_get_local_sdp_from_tid (struct eXosip_t * excontext, int tid)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *tr = NULL;

  if (tid > 0) {
    _eXosip_call_transaction_find (excontext, tid, &jc, &jd, &tr);
  }
  if (jc == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    return NULL;
  }
  if (tr == NULL)
    return NULL;

  return _eXosip_get_local_sdp (tr);
}

sdp_message_t *
eXosip_get_remote_sdp (struct eXosip_t * excontext, int jid)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *invite_tr = NULL;

  if (jid > 0) {
    _eXosip_call_dialog_find (excontext, jid, &jc, &jd);
  }
  if (jc == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    return NULL;
  }
  invite_tr = _eXosip_find_last_invite (jc, jd);
  if (invite_tr == NULL)
    return NULL;

  return _eXosip_get_remote_sdp (invite_tr);
}

sdp_message_t *
eXosip_get_previous_local_sdp (struct eXosip_t * excontext, int jid)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *invite_tr = NULL;

  if (jid > 0) {
    _eXosip_call_dialog_find (excontext, jid, &jc, &jd);
  }
  if (jc == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    return NULL;
  }
  invite_tr = _eXosip_find_last_invite (jc, jd);
  if (invite_tr == NULL)
    return NULL;
  invite_tr = _eXosip_find_previous_invite (jc, jd, invite_tr);
  if (invite_tr == NULL)
    return NULL;

  return _eXosip_get_local_sdp (invite_tr);
}

sdp_message_t *
eXosip_get_local_sdp (struct eXosip_t * excontext, int jid)
{
  eXosip_dialog_t *jd = NULL;
  eXosip_call_t *jc = NULL;
  osip_transaction_t *invite_tr = NULL;

  if (jid > 0) {
    _eXosip_call_dialog_find (excontext, jid, &jc, &jd);
  }
  if (jc == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: No call here?\n"));
    return NULL;
  }
  invite_tr = _eXosip_find_last_invite (jc, jd);
  if (invite_tr == NULL)
    return NULL;

  return _eXosip_get_local_sdp (invite_tr);
}

sdp_message_t *
_eXosip_get_remote_sdp (osip_transaction_t * invite_tr)
{
  if (invite_tr == NULL)
    return NULL;
  if (invite_tr->ctx_type == IST || invite_tr->ctx_type == NIST) {
    sdp_message_t *sdp;
    sdp = eXosip_get_sdp_info (invite_tr->orig_request);
    if (sdp==NULL) {
      sdp = eXosip_get_sdp_info (invite_tr->ack);
    }
    return sdp;
  } 
  if (invite_tr->ctx_type == ICT || invite_tr->ctx_type == NICT)
    return eXosip_get_sdp_info (invite_tr->last_response);

  return NULL;
}

sdp_message_t *
_eXosip_get_local_sdp (osip_transaction_t * invite_tr)
{
  if (invite_tr == NULL)
    return NULL;
  if (invite_tr->ctx_type == IST || invite_tr->ctx_type == NIST)
    return eXosip_get_sdp_info (invite_tr->last_response);
  if (invite_tr->ctx_type == ICT || invite_tr->ctx_type == NICT) {
    sdp_message_t *sdp;
    sdp = eXosip_get_sdp_info (invite_tr->orig_request);
    if (sdp==NULL) {
      sdp = eXosip_get_sdp_info (invite_tr->ack);
    }
    return sdp;
  }

  return NULL;
}

sdp_message_t *
eXosip_get_sdp_info (osip_message_t * message)
{
  osip_content_type_t *ctt;
  sdp_message_t *sdp;
  osip_body_t *oldbody;
  osip_list_iterator_t it;

  if (message == NULL)
    return NULL;

  /* get content-type info */
  ctt = osip_message_get_content_type (message);
  if (ctt == NULL)
    return NULL;                /* previous message was not correct or empty */

  if (ctt->type == NULL || ctt->subtype == NULL)
    return NULL;
  if (osip_strcasecmp (ctt->type, "multipart") == 0) {
    /* probably within the multipart attachement */
  }
  else if (osip_strcasecmp (ctt->type, "application") != 0 || osip_strcasecmp (ctt->subtype, "sdp") != 0)
    return NULL;

  oldbody = (osip_body_t *)osip_list_get_first(&message->bodies, &it);
  while (oldbody != NULL) {
    int i;
    sdp_message_init (&sdp);
    i = sdp_message_parse (sdp, oldbody->body);
    if (i == 0)
      return sdp;
    sdp_message_free (sdp);
    sdp = NULL;
    oldbody = (osip_body_t *)osip_list_get_next(&it);
  }
  return NULL;
}


sdp_connection_t *
eXosip_get_audio_connection (sdp_message_t * sdp)
{
  int pos = 0;
  sdp_media_t *med = (sdp_media_t *) osip_list_get (&sdp->m_medias, 0);

  while (med != NULL) {
    if (med->m_media != NULL && osip_strcasecmp (med->m_media, "audio") == 0)
      break;
    pos++;
    med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos);
  }
  if (med == NULL)
    return NULL;                /* no audio stream */
  if (osip_list_eol (&med->c_connections, 0))
    return sdp->c_connection;

  /* just return the first one... */
  return (sdp_connection_t *) osip_list_get (&med->c_connections, 0);
}


sdp_media_t *
eXosip_get_audio_media (sdp_message_t * sdp)
{
  int pos = 0;
  sdp_media_t *med = (sdp_media_t *) osip_list_get (&sdp->m_medias, 0);

  while (med != NULL) {
    if (med->m_media != NULL && osip_strcasecmp (med->m_media, "audio") == 0)
      return med;
    pos++;
    med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos);
  }

  return NULL;
}

sdp_connection_t *
eXosip_get_video_connection (sdp_message_t * sdp)
{
  int pos = 0;
  sdp_media_t *med = (sdp_media_t *) osip_list_get (&sdp->m_medias, 0);

  while (med != NULL) {
    if (med->m_media != NULL && osip_strcasecmp (med->m_media, "video") == 0)
      break;
    pos++;
    med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos);
  }
  if (med == NULL)
    return NULL;                /* no video stream */
  if (osip_list_eol (&med->c_connections, 0))
    return sdp->c_connection;

  /* just return the first one... */
  return (sdp_connection_t *) osip_list_get (&med->c_connections, 0);
}


sdp_media_t *
eXosip_get_video_media (sdp_message_t * sdp)
{
  int pos = 0;
  sdp_media_t *med = (sdp_media_t *) osip_list_get (&sdp->m_medias, 0);

  while (med != NULL) {
    if (med->m_media != NULL && osip_strcasecmp (med->m_media, "video") == 0)
      return med;
    pos++;
    med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos);
  }

  return NULL;
}

sdp_connection_t *
eXosip_get_connection (sdp_message_t * sdp, const char *media)
{
  int pos = 0;
  sdp_media_t *med = (sdp_media_t *) osip_list_get (&sdp->m_medias, 0);

  while (med != NULL) {
    if (med->m_media != NULL && osip_strcasecmp (med->m_media, media) == 0)
      break;
    pos++;
    med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos);
  }
  if (med == NULL)
    return NULL;                /* no video stream */
  if (osip_list_eol (&med->c_connections, 0))
    return sdp->c_connection;

  /* just return the first one... */
  return (sdp_connection_t *) osip_list_get (&med->c_connections, 0);
}

sdp_media_t *
eXosip_get_media (sdp_message_t * sdp, const char *media)
{
  int pos = 0;
  sdp_media_t *med = (sdp_media_t *) osip_list_get (&sdp->m_medias, 0);

  while (med != NULL) {
    if (med->m_media != NULL && osip_strcasecmp (med->m_media, media) == 0)
      return med;
    pos++;
    med = (sdp_media_t *) osip_list_get (&sdp->m_medias, pos);
  }

  return NULL;
}
