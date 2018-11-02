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


#ifndef _OSIP_CONST_H_
#define _OSIP_CONST_H_

#ifdef ENABLE_MPATROL
#include <mpatrol.h>
#endif

#define CRLF "\r\n\0"
#define CR   "\r\0"
#define LF   "\n\0"
#define SP   " \0"


#define ACCEPT			"accept"
#define ACCEPT_ENCODING		"accept-encoding"
#define ACCEPT_LANGUAGE		"accept-language"
#define ALERT_INFO		"alert-info"
#define ALLOW			"allow"
#define AUTHENTICATION_INFO	"authentication-info"
#define AUTHORIZATION		"authorization"
#define CALL_ID			"call-id"
#define CALL_ID_SHORT		"i"
#define CALL_INFO		"call-info"
#define CONTACT			"contact"
#define CONTACT_SHORT		"m"
#define CONTENT_DISPOSITION	"content-disposition"
#define CONTENT_ENCODING_SHORT	"e"
#define CONTENT_ENCODING	"content-encoding"
#define CONTENT_LANGUAGE	"content-language"
#define CONTENT_LENGTH_SHORT	"l"
#define CONTENT_LENGTH		"content-length"
#define CONTENT_TYPE_SHORT	"c"
#define CONTENT_TYPE		"content-type"
#define CSEQ			"cseq"
#define SIPDATE			"date"
#define ERROR_INFO		"error-info"
#define EXPIRES			"expires"
#define FROM			"from"
#define FROM_SHORT		"f"
#define IN_REPLY_TO		"in-reply-to"
#define MAX_FORWARDS		"max-forwards"
#define MIME_VERSION		"mime-version"
#define MIN_EXPIRES		"min-expires"
#define ORGANIZATION		"organization"
#define PRIORITY		"priority"
#define PROXY_AUTHENTICATE	"proxy-authenticate"
#define PROXY_AUTHENTICATION_INFO	"proxy-authentication-info"
#define PROXY_AUTHORIZATION	"proxy-authorization"
#define PROXY_REQUIRE		"proxy-require"
#define RECORD_ROUTE		"record-route"
#define REPLY_TO		"reply-to"
#define REQUIRE			"require"
#define RETRY_AFTER		"retry-after"
#define ROUTE			"route"
#define SERVER			"server"
#define SUBJECT			"subject"
#define SUBJECT_SHORT		"s"
#define SUPPORTED		"supported"
#define TIMESTAMP		"timestamp"
#define TO			"to"
#define TO_SHORT		"t"
#define UNSUPPORTED		"unsupported"
#define USER_AGENT		"user-agent"
#define VIA			"via"
#define VIA_SHORT		"v"
#define WARNING			"warning"
#define WWW_AUTHENTICATE	"www-authenticate"


#define RESPONSE_CODES 51

#define SIP_TRYING                        100
#define SIP_RINGING                       180
#define SIP_CALL_IS_BEING_FORWARDED       181
#define SIP_QUEUED                        182
#define SIP_SESSION_PROGRESS              183
#define SIP_OK                            200
#define SIP_ACCEPTED                      202
#define SIP_MULTIPLE_CHOICES              300
#define SIP_MOVED_PERMANENTLY             301
#define SIP_MOVED_TEMPORARILY             302
#define SIP_USE_PROXY                     305
#define SIP_ALTERNATIVE_SERVICE           380
#define SIP_BAD_REQUEST                   400
#define SIP_UNAUTHORIZED                  401
#define SIP_PAYMENT_REQUIRED              402
#define SIP_FORBIDDEN                     403
#define SIP_NOT_FOUND                     404
#define SIP_METHOD_NOT_ALLOWED            405
#define SIP_406_NOT_ACCEPTABLE            406
#define SIP_PROXY_AUTHENTICATION_REQUIRED 407
#define SIP_REQUEST_TIME_OUT              408
#define SIP_GONE                          410
#define SIP_REQUEST_ENTITY_TOO_LARGE      413
#define SIP_REQUEST_URI_TOO_LARGE         414
#define SIP_UNSUPPORTED_MEDIA_TYPE        415
#define SIP_UNSUPPORTED_URI_SCHEME        416
#define SIP_BAD_EXTENSION                 420
#define SIP_EXTENSION_REQUIRED            421
#define SIP_INTERVAL_TOO_BRIEF            423
#define SIP_TEMPORARILY_UNAVAILABLE       480
#define SIP_CALL_TRANSACTION_DOES_NOT_EXIST 481
#define SIP_LOOP_DETECTED                 482
#define SIP_TOO_MANY_HOPS                 483
#define SIP_ADDRESS_INCOMPLETE            484
#define SIP_AMBIGUOUS                     485
#define SIP_BUSY_HERE                     486
#define SIP_REQUEST_TERMINATED            487
#define SIP_NOT_ACCEPTABLE_HERE           488
#define SIP_BAD_EVENT                     489
#define SIP_REQUEST_PENDING               491
#define SIP_UNDECIPHERABLE                493
#define SIP_INTERNAL_SERVER_ERROR         500
#define SIP_NOT_IMPLEMENTED               501
#define SIP_BAD_GATEWAY                   502
#define SIP_SERVICE_UNAVAILABLE           503
#define SIP_SERVER_TIME_OUT               504
#define SIP_VERSION_NOT_SUPPORTED         505
#define SIP_MESSAGE_TOO_LARGE             513
#define SIP_BUSY_EVRYWHERE                600
#define SIP_DECLINE                       603
#define SIP_DOES_NOT_EXIST_ANYWHERE       604
#define SIP_606_NOT_ACCEPTABLE                606

/** is the status code informational */
#define OSIP_IS_SIP_INFO(x)         (((x) >= 100)&&((x) < 200))
/** is the status code OK ?*/
#define OSIP_IS_SIP_SUCCESS(x)      (((x) >= 200)&&((x) < 300))
/** is the status code a redirect */
#define OSIP_IS_SIP_REDIRECT(x)     (((x) >= 300)&&((x) < 400))
/** is the status code a error (client or server) */
#define OSIP_IS_SIP_ERROR(x)        (((x) >= 400)&&((x) < 600))
/** is the status code a client error  */
#define OSIP_IS_SIP_CLIENT_ERROR(x) (((x) >= 400)&&((x) < 500))
/** is the status code a server error  */
#define OSIP_IS_SIP_SERVER_ERROR(x) (((x) >= 500)&&((x) < 600))


#endif /*  _CONST_H_ */
