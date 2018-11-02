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

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef __APPLE_CC__
#include <unistd.h>
#endif
#else
#include <windows.h>
#endif

/* Private functions */
static int dialog_fill_route_set (osip_dialog_t * dialog, osip_message_t * request);

/* should use cryptographically random identifier is RECOMMENDED.... */
/* by now this should lead to identical call-id when application are
   started at the same time...   */
char *
_eXosip_malloc_new_random ()
{
  char *tmp = (char *) osip_malloc (33);
  unsigned int number = osip_build_random_number ();

  if (tmp == NULL)
    return NULL;

  sprintf (tmp, "%u", number);
  return tmp;
}

int
eXosip_generate_random (char *buf, int buf_size)
{
  unsigned int number = osip_build_random_number ();

  snprintf (buf, buf_size, "%u", number);
  return OSIP_SUCCESS;
}

int
_eXosip_dialog_add_contact (struct eXosip_t *excontext, osip_message_t * request)
{
  osip_from_t *a_from;
  char *contact = NULL;
  char scheme[10];
  int len;

  if (excontext->eXtl_transport.enabled <= 0)
    return OSIP_NO_NETWORK;
  if (request == NULL)
    return OSIP_BADPARAMETER;

  a_from = request->from;

  if (a_from == NULL || a_from->url == NULL)
    return OSIP_SYNTAXERROR;

  /* rfc3261: 8.1.1.8 Contact */
  if (request->req_uri!=NULL && request->req_uri->scheme!=NULL && osip_strcasecmp(request->req_uri->scheme, "sips")==0)
    snprintf(scheme, sizeof(scheme), "sips");
  else
    snprintf(scheme, sizeof(scheme), "sip");

  if (a_from->url->username != NULL)
    len = (int) (2 + 4 + (strlen (a_from->url->username) * 3) + 1 + 100 + 6 + 10 + 3 + strlen (excontext->transport));
  else
    len = (int) (2 + 4 + 100 + 6 + 10 + 3 + strlen (excontext->transport));

  len++; /* if using sips instead of sip */

  if (excontext->sip_instance[0] != 0)
    len+=65;

  contact = (char *) osip_malloc (len + 1);
  if (contact == NULL)
    return OSIP_NOMEM;

  
  /* special values to be replaced in transport layer (eXtl_*.c files) */
  if (a_from->url->username != NULL) {
    char *tmp2 = __osip_uri_escape_userinfo (a_from->url->username);

    snprintf (contact, len, "<%s:%s@999.999.999.999:99999>", scheme, tmp2);
    osip_free (tmp2);
  }
  else
    snprintf (contact, len - strlen (excontext->transport) - 10, "<%s:999.999.999.999:99999>", scheme);

  if (excontext->enable_outbound==1) {
    contact[strlen (contact) - 1] = '\0';
    strcat (contact, ";ob");
    strcat (contact, ">");
  }
  if (osip_strcasecmp (excontext->transport, "UDP") != 0) {
    contact[strlen (contact) - 1] = '\0';
    strcat (contact, ";transport=");
    strcat (contact, excontext->transport);
    strcat (contact, ">");
  }
  if (excontext->sip_instance[0] != 0) {
    strcat(contact, ";+sip.instance=\"<urn:uuid:");
    strcat(contact, excontext->sip_instance);
    strcat(contact, ">\"");
  }

  osip_message_set_contact (request, contact);
  osip_free (contact);

  if (excontext->default_contact_displayname[0]!='\0') {
    osip_contact_t *new_contact;
    osip_message_get_contact(request, 0, &new_contact);
    if (new_contact!=NULL) {
      new_contact->displayname = osip_strdup (excontext->default_contact_displayname);
    }
  }

  if (excontext->eXtl_transport._tl_update_contact!=NULL)
    excontext->eXtl_transport._tl_update_contact(excontext, request);
  return OSIP_SUCCESS;
}

int
_eXosip_request_add_via (struct eXosip_t *excontext, osip_message_t * request)
{
  char tmp[200];

  if (excontext->eXtl_transport.enabled <= 0)
    return OSIP_NO_NETWORK;

  if (request == NULL)
    return OSIP_BADPARAMETER;

  if (request->call_id == NULL)
    return OSIP_SYNTAXERROR;

  /* special values to be replaced in transport layer (eXtl_*.c files) */
  if (excontext->use_rport != 0 && excontext->eXtl_transport.proto_family == AF_INET)
    snprintf (tmp, 200, "SIP/2.0/%s 999.999.999.999:99999;rport;branch=z9hG4bK%u", excontext->transport, osip_build_random_number ());
  else
    snprintf (tmp, 200, "SIP/2.0/%s 999.999.999.999:99999;branch=z9hG4bK%u", excontext->transport, osip_build_random_number ());

  osip_message_set_via (request, tmp);

  return OSIP_SUCCESS;
}

/* prepare a minimal request (outside of a dialog) with required headers */
/* 
   method is the type of request. ("INVITE", "REGISTER"...)
   to is the remote target URI
   transport is either "TCP" or "UDP" (by now, only UDP is implemented!)
*/
int
_eXosip_generating_request_out_of_dialog (struct eXosip_t *excontext, osip_message_t ** dest, const char *method, const char *to, const char *from, const char *proxy)
{
  /* Section 8.1:
     A valid request contains at a minimum "To, From, Call-iD, Cseq,
     Max-Forwards and Via
   */
  int i;
  osip_message_t *request;
  int doing_register;

  *dest = NULL;

  if (!method || !*method)
    return OSIP_BADPARAMETER;

  if (excontext->eXtl_transport.enabled <= 0)
    return OSIP_NO_NETWORK;

  i = osip_message_init (&request);
  if (i != 0)
    return i;

  /* prepare the request-line */
  osip_message_set_method (request, osip_strdup (method));
  osip_message_set_version (request, osip_strdup ("SIP/2.0"));
  osip_message_set_status_code (request, 0);
  osip_message_set_reason_phrase (request, NULL);

  doing_register = 0 == strcmp ("REGISTER", method);

  if (doing_register) {
    i = osip_uri_init (&(request->req_uri));
    if (i != 0) {
      osip_message_free (request);
      return i;
    }
    i = osip_uri_parse (request->req_uri, proxy);
    if (i != 0) {
      osip_message_free (request);
      return i;
    }
    i = osip_message_set_to (request, from);
    if (i != 0 || request->to == NULL) {
      if (i >= 0)
        i = OSIP_SYNTAXERROR;
      osip_message_free (request);
      return i;
    }

    /* REMOVE ALL URL PARAMETERS from to->url headers and add them as headers */
    if (request->to != NULL && request->to->url != NULL) {
      osip_uri_t *url = request->to->url;

      while (osip_list_size (&url->url_headers) > 0) {
        osip_uri_header_t *u_header;

        u_header = (osip_uri_param_t *) osip_list_get (&url->url_headers, 0);
        if (u_header == NULL)
          break;

        if (osip_strcasecmp (u_header->gname, "from") == 0) {
        }
        else if (osip_strcasecmp (u_header->gname, "to") == 0) {
        }
        else if (osip_strcasecmp (u_header->gname, "call-id") == 0) {
        }
        else if (osip_strcasecmp (u_header->gname, "cseq") == 0) {
        }
        else if (osip_strcasecmp (u_header->gname, "via") == 0) {
        }
        else if (osip_strcasecmp (u_header->gname, "contact") == 0) {
        }
        else if (osip_strcasecmp (u_header->gname, "route") == 0) {
          osip_message_set_route (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "call-info") == 0) {
          osip_message_set_call_info (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "accept") == 0) {
          osip_message_set_accept (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "accept-encoding") == 0) {
          osip_message_set_accept_encoding (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "accept-language") == 0) {
          osip_message_set_accept_language (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "alert-info") == 0) {
          osip_message_set_alert_info (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "allow") == 0) {
          osip_message_set_allow (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "content-type") == 0) {
          osip_message_set_content_type (request, u_header->gvalue);
        }
        else
          osip_message_set_header (request, u_header->gname, u_header->gvalue);
        osip_list_remove (&url->url_headers, 0);
        osip_uri_param_free (u_header);
      }
    }
  }
  else {
    /* in any cases except REGISTER: */
    i = osip_message_set_to (request, to);
    if (i != 0 || request->to == NULL) {
      if (i >= 0)
        i = OSIP_SYNTAXERROR;
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "ERROR: callee address does not seems to be a sipurl: %s\n", to));
      osip_message_free (request);
      return i;
    }

    /* REMOVE ALL URL PARAMETERS from to->url headers and add them as headers */
    if (request->to != NULL && request->to->url != NULL) {
      osip_uri_t *url = request->to->url;

      while (osip_list_size (&url->url_headers) > 0) {
        osip_uri_header_t *u_header;

        u_header = (osip_uri_param_t *) osip_list_get (&url->url_headers, 0);
        if (u_header == NULL)
          break;

        if (osip_strcasecmp (u_header->gname, "from") == 0) {
        }
        else if (osip_strcasecmp (u_header->gname, "to") == 0) {
        }
        else if (osip_strcasecmp (u_header->gname, "call-id") == 0) {
        }
        else if (osip_strcasecmp (u_header->gname, "cseq") == 0) {
        }
        else if (osip_strcasecmp (u_header->gname, "via") == 0) {
        }
        else if (osip_strcasecmp (u_header->gname, "contact") == 0) {
        }
        else if (osip_strcasecmp (u_header->gname, "route") == 0) {
          osip_message_set_route (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "call-info") == 0) {
          osip_message_set_call_info (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "accept") == 0) {
          osip_message_set_accept (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "accept-encoding") == 0) {
          osip_message_set_accept_encoding (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "accept-language") == 0) {
          osip_message_set_accept_language (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "alert-info") == 0) {
          osip_message_set_alert_info (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "allow") == 0) {
          osip_message_set_allow (request, u_header->gvalue);
        }
        else if (osip_strcasecmp (u_header->gname, "content-type") == 0) {
          osip_message_set_content_type (request, u_header->gvalue);
        }
        else
          osip_message_set_header (request, u_header->gname, u_header->gvalue);
        osip_list_remove (&url->url_headers, 0);
        osip_uri_param_free (u_header);
      }
    }

    if (proxy != NULL && proxy[0] != 0) {       /* equal to a pre-existing route set */
      /* if the pre-existing route set contains a "lr" (compliance
         with bis-08) then the req_uri should contains the remote target
         URI */
      osip_uri_param_t *lr_param;
      osip_route_t *o_proxy;

      osip_route_init (&o_proxy);
      i = osip_route_parse (o_proxy, proxy);
      if (i != 0) {
        osip_route_free (o_proxy);
        osip_message_free (request);
        return i;
      }

      osip_uri_uparam_get_byname (o_proxy->url, "lr", &lr_param);
      if (lr_param != NULL) {   /* to is the remote target URI in this case! */
        i = osip_uri_clone (request->to->url, &(request->req_uri));
        if (i != 0) {
          osip_route_free (o_proxy);
          osip_message_free (request);
          return i;
        }

        /* "[request] MUST includes a Route header field containing
           the route set values in order." */
        osip_list_add (&request->routes, o_proxy, 0);
      }
      else
        /* if the first URI of route set does not contain "lr", the req_uri
           is set to the first uri of route set */
      {
        request->req_uri = o_proxy->url;
        o_proxy->url = NULL;
        osip_route_free (o_proxy);
        /* add the route set */
        /* "The UAC MUST add a route header field containing
           the remainder of the route set values in order.
           The UAC MUST then place the remote target URI into
           the route header field as the last value
         */
        osip_message_set_route (request, to);
      }
    }
    else {                      /* No route set (outbound proxy) is used */

      /* The UAC must put the remote target URI (to field) in the req_uri */
      i = osip_uri_clone (request->to->url, &(request->req_uri));
      if (i != 0) {
        osip_message_free (request);
        return i;
      }
    }
  }

  /* set To and From */
  i = osip_message_set_from (request, from);
  if (i != 0 || request->from == NULL) {
    if (i >= 0)
      i = OSIP_SYNTAXERROR;
    osip_message_free (request);
    return i;
  }

  /* REMOVE ALL URL PARAMETERS from from->url headers and add them as headers */
  if (doing_register && request->from != NULL && request->from->url != NULL) {
    osip_uri_t *url = request->from->url;

    while (osip_list_size (&url->url_headers) > 0) {
      osip_uri_header_t *u_header;

      u_header = (osip_uri_param_t *) osip_list_get (&url->url_headers, 0);
      if (u_header == NULL)
        break;

      osip_list_remove (&url->url_headers, 0);
      osip_uri_param_free (u_header);
    }
  }

  if (request->to != NULL && request->to->url != NULL) {
    osip_list_iterator_t it;
    osip_uri_param_t* u_param = (osip_uri_param_t*)osip_list_get_first(&request->to->url->url_params, &it);

    while (u_param != NULL) {
      if (u_param->gvalue != NULL && u_param->gname!=NULL && osip_strcasecmp (u_param->gname, "method") == 0) {
        osip_list_iterator_remove(&it);
        osip_uri_param_free (u_param);
        break;
      }
      u_param = (osip_uri_param_t *)osip_list_get_next(&it);
    }
  }

  if (request->from != NULL && request->from->url != NULL) {
    osip_list_iterator_t it;
    osip_uri_param_t* u_param = (osip_uri_param_t*)osip_list_get_first(&request->from->url->url_params, &it);

    while (u_param != NULL) {
      if (u_param->gvalue != NULL && u_param->gname!=NULL && osip_strcasecmp (u_param->gname, "method") == 0) {
        osip_list_iterator_remove(&it);
        osip_uri_param_free (u_param);
        break;
      }
      u_param = (osip_uri_param_t *)osip_list_get_next(&it);
    }
  }

  if (request->req_uri) {
    osip_list_iterator_t it;
    osip_uri_param_t* u_param = (osip_uri_param_t*)osip_list_get_first(&request->req_uri->url_params, &it);

    while (u_param != NULL) {
      if (u_param->gvalue != NULL && u_param->gname!=NULL && osip_strcasecmp (u_param->gname, "method") == 0) {
        osip_list_iterator_remove(&it);
        osip_uri_param_free (u_param);
        break;
      }
      u_param = (osip_uri_param_t *)osip_list_get_next(&it);
    }
  }

  /* add a tag */
  osip_from_set_tag (request->from, _eXosip_malloc_new_random ());

  /* set the cseq and call_id header */
  {
    osip_call_id_t *callid;
    osip_cseq_t *cseq;
    char *num;
    char *cidrand;

    /* call-id is always the same for REGISTRATIONS */
    i = osip_call_id_init (&callid);
    if (i != 0) {
      osip_message_free (request);
      return i;
    }
    cidrand = _eXosip_malloc_new_random ();
    osip_call_id_set_number (callid, cidrand);

    request->call_id = callid;

    i = osip_cseq_init (&cseq);
    if (i != 0) {
      osip_message_free (request);
      return i;
    }
    num = osip_strdup (doing_register ? "1" : "20");
    osip_cseq_set_number (cseq, num);
    osip_cseq_set_method (cseq, osip_strdup (method));
    request->cseq = cseq;

    if (cseq->method == NULL || cseq->number == NULL) {
      osip_message_free (request);
      return OSIP_NOMEM;
    }
  }

  i = _eXosip_request_add_via (excontext, request);
  if (i != 0) {
    osip_message_free (request);
    return i;
  }

  /* always add the Max-Forward header */
  osip_message_set_max_forwards (request, "70");        /* a UA should start a request with 70 */

  if (0 == strcmp ("REGISTER", method)) {
  }
  else if (0 == strcmp ("INFO", method)) {
  }
  else if (0 == strcmp ("OPTIONS", method)) {
    osip_message_set_accept (request, "application/sdp");
  }

  osip_message_set_user_agent (request, excontext->user_agent);
  /*  else if ... */
  *dest = request;
  return OSIP_SUCCESS;
}

#ifndef MINISIZE

int
_eXosip_generating_publish (struct eXosip_t *excontext, osip_message_t ** message, const char *to, const char *from, const char *route)
{
  int i;

  if (to != NULL && *to == '\0')
    return OSIP_BADPARAMETER;

  if (route != NULL && *route == '\0')
    route = NULL;

  i = _eXosip_generating_request_out_of_dialog (excontext, message, "PUBLISH", to, from, route);
  if (i != 0)
    return i;

  if (excontext->sip_instance[0]!= 0)
    _eXosip_dialog_add_contact(excontext, *message);
  /* osip_message_set_organization(*message, "Jack's Org"); */

  return OSIP_SUCCESS;
}

#endif

static int
dialog_fill_route_set (osip_dialog_t * dialog, osip_message_t * request)
{
  /* if the pre-existing route set contains a "lr" (compliance
     with bis-08) then the req_uri should contains the remote target
     URI */
  osip_list_iterator_t it;
  int i;
  osip_uri_param_t *lr_param;
  osip_route_t *route;

  route = (osip_route_t*)osip_list_get_first(&dialog->route_set, &it);

  osip_uri_uparam_get_byname (route->url, "lr", &lr_param);
  if (lr_param != NULL) {       /* the remote target URI is the req_uri! */
    i = osip_uri_clone (dialog->remote_contact_uri->url, &(request->req_uri));
    if (i != 0)
      return i;
    /* "[request] MUST includes a Route header field containing
       the route set values in order." */

    while (route != NULL) {
      osip_route_t *route2;

      i = osip_route_clone (route, &route2);
      if (i != 0)
        return i;
      osip_list_add (&request->routes, route2, -1);
      route = (osip_route_t*)osip_list_get_next(&it);
    }
    return OSIP_SUCCESS;
  }

  /* if the first URI of route set does not contain "lr", the req_uri
     is set to the first uri of route set */


  i = osip_uri_clone (route->url, &(request->req_uri));
  if (i != 0)
    return i;
  /* add the route set */
  /* "The UAC MUST add a route header field containing
     the remainder of the route set values in order. */
  route = (osip_route_t*)osip_list_get_next(&it);  /* yes it is, skip first */

  while (route != NULL) {
    osip_route_t *route2;

    i = osip_route_clone (route, &route2);
    if (i != 0)
      return i;
    osip_list_add (&request->routes, route2, -1);
    route = (osip_route_t*)osip_list_get_next(&it);
  }

  /* The UAC MUST then place the remote target URI into
     the route header field as the last value */
  {
    osip_uri_t *new_url;
    /* Feb 05, 2016: old code was converting contact's URI's uri-param into Route route-param */
    i = osip_uri_clone(dialog->remote_contact_uri->url, &new_url);
    if (i != 0)
      return i;
    i = osip_route_init(&route);
    if (i != 0) {
      osip_uri_free(new_url);
      return i;
    }
    osip_uri_free(route->url);
    route->url = new_url;
  }

  /* route header and req_uri set */
  return OSIP_SUCCESS;
}

int
_eXosip_build_request_within_dialog (struct eXosip_t *excontext, osip_message_t ** dest, const char *method, osip_dialog_t * dialog)
{
  int i;
  osip_message_t *request;

  *dest = NULL;

  if (dialog == NULL)
    return OSIP_BADPARAMETER;

  if (excontext->eXtl_transport.enabled <= 0)
    return OSIP_NO_NETWORK;

  i = osip_message_init (&request);
  if (i != 0)
    return i;

  if (dialog->remote_contact_uri == NULL) {
    /* this dialog is probably not established! or the remote UA
       is not compliant with the latest RFC
     */
    osip_message_free (request);
    return OSIP_SYNTAXERROR;
  }

  /* prepare the request-line */
  request->sip_method = osip_strdup (method);
  if (request->sip_method == NULL) {
    osip_message_free (request);
    return OSIP_NOMEM;
  }
  request->sip_version = osip_strdup ("SIP/2.0");
  if (request->sip_version == NULL) {
    osip_message_free (request);
    return OSIP_NOMEM;
  }
  request->status_code = 0;
  request->reason_phrase = NULL;

  /* and the request uri???? */
  if (osip_list_eol (&dialog->route_set, 0)) {
    /* The UAC must put the remote target URI (to field) in the req_uri */
    i = osip_uri_clone (dialog->remote_contact_uri->url, &(request->req_uri));
    if (i != 0) {
      osip_message_free (request);
      return i;
    }
  }
  else {
    /* fill the request-uri, and the route headers. */
    i = dialog_fill_route_set (dialog, request);
    if (i != 0) {
      osip_message_free (request);
      return i;
    }
  }

  /* To and From already contains the proper tag! */
  i = osip_to_clone (dialog->remote_uri, &(request->to));
  if (i != 0) {
    osip_message_free (request);
    return i;
  }
  i = osip_from_clone (dialog->local_uri, &(request->from));
  if (i != 0) {
    osip_message_free (request);
    return i;
  }

  /* set the cseq and call_id header */
  osip_message_set_call_id (request, dialog->call_id);

  if (0 == strcmp ("ACK", method)) {
    osip_cseq_t *cseq;
    char *tmp;

    i = osip_cseq_init (&cseq);
    if (i != 0) {
      osip_message_free (request);
      return i;
    }
    tmp = osip_malloc (20);
    if (tmp == NULL) {
      osip_message_free (request);
      return OSIP_NOMEM;
    }
    sprintf (tmp, "%i", dialog->local_cseq);
    osip_cseq_set_number (cseq, tmp);
    osip_cseq_set_method (cseq, osip_strdup (method));
    request->cseq = cseq;
  }
  else {
    osip_cseq_t *cseq;
    char *tmp;

    i = osip_cseq_init (&cseq);
    if (i != 0) {
      osip_message_free (request);
      return i;
    }
    dialog->local_cseq++;       /* we should we do that?? */
    tmp = osip_malloc (20);
    if (tmp == NULL) {
      osip_message_free (request);
      return OSIP_NOMEM;
    }
    snprintf (tmp, 20, "%i", dialog->local_cseq);
    osip_cseq_set_number (cseq, tmp);
    osip_cseq_set_method (cseq, osip_strdup (method));
    request->cseq = cseq;
  }

  /* always add the Max-Forward header */
  osip_message_set_max_forwards (request, "70");        /* a UA should start a request with 70 */

  i = _eXosip_request_add_via (excontext, request);
  if (i != 0) {
    osip_message_free (request);
    return i;
  }

  /* add specific headers for each kind of request... */
  if ((0 != strcmp ("BYE", method)) && (0 != strcmp ("CANCEL", method)))
  {
    _eXosip_dialog_add_contact (excontext, request);
  }

  if (0 == strcmp ("NOTIFY", method)) {
  }
  else if (0 == strcmp ("INFO", method)) {

  }
  else if (0 == strcmp ("OPTIONS", method)) {
    osip_message_set_accept (request, "application/sdp");
  }
  else if (0 == strcmp ("ACK", method)) {
    /* The ACK MUST contains the same credential than the INVITE!! */
    /* TODO... */
  }

  osip_message_set_user_agent (request, excontext->user_agent);
  /*  else if ... */
  *dest = request;
  return OSIP_SUCCESS;
}

/* this request is only build within a dialog!! */
int
_eXosip_generating_bye (struct eXosip_t *excontext, osip_message_t ** bye, osip_dialog_t * dialog)
{
  int i;

  i = _eXosip_build_request_within_dialog (excontext, bye, "BYE", dialog);
  if (i != 0)
    return i;

  return OSIP_SUCCESS;
}

/* It is RECOMMENDED to only cancel INVITE request */
int
_eXosip_generating_cancel (struct eXosip_t *excontext, osip_message_t ** dest, osip_message_t * request_cancelled)
{
  int i;
  osip_message_t *request;

  i = osip_message_init (&request);
  if (i != 0)
    return i;

  /* prepare the request-line */
  osip_message_set_method (request, osip_strdup ("CANCEL"));
  osip_message_set_version (request, osip_strdup ("SIP/2.0"));
  osip_message_set_status_code (request, 0);
  osip_message_set_reason_phrase (request, NULL);

  i = osip_uri_clone (request_cancelled->req_uri, &(request->req_uri));
  if (i != 0) {
    osip_message_free (request);
    *dest = NULL;
    return i;
  }

  i = osip_to_clone (request_cancelled->to, &(request->to));
  if (i != 0) {
    osip_message_free (request);
    *dest = NULL;
    return i;
  }
  i = osip_from_clone (request_cancelled->from, &(request->from));
  if (i != 0) {
    osip_message_free (request);
    *dest = NULL;
    return i;
  }

  /* set the cseq and call_id header */
  i = osip_call_id_clone (request_cancelled->call_id, &(request->call_id));
  if (i != 0) {
    osip_message_free (request);
    *dest = NULL;
    return i;
  }
  i = osip_cseq_clone (request_cancelled->cseq, &(request->cseq));
  if (i != 0) {
    osip_message_free (request);
    *dest = NULL;
    return i;
  }
  osip_free (request->cseq->method);
  request->cseq->method = osip_strdup ("CANCEL");
  if (request->cseq->method == NULL) {
    osip_message_free (request);
    *dest = NULL;
    return OSIP_NOMEM;
  }

  /* copy ONLY the top most Via Field (this method is also used by proxy) */
  {
    osip_via_t *via;
    osip_via_t *via2;

    i = osip_message_get_via (request_cancelled, 0, &via);
    if (i < 0) {
      osip_message_free (request);
      *dest = NULL;
      return i;
    }
    i = osip_via_clone (via, &via2);
    if (i != 0) {
      osip_message_free (request);
      *dest = NULL;
      return i;
    }
    osip_list_add (&request->vias, via2, -1);
  }

  /* add the same route-set than in the previous request */
  {
    osip_list_iterator_t it;
    osip_route_t* route = (osip_route_t*)osip_list_get_first(&request_cancelled->routes, &it);
    osip_route_t *route2;

    while (route != NULL) {
      i = osip_route_clone (route, &route2);
      if (i != 0) {
        osip_message_free (request);
        *dest = NULL;
        return i;
      }
      osip_list_add (&request->routes, route2, -1);
      route = (osip_route_t *)osip_list_get_next(&it);
    }
  }

  osip_message_set_max_forwards (request, "70");        /* a UA should start a request with 70 */
  osip_message_set_user_agent (request, excontext->user_agent);

  *dest = request;
  return OSIP_SUCCESS;
}

int
_eXosip_request_viamanager(struct eXosip_t *excontext, osip_transaction_t * tr, osip_message_t * sip, int proto, struct sockaddr_storage *udp_local_bind, int ephemeral_port, int tcp_sock, char *host)
{
  /* step1: put local-ip in VIA->host or udp_firewall_ip set by eXosip_masquerade_contact (tl_get_masquerade_contact) */
  /* step2: put local-port in VIA->port or udp_firewall_port set by eXosip_masquerade_contact (tl_get_masquerade_contact) */
  /* step3: #ifdef MASQUERADE_VIA */
  /* step4: put firewall-port in VIA->port, excontext->masquerade_via (udp_tl_update_contact)*/
  /* step5: put firewall ip in VIA->host , excontext->masquerade_via (udp_tl_update_contact)*/
  
  char masquerade_ip[65];
  char masquerade_port[10];
  osip_via_t *via;
  char *via_ip=NULL;
  char *via_port=NULL;

  if (MSG_IS_RESPONSE(sip))
    return OSIP_SUCCESS; /* not needed */

  via = (osip_via_t *)osip_list_get(&sip->vias, 0);
  if (via==NULL || via->host==NULL)
    return OSIP_SYNTAXERROR;
  if (osip_strcasecmp(via->host, "999.999.999.999")!=0 && via->port!=NULL && osip_strcasecmp(via->port, "99999")!=0)
    return OSIP_SUCCESS;

  masquerade_ip[0] = '\0';
  masquerade_port[0] = '\0';
  if (excontext->eXtl_transport.tl_get_masquerade_contact != NULL) {
    excontext->eXtl_transport.tl_get_masquerade_contact (excontext, masquerade_ip, sizeof (masquerade_ip), masquerade_port, sizeof (masquerade_port));
  }
  if (masquerade_port[0]!='\0') {
    via_port = masquerade_port;
  }
  if (via_port==NULL && ephemeral_port>0) {
    snprintf(masquerade_port, sizeof(masquerade_port), "%i", ephemeral_port);
    via_port = masquerade_port;
  }
  if (via_port==NULL && excontext->eXtl_transport.proto_local_port>0) {
    snprintf(masquerade_port, sizeof(masquerade_port), "%i", excontext->eXtl_transport.proto_local_port);
    via_port = masquerade_port;
  }
  
#ifdef MASQUERADE_VIA
  if (masquerade_ip[0]!='\0') {
    via_ip = masquerade_ip;
  } else 
#endif
  if (excontext->masquerade_via>0) {
    if (masquerade_ip[0]!='\0') {
      via_ip = masquerade_ip;
    }
  }

  if (via_ip==NULL) {
    masquerade_ip[0] = '\0';
    _eXosip_guess_ip_for_destinationsock (excontext, excontext->eXtl_transport.proto_family, proto, udp_local_bind, tcp_sock, host, masquerade_ip, 49);
    if (masquerade_ip[0] != '\0') {
      via_ip = masquerade_ip;
    }
  }

  if (via_ip==NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "missing ip for Via header\n"));
  }
  if (via_port==NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "missing port for Via header\n"));
  }
  if (via_ip==NULL || via_port==NULL) {
    return OSIP_UNDEFINED_ERROR;
  }

  if (osip_strcasecmp(via->host, "999.999.999.999")==0) {
    osip_free(via->host);
    via->host = osip_strdup(via_ip);
  }

  if (via->port!=NULL && osip_strcasecmp(via->port, "99999")==0) {
    osip_free(via->port);
    via->port = osip_strdup(via_port);
  }
  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "updating: Via header to %s:%s\n", via_ip, via_port));
  osip_message_force_update(sip);
  return OSIP_SUCCESS;
}

int
_eXosip_message_contactmanager(struct eXosip_t *excontext, osip_transaction_t * tr, osip_message_t * sip, int proto, struct sockaddr_storage *udp_local_bind, int ephemeral_port, int sock, char *host)
{
  /* step1: put local-ip in Contact ->host or udp_firewall_ip set by eXosip_masquerade_contact (_eXosip_register_add_contact) */
  /* step2: put local-port in Contact->port or udp_firewall_port set by eXosip_masquerade_contact (_eXosip_register_add_contact) */
  /* step3: #ifdef USE_LOCALIP_WITH_LOCALPROXY (localip instead of masquerade IP) */
  char locip[65];
  char masquerade_ip[65];
  char masquerade_port[10];
  osip_contact_t *acontact;
  char *contact_ip=NULL;
  char *contact_port=NULL;

  acontact = (osip_contact_t *)osip_list_get(&sip->contacts, 0);
  if (acontact==NULL || acontact->url==NULL || acontact->url->host==NULL)
    return OSIP_SUCCESS;
  if (osip_strcasecmp(acontact->url->host, "999.999.999.999")!=0 && acontact->url->port!=NULL && osip_strcasecmp(acontact->url->port, "99999")!=0)
    return OSIP_SUCCESS;


  /* firewall ip & firewall port configured by APPLICATION layer */
  masquerade_ip[0] = '\0';
  masquerade_port[0] = '\0';
  if (excontext->eXtl_transport.tl_get_masquerade_contact != NULL) {
    excontext->eXtl_transport.tl_get_masquerade_contact (excontext, masquerade_ip, sizeof (masquerade_ip), masquerade_port, sizeof (masquerade_port));
  }

  if (masquerade_port[0]!='\0') {
    contact_port = masquerade_port;
  }
  if (contact_port==NULL && ephemeral_port>0) {
    snprintf(masquerade_port, sizeof(masquerade_port), "%i", ephemeral_port);
    contact_port = masquerade_port;
  }
  if (contact_port==NULL && excontext->eXtl_transport.proto_local_port>0) {
    snprintf(masquerade_port, sizeof(masquerade_port), "%i", excontext->eXtl_transport.proto_local_port);
    contact_port = masquerade_port;
  }

  if (masquerade_ip[0]!='\0') {
    contact_ip = masquerade_ip;
  } 

  locip[0] = '\0';
  _eXosip_guess_ip_for_destinationsock (excontext, excontext->eXtl_transport.proto_family, proto, udp_local_bind, sock, host, locip, 49);
  if (locip[0] == '\0') {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: no network interface found\n"));
    return OSIP_NO_NETWORK;
  }

  /* search for correct ip */
  if (masquerade_ip[0] != '\0' && sip->req_uri != NULL && sip->req_uri->host != NULL) {
#ifdef USE_LOCALIP_WITH_LOCALPROXY      /* disable this code for local testing because it adds an extra DNS */
    char *c_address = sip->req_uri->host;

    struct addrinfo *addrinfo;
    struct __eXosip_sockaddr addr;

    i = _eXosip_get_addrinfo (excontext, &addrinfo, sip->req_uri->host, 5060, IPPROTO_UDP);
    if (i == 0) {
      memcpy (&addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
      _eXosip_freeaddrinfo (addrinfo);
      c_address = inet_ntoa (((struct sockaddr_in *) &addr)->sin_addr);
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip: here is the resolved destination host=%s\n", c_address));
    }

    if (_eXosip_is_public_address (c_address)) {
      contact_ip = masquerade_ip;
    }
    else {
      contact_ip = locip;
    }
#else
    contact_ip = masquerade_ip;
#endif
  }
  
  if (contact_ip==NULL || contact_ip[0]=='\0') {
    contact_ip = locip;
  }

  if (contact_ip==NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "missing ip for Contact header\n"));
  }
  if (contact_port==NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "missing port for Contact header\n"));
  }
  if (contact_ip==NULL || contact_port==NULL) {
    return OSIP_UNDEFINED_ERROR;
  }

  if (osip_strcasecmp(acontact->url->host, "999.999.999.999")==0) {
    osip_free(acontact->url->host);
    acontact->url->host = osip_strdup(contact_ip);
  }

  if (acontact->url->port!=NULL && osip_strcasecmp(acontact->url->port, "99999")==0) {
    osip_free(acontact->url->port);
    acontact->url->port = osip_strdup(contact_port);
  }
  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "updating: Contact header to %s:%s\n", contact_ip, contact_port));
  osip_message_force_update(sip);
  return OSIP_SUCCESS;
}


