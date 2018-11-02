/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc2543-)
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


#ifndef _XIXT_H_
#define _XIXT_H_

#include <osipparser2/osip_const.h>
#include <osipparser2/osip_port.h>

#ifdef __cplusplus
extern "C" {
#endif


  void __osip_message_callback (int type, osip_transaction_t *, osip_message_t *);
  void __osip_kill_transaction_callback (int type, osip_transaction_t *);
  void __osip_transport_error_callback (int type, osip_transaction_t *, int error);

/**
 * Allocate an osip_ict_t element. (for outgoing INVITE transaction)
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param ict The element to allocate.
 * @param osip The global instance of oSIP.
 * @param invite The SIP request that initiate the transaction.
 */
  int __osip_ict_init (osip_ict_t ** ict, osip_t * osip, osip_message_t * invite);
/**
 * Free all resource in a osip_ict_t element.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param ict The element to free.
 */
  int __osip_ict_free (osip_ict_t * ict);


/**
 * Check if this transaction needs a TIMEOUT_A event 
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param ict The element to work on.
 * @param state The actual state of the transaction.
 * @param transactionid The transaction id.
 */
  osip_event_t *__osip_ict_need_timer_a_event (osip_ict_t * ict, state_t state, int transactionid);
/**
 * Check if this transaction needs a TIMEOUT_B event 
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param ict The element to work on.
 * @param state The actual state of the transaction.
 * @param transactionid The transaction id.
 */
  osip_event_t *__osip_ict_need_timer_b_event (osip_ict_t * ict, state_t state, int transactionid);
/**
 * Check if this transaction needs a TIMEOUT_D event 
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param ict The element to work on.
 * @param state The actual state of the transaction.
 * @param transactionid The transaction id.
 */
  osip_event_t *__osip_ict_need_timer_d_event (osip_ict_t * ict, state_t state, int transactionid);

/**
 * Allocate an osip_nict_t element. (for outgoing NON-INVITE transaction)
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param nict The element to allocate.
 * @param osip The global instance of oSIP.
 * @param request The SIP request that initiate the transaction.
 */
  int __osip_nict_init (osip_nict_t ** nict, osip_t * osip, osip_message_t * request);
/**
 * Free all resource in an osip_nict_t element.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param nict The element to free.
 */
  int __osip_nict_free (osip_nict_t * nict);


/**
 * Check if this transaction needs a TIMEOUT_E event 
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param nict The element to work on.
 * @param state The actual state of the transaction.
 * @param transactionid The transaction id.
 */
  osip_event_t *__osip_nict_need_timer_e_event (osip_nict_t * nict, state_t state, int transactionid);
/**
 * Check if this transaction needs a TIMEOUT_F event 
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param nict The element to work on.
 * @param state The actual state of the transaction.
 * @param transactionid The transaction id.
 */
  osip_event_t *__osip_nict_need_timer_f_event (osip_nict_t * nict, state_t state, int transactionid);
/**
 * Check if this transaction needs a TIMEOUT_K event 
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param nict The element to work on.
 * @param state The actual state of the transaction.
 * @param transactionid The transaction id.
 */
  osip_event_t *__osip_nict_need_timer_k_event (osip_nict_t * nict, state_t state, int transactionid);

/**
 * Allocate an osip_ist_t element. (for incoming INVITE transaction)
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param ist The element to allocate.
 * @param osip The global instance of oSIP.
 * @param invite The SIP invite that initiate the transaction.
 */
  int __osip_ist_init (osip_ist_t ** ist, osip_t * osip, osip_message_t * invite);
/**
 * Free all resource in a osip_ist_t element.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param ist The element to free.
 */
  int __osip_ist_free (osip_ist_t * ist);

/**
 * Check if this transaction needs a TIMEOUT_G event 
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param ist The element to work on.
 * @param state The actual state of the transaction.
 * @param transactionid The transaction id.
 */
  osip_event_t *__osip_ist_need_timer_g_event (osip_ist_t * ist, state_t state, int transactionid);
/**
 * Check if this transaction needs a TIMEOUT_H event 
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param ist The element to work on.
 * @param state The actual state of the transaction.
 * @param transactionid The transaction id.
 */
  osip_event_t *__osip_ist_need_timer_h_event (osip_ist_t * ist, state_t state, int transactionid);
/**
 * Check if this transaction needs a TIMEOUT_I event 
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param ist The element to work on.
 * @param state The actual state of the transaction.
 * @param transactionid The transaction id.
 */
  osip_event_t *__osip_ist_need_timer_i_event (osip_ist_t * ist, state_t state, int transactionid);

/**
 * Allocate an osip_nist_t element. (for incoming NON-INVITE transaction)
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param nist The element to allocate.
 * @param osip The global instance of oSIP.
 * @param request The SIP request that initiate the transaction.
 */
  int __osip_nist_init (osip_nist_t ** nist, osip_t * osip, osip_message_t * request);

/**
 * Free all resource in a osip_nist_t element.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param nist The element to free.
 */
  int __osip_nist_free (osip_nist_t * nist);


/**
 * Check if this transaction needs a TIMEOUT_J event 
 * @param nist The element to work on.
 * @param state The actual state of the transaction.
 * @param transactionid The transaction id.
 */
  osip_event_t *__osip_nist_need_timer_j_event (osip_nist_t * nist, state_t state, int transactionid);

/**
 * Check if the response match a server transaction.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param tr The transaction.
 * @param resp The SIP response received.
 */
  int
    __osip_transaction_matching_response_osip_to_xict_17_1_3 (osip_transaction_t * tr, osip_message_t * resp);

/**
 * Check if the request match a client transaction.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param tr The transaction.
 * @param request The SIP request received.
 */
  int
    __osip_transaction_matching_request_osip_to_xist_17_2_3 (osip_transaction_t * tr, osip_message_t * request);



/**
 * Lock access to the list of ict transactions.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param osip The element to work on.
 */
  int osip_ict_lock (osip_t * osip);
/**
 * Unlock access to the list of ict transactions.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param osip The element to work on.
 */
  int osip_ict_unlock (osip_t * osip);
/**
 * Lock access to the list of ist transactions.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param osip The element to work on.
 */
  int osip_ist_lock (osip_t * osip);
/**
 * Unlock access to the list of ist transactions.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param osip The element to work on.
 */
  int osip_ist_unlock (osip_t * osip);
/**
 * Lock access to the list of nict transactions.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param osip The element to work on.
 */
  int osip_nict_lock (osip_t * osip);
/**
 * Unlock access to the list of nict transactions.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param osip The element to work on.
 */
  int osip_nict_unlock (osip_t * osip);
/**
 * Lock access to the list of nist transactions.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param osip The element to work on.
 */
  int osip_nist_lock (osip_t * osip);
/**
 * Unlock access to the list of nist transactions.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param osip The element to work on.
 */
  int osip_nist_unlock (osip_t * osip);

/**
 * Add a ict transaction in the ict list of transaction.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param osip The element to work on.
 * @param ict The transaction to add.
 */
  int __osip_add_ict (osip_t * osip, osip_transaction_t * ict);
/**
 * Add a ist transaction in the ist list of transaction.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param osip The element to work on.
 * @param ist The transaction to add.
 */
  int __osip_add_ist (osip_t * osip, osip_transaction_t * ist);
/**
 * Add a nict transaction in the nict list of transaction.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param osip The element to work on.
 * @param nict The transaction to add.
 */
  int __osip_add_nict (osip_t * osip, osip_transaction_t * nict);
/**
 * Add a nist transaction in the nist list of transaction.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param osip The element to work on.
 * @param nist The transaction to add.
 */
  int __osip_add_nist (osip_t * osip, osip_transaction_t * nist);

/**
 * Remove a ict transaction from the ict list of transaction.
 * @param osip The element to work on.
 * @param ict The transaction to add.
 */
  int __osip_remove_ict_transaction (osip_t * osip, osip_transaction_t * ict);
/**
 * Remove a ist transaction from the ist list of transaction.
 * @param osip The element to work on.
 * @param ist The transaction to add.
 */
  int __osip_remove_ist_transaction (osip_t * osip, osip_transaction_t * ist);
/**
 * Remove a nict transaction from the nict list of transaction.
 * @param osip The element to work on.
 * @param nict The transaction to add.
 */
  int __osip_remove_nict_transaction (osip_t * osip, osip_transaction_t * nict);
/**
 * Remove a nist transaction from the nist list of transaction.
 * @param osip The element to work on.
 * @param nist The transaction to add.
 */
  int __osip_remove_nist_transaction (osip_t * osip, osip_transaction_t * nist);

/**
 * Allocate a sipevent.
 * NOTE: THIS IS AN INTERNAL METHOD ONLY
 * @param type The type of the event.
 * @param transactionid The transaction id for this event.
 */
  osip_event_t *__osip_event_new (type_t type, int transactionid);


/**
 * Allocate a sipevent (we know this message is an OUTGOING SIP message).
 * @param sip The SIP message we want to send.
 */
  osip_event_t *osip_new_outgoing_sipmessage (osip_message_t * sip);

#ifdef __cplusplus
}
#endif
#endif
