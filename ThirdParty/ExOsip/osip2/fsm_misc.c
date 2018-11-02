/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001-2012 Aymeric MOIZARD amoizard@antisip.com
  
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

#include <osip2/internal.h>
#include <osip2/osip_fifo.h>
#include <osip2/osip.h>
#include "fsm.h"

static transition_t *fsm_findmethod (type_t type, state_t state, osip_statemachine_t * statemachine);

/* find the transition for state and type in statemachine */
/* return NULL; if transition is not found.               */
static transition_t *
fsm_findmethod (type_t type, state_t state, osip_statemachine_t * statemachine)
{
  transition_t *transition;

  for (transition = statemachine->transitions; transition != NULL; transition = transition->next) {
    if (transition->type == type && transition->state == state)
      return transition;
  }
  return NULL;
}

/* call the right execution method.          */
/*   return -1 when event must be discarded  */
int
fsm_callmethod (type_t type, state_t state, osip_statemachine_t * statemachine, void *sipevent, void *transaction)
{
  transition_t *transition;

  transition = fsm_findmethod (type, state, statemachine);
  if (transition == NULL) {
    /* No transition found for this event */
    return OSIP_UNDEFINED_ERROR;        /* error */
  }
  transition->method (transaction, sipevent);
  return OSIP_SUCCESS;          /* ok */
}
