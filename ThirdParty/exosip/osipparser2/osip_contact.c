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
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>

/* adds the contact header to message.              */
/* INPUT : const char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_message_set_contact (osip_message_t * sip, const char *hvalue)
{
  int i;
  osip_contact_t *contact;

  if (hvalue == NULL || hvalue[0] == '\0')
    return OSIP_SUCCESS;

  i = osip_contact_init (&contact);
  if (i != 0)
    return i;
  i = osip_contact_parse (contact, hvalue);
  if (i != 0) {
    osip_contact_free (contact);
    return i;
  }
  sip->message_property = 2;
  osip_list_add (&sip->contacts, contact, -1);
  return OSIP_SUCCESS;          /* ok */
}

/* parses a contact header.                                 */
/* INPUT : const char *string | pointer to a contact string.*/
/* OUTPUT: osip_contact_t *contact | structure to save results.  */
/* returns -1 on error. */
int
osip_contact_parse (osip_contact_t * contact, const char *hvalue)
{
  if (contact == NULL)
    return OSIP_BADPARAMETER;
  if (strncmp (hvalue, "*", 1) == 0) {
    contact->displayname = osip_strdup (hvalue);
    if (contact->displayname == NULL) {
      return OSIP_NOMEM;
    }
    return OSIP_SUCCESS;
  }
  return osip_from_parse ((osip_from_t *) contact, hvalue);
}

#ifndef MINISIZE
int
osip_contact_init (osip_contact_t ** contact)
{
  return osip_from_init ((osip_from_t **) contact);
}
#endif

#ifndef MINISIZE
/* returns the pos of contact header.                      */
/* INPUT : int pos | pos of contact header.                */
/* INPUT : osip_message_t *sip | sip message.                       */
/* OUTPUT: osip_contact_t *contact | structure to save results. */
/* returns -1 on error. */
int
osip_message_get_contact (const osip_message_t * sip, int pos, osip_contact_t ** dest)
{
  *dest = NULL;
  if (sip == NULL)
    return OSIP_BADPARAMETER;
  if (osip_list_size (&sip->contacts) <= pos)
    return OSIP_UNDEFINED_ERROR;        /* does not exist */
  *dest = (osip_contact_t *) osip_list_get (&sip->contacts, pos);
  return pos;
}
#endif

/* returns the contact header as a string.*/
/* INPUT : osip_contact_t *contact | contact.  */
/* returns null on error. */
int
osip_contact_to_str (const osip_contact_t * contact, char **dest)
{
  if (contact == NULL)
    return OSIP_BADPARAMETER;
  if (contact->displayname != NULL) {
    if (strncmp (contact->displayname, "*", 1) == 0) {
      *dest = osip_strdup ("*");
      if (*dest == NULL)
        return OSIP_NOMEM;
      return OSIP_SUCCESS;
    }
  }
  return osip_from_to_str ((osip_from_t *) contact, dest);
}

#ifndef MINISIZE
/* deallocates a osip_contact_t structure.  */
/* INPUT : osip_contact_t *| contact. */
void
osip_contact_free (osip_contact_t * contact)
{
  osip_from_free ((osip_from_t *) contact);
}

int
osip_contact_clone (const osip_contact_t * contact, osip_contact_t ** dest)
{
  return osip_from_clone ((osip_from_t *) contact, (osip_from_t **) dest);
}
#endif
