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

#ifdef WIN32
#ifndef UNICODE
#define UNICODE
#endif
#endif

#include "eXosip2.h"
#include "eXtransport.h"

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if defined(_MSC_VER) && defined(WIN32) && !defined(_WIN32_WCE)
#define HAVE_MSTCPIP_H
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#undef HAVE_MSTCPIP_H
#endif
#endif

#if defined(WIN32)
#define HAVE_WINCRYPT_H
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#undef HAVE_WINCRYPT_H
#endif
#endif

#ifdef HAVE_MSTCPIP_H
#include <Mstcpip.h>
#else
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#endif

#ifdef HAVE_WINCRYPT_H
#include <wincrypt.h>
#endif

#if !defined(_WIN32_WCE)
#include <errno.h>
#endif

#if defined(HAVE_NETINET_TCP_H)
#include <netinet/tcp.h>
#endif

#if defined(_WIN32_WCE) || defined(WIN32)
#define strerror(X) "-1"
#define ex_errno WSAGetLastError()
#define is_wouldblock_error(r) ((r)==WSAEINTR||(r)==WSAEWOULDBLOCK)
#define is_connreset_error(r) ((r)==WSAECONNRESET || (r)==WSAECONNABORTED || (r)==WSAETIMEDOUT || (r)==WSAENETRESET || (r)==WSAENOTCONN)
#else
#define ex_errno errno
#endif
#ifndef is_wouldblock_error
#define is_wouldblock_error(r) ((r)==EINTR||(r)==EWOULDBLOCK||(r)==EAGAIN)
#define is_connreset_error(r) ((r)==ECONNRESET || (r)==ECONNABORTED || (r)==ETIMEDOUT || (r)==ENETRESET || (r)==ENOTCONN)
#endif

#ifdef HAVE_OPENSSL_SSL_H

#define ex_verify_depth 10
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#if !(OPENSSL_VERSION_NUMBER < 0x10002000L)
#include <openssl/x509v3.h>
#endif
#include <openssl/rand.h>
#include <openssl/tls1.h>

#define SSLDEBUG 1
/*#define PATH "D:/conf/"

#define PASSWORD "23-+Wert"
#define CLIENT_KEYFILE PATH"ckey.pem"
#define CLIENT_CERTFILE PATH"c.pem"
#define SERVER_KEYFILE PATH"skey.pem"
#define SERVER_CERTFILE PATH"s.pem"
#define CA_LIST PATH"cacert.pem"
#define RANDOM  PATH"random.pem"
#define DHFILE PATH"dh1024.pem"*/

#ifdef __APPLE_CC__
#include "TargetConditionals.h"
#endif

#if defined(__APPLE__) && (TARGET_OS_IPHONE==0)
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <Security/Security.h>
#endif

#if TARGET_OS_IPHONE
#include <CoreFoundation/CFStream.h>
#include <CFNetwork/CFSocketStream.h>
#define MULTITASKING_ENABLED
#endif

SSL_CTX *initialize_client_ctx (struct eXosip_t *excontext, const char *certif_client_local_cn_name, eXosip_tls_ctx_t * client_ctx, int transport, const char *sni_servernameindication);

SSL_CTX *initialize_server_ctx (struct eXosip_t *excontext, const char *certif_local_cn_name, eXosip_tls_ctx_t * srv_ctx, int transport);

int verify_cb (int preverify_ok, X509_STORE_CTX * store);


/* persistent connection */
struct _tls_stream {
  int socket;
  struct sockaddr ai_addr;
  socklen_t ai_addrlen;
  char sni_servernameindication[256];
  char remote_ip[65];
  int remote_port;
  char *previous_content;
  int previous_content_len;
  SSL *ssl_conn;
  SSL_CTX *ssl_ctx;
  int ssl_state;
  char *buf;                    /* recv buffer */
  size_t bufsize;               /* allocated size of buf */
  size_t buflen;                /* current length of buf */
  char *sendbuf;                /* send buffer */
  size_t sendbufsize;
  size_t sendbuflen;
#ifdef MULTITASKING_ENABLED
  CFReadStreamRef readStream;
  CFWriteStreamRef writeStream;
#endif
  char natted_ip[65];
  int natted_port;
  int ephemeral_port;
  int invalid;
  int is_server;
  time_t tcp_max_timeout;
  time_t tcp_inprogress_max_timeout;
  char reg_call_id[64];
};

#ifndef SOCKET_TIMEOUT
/* when stream has sequence error: */
/* having SOCKET_TIMEOUT > 0 helps the system to recover */
#define SOCKET_TIMEOUT 0
#endif

#ifndef EXOSIP_MAX_SOCKETS
#define EXOSIP_MAX_SOCKETS 200
#endif

struct eXtltls {

  int tls_socket;
  struct sockaddr_storage ai_addr;
  int ai_addr_len;

  SSL_CTX *server_ctx;
  SSL_CTX *client_ctx;

  struct _tls_stream socket_tab[EXOSIP_MAX_SOCKETS];
};

static int
tls_tl_init (struct eXosip_t *excontext)
{
  struct eXtltls *reserved = (struct eXtltls *) osip_malloc (sizeof (struct eXtltls));

  if (reserved == NULL)
    return OSIP_NOMEM;
  reserved->tls_socket = 0;
  reserved->server_ctx = NULL;
  reserved->client_ctx = NULL;
  memset (&reserved->ai_addr, 0, sizeof (struct sockaddr_storage));
  reserved->ai_addr_len=0;
  memset (&reserved->socket_tab, 0, sizeof (struct _tls_stream) * EXOSIP_MAX_SOCKETS);

  excontext->eXtltls_reserved = reserved;
  return OSIP_SUCCESS;
}

static void
_tls_tl_close_sockinfo (struct _tls_stream *sockinfo)
{
  if (sockinfo->socket > 0) {
    if (sockinfo->ssl_conn != NULL) {
      SSL_shutdown (sockinfo->ssl_conn);
      SSL_shutdown (sockinfo->ssl_conn);
      SSL_free (sockinfo->ssl_conn);
    }
    if (sockinfo->ssl_ctx != NULL)
      SSL_CTX_free (sockinfo->ssl_ctx);
    _eXosip_closesocket (sockinfo->socket);
  }
  if (sockinfo->buf != NULL)
    osip_free (sockinfo->buf);
  if (sockinfo->sendbuf != NULL)
    osip_free (sockinfo->sendbuf);
#ifdef MULTITASKING_ENABLED
  if (sockinfo->readStream != NULL) {
    CFReadStreamClose (sockinfo->readStream);
    CFRelease (sockinfo->readStream);
  }
  if (sockinfo->writeStream != NULL) {
    CFWriteStreamClose (sockinfo->writeStream);
    CFRelease (sockinfo->writeStream);
  }
#endif
  memset (sockinfo, 0, sizeof (*sockinfo));
}

static int
tls_tl_free (struct eXosip_t *excontext)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;
  int pos;

  if (reserved == NULL)
    return OSIP_SUCCESS;

  if (reserved->server_ctx != NULL)
    SSL_CTX_free (reserved->server_ctx);
  reserved->server_ctx = NULL;

  if (reserved->client_ctx != NULL)
    SSL_CTX_free (reserved->client_ctx);
  reserved->client_ctx = NULL;

  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    _tls_tl_close_sockinfo (&reserved->socket_tab[pos]);
  }

  ERR_remove_state (0);

  memset (&reserved->socket_tab, 0, sizeof (struct _tls_stream) * EXOSIP_MAX_SOCKETS);

  memset (&reserved->ai_addr, 0, sizeof (struct sockaddr_storage));
  reserved->ai_addr_len=0;
  if (reserved->tls_socket > 0)
    _eXosip_closesocket (reserved->tls_socket);
  reserved->tls_socket = 0;

  osip_free (reserved);
  excontext->eXtltls_reserved = NULL;
  return OSIP_SUCCESS;
}

static int
tls_tl_reset (struct eXosip_t *excontext)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;
  int pos;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    if (reserved->socket_tab[pos].socket > 0)
      reserved->socket_tab[pos].invalid = 1;
  }
  return OSIP_SUCCESS;
}

static void
tls_dump_cert_info (const char *s, X509 * cert)
{
  char *subj;
  char *issuer;

  subj = X509_NAME_oneline (X509_get_subject_name (cert), 0, 0);
  issuer = X509_NAME_oneline (X509_get_issuer_name (cert), 0, 0);

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "%s subject:%s\n", s ? s : "", subj));
  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "%s issuer: %s\n", s ? s : "", issuer));
  OPENSSL_free (subj);
  OPENSSL_free (issuer);
}

static int
_tls_add_certificates (SSL_CTX * ctx)
{
  int count = 0;

#ifdef HAVE_WINCRYPT_H
  PCCERT_CONTEXT pCertCtx;
  X509 *cert = NULL;
  HCERTSTORE hStore = CertOpenSystemStore (0, L"CA");

  for (pCertCtx = CertEnumCertificatesInStore (hStore, NULL); pCertCtx != NULL; pCertCtx = CertEnumCertificatesInStore (hStore, pCertCtx)) {
    cert = d2i_X509 (NULL, (const unsigned char **) &pCertCtx->pbCertEncoded, pCertCtx->cbCertEncoded);
    if (cert == NULL) {
      continue;
    }
    /*tls_dump_cert_info("CA", cert); */

    if (!X509_STORE_add_cert (ctx->cert_store, cert)) {
      X509_free (cert);
      continue;
    }
    count++;
    X509_free (cert);
  }

  CertCloseStore (hStore, 0);

  hStore = CertOpenSystemStore (0, L"ROOT");

  for (pCertCtx = CertEnumCertificatesInStore (hStore, NULL); pCertCtx != NULL; pCertCtx = CertEnumCertificatesInStore (hStore, pCertCtx)) {
    cert = d2i_X509 (NULL, (const unsigned char **) &pCertCtx->pbCertEncoded, pCertCtx->cbCertEncoded);
    if (cert == NULL) {
      continue;
    }
    /*tls_dump_cert_info("ROOT", cert); */

    if (!X509_STORE_add_cert (ctx->cert_store, cert)) {
      X509_free (cert);
      continue;
    }
    count++;
    X509_free (cert);
  }

  CertCloseStore (hStore, 0);

  hStore = CertOpenSystemStore (0, L"MY");

  for (pCertCtx = CertEnumCertificatesInStore (hStore, NULL); pCertCtx != NULL; pCertCtx = CertEnumCertificatesInStore (hStore, pCertCtx)) {
    cert = d2i_X509 (NULL, (const unsigned char **) &pCertCtx->pbCertEncoded, pCertCtx->cbCertEncoded);
    if (cert == NULL) {
      continue;
    }
    /*tls_dump_cert_info("MY", cert); */

    if (!X509_STORE_add_cert (ctx->cert_store, cert)) {
      X509_free (cert);
      continue;
    }
    count++;
    X509_free (cert);
  }

  CertCloseStore (hStore, 0);

  hStore = CertOpenSystemStore (0, L"Trustedpublisher");

  for (pCertCtx = CertEnumCertificatesInStore (hStore, NULL); pCertCtx != NULL; pCertCtx = CertEnumCertificatesInStore (hStore, pCertCtx)) {
    cert = d2i_X509 (NULL, (const unsigned char **) &pCertCtx->pbCertEncoded, pCertCtx->cbCertEncoded);
    if (cert == NULL) {
      continue;
    }
    /*tls_dump_cert_info("Trustedpublisher", cert); */

    if (!X509_STORE_add_cert (ctx->cert_store, cert)) {
      X509_free (cert);
      continue;
    }
    count++;
    X509_free (cert);
  }

  CertCloseStore (hStore, 0);
#elif defined(__APPLE__) && (TARGET_OS_IPHONE==0)
  SecKeychainSearchRef pSecKeychainSearch = NULL;
  SecKeychainRef pSecKeychain;
  OSStatus status = noErr;
  X509 *cert = NULL;
  SInt32 osx_version = 0;

  if (Gestalt (gestaltSystemVersion, &osx_version) != noErr) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "macosx certificate store: can't get osx version"));
    return 0;
  }
  if (osx_version >= 0x1050) {
    /* Leopard store location */
    status = SecKeychainOpen ("/System/Library/Keychains/SystemRootCertificates.keychain", &pSecKeychain);
  }
  else {
    /* Tiger and below store location */
    status = SecKeychainOpen ("/System/Library/Keychains/X509Anchors", &pSecKeychain);
  }
  if (status != noErr) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "macosx certificate store: can't get osx version"));
    return 0;
  }

  status = SecKeychainSearchCreateFromAttributes (pSecKeychain, kSecCertificateItemClass, NULL, &pSecKeychainSearch);
  for (;;) {
    SecKeychainItemRef pSecKeychainItem = nil;

    status = SecKeychainSearchCopyNext (pSecKeychainSearch, &pSecKeychainItem);
    if (status == errSecItemNotFound) {
      break;
    }

    if (status == noErr) {
      void *_pCertData;
      UInt32 _pCertLength;

      status = SecKeychainItemCopyAttributesAndData (pSecKeychainItem, NULL, NULL, NULL, &_pCertLength, &_pCertData);

      if (status == noErr && _pCertData != NULL) {
        unsigned char *ptr;

        ptr = _pCertData;       /*required because d2i_X509 is modifying pointer */
        cert = d2i_X509 (NULL, (const unsigned char **) &ptr, _pCertLength);
        if (cert == NULL) {
          continue;
        }
        /*tls_dump_cert_info("ROOT", cert); */

        if (!X509_STORE_add_cert (ctx->cert_store, cert)) {
          X509_free (cert);
          continue;
        }
        count++;
        X509_free (cert);

        status = SecKeychainItemFreeAttributesAndData (NULL, _pCertData);
      }
    }

    if (pSecKeychainItem != NULL)
      CFRelease (pSecKeychainItem);

    if (status != noErr) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "macosx certificate store: can't add certificate (%i)", status));
    }
  }

  CFRelease (pSecKeychainSearch);
  CFRelease (pSecKeychain);

#endif
  return count;
}

#ifdef HAVE_WINCRYPT_H

struct rsa_ctx {
  const CERT_CONTEXT *cert;
  HCRYPTPROV crypt_prov;
  DWORD key_spec;
  BOOL free_crypt_prov;
  HCRYPTKEY hCryptKey;
};

static int
rsa_pub_enc (int flen, const unsigned char *from, unsigned char *to, RSA * rsa, int padding)
{
  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "rsa_pub_enc - not implemented"));
  return 0;
}

static int
rsa_pub_dec (int flen, const unsigned char *from, unsigned char *to, RSA * rsa, int padding)
{
  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "rsa_pub_dec - not implemented"));
  return 0;
}

static int
rsa_priv_enc (int flen, const unsigned char *from, unsigned char *to, RSA * rsa, int padding)
{
  struct rsa_ctx *priv = (struct rsa_ctx *) rsa->meth->app_data;
  HCRYPTHASH hash;
  DWORD hash_size, len, i;
  unsigned char *buf = NULL;
  int ret = 0;

  if (priv == NULL) {
    RSAerr (RSA_F_RSA_EAY_PRIVATE_ENCRYPT, ERR_R_PASSED_NULL_PARAMETER);
    return 0;
  }

  if (padding != RSA_PKCS1_PADDING) {
    RSAerr (RSA_F_RSA_EAY_PRIVATE_ENCRYPT, RSA_R_UNKNOWN_PADDING_TYPE);
    return 0;
  }

  if (flen != 16 /* MD5 */  + 20 /* SHA-1 */ ) {

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "rsa_priv_enc - only MD5-SHA1 hash supported"));
    RSAerr (RSA_F_RSA_EAY_PRIVATE_ENCRYPT, RSA_R_INVALID_MESSAGE_LENGTH);
    return 0;
  }

  if (!CryptCreateHash (priv->crypt_prov, CALG_SSL3_SHAMD5, 0, 0, &hash)) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "CryptCreateHash failed"));
    return 0;
  }

  len = sizeof (hash_size);
  if (!CryptGetHashParam (hash, HP_HASHSIZE, (BYTE *) & hash_size, &len, 0)) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "CryptGetHashParam failed"));
    goto err;
  }

  if (hash_size != flen) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "CryptoAPI: Invalid hash size (%u != %d)", (unsigned) hash_size, flen));
    RSAerr (RSA_F_RSA_EAY_PRIVATE_ENCRYPT, RSA_R_INVALID_MESSAGE_LENGTH);
    goto err;
  }
  if (!CryptSetHashParam (hash, HP_HASHVAL, (BYTE *) from, 0)) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "CryptSetHashParam failed"));
    goto err;
  }

  len = RSA_size (rsa);
  buf = osip_malloc (len);
  if (buf == NULL) {
    RSAerr (RSA_F_RSA_EAY_PRIVATE_ENCRYPT, ERR_R_MALLOC_FAILURE);
    goto err;
  }

  if (!CryptSignHash (hash, priv->key_spec, NULL, 0, buf, &len)) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "CryptSignHash failed"));
    goto err;
  }

  for (i = 0; i < len; i++)
    to[i] = buf[len - i - 1];
  ret = len;

err:
  osip_free (buf);
  CryptDestroyHash (hash);

  return ret;
}

static int
rsa_priv_dec (int flen, const unsigned char *from, unsigned char *to, RSA * rsa, int padding)
{
  struct rsa_ctx *priv = (struct rsa_ctx *) rsa->meth->app_data;
  BOOL ret;
  DWORD cbData = flen;
  int i;
  unsigned char *buf = NULL;

  if (padding != RSA_PKCS1_PADDING)
    return -1;
  if (priv->hCryptKey == 0)
    return -1;

  buf = osip_malloc (cbData * 4);
  for (i = 0; i < flen; i++)
    buf[flen - i - 1] = from[i];

  ret = CryptDecrypt (priv->hCryptKey, 0, TRUE, 0, buf, &flen);
  if (!ret)
    return -1;

  memcpy (to, buf, flen);

  osip_free (buf);
  return flen;
}

static void
rsa_free_data (struct rsa_ctx *priv)
{
  if (priv == NULL)
    return;
  if (priv->hCryptKey)
    CryptDestroyKey (priv->hCryptKey);
  if (priv->crypt_prov && priv->free_crypt_prov)
    CryptReleaseContext (priv->crypt_prov, 0);
  if (priv->cert)
    CertFreeCertificateContext (priv->cert);
  osip_free (priv);
}

static int
rsa_finish (RSA * rsa)
{
  rsa_free_data ((struct rsa_ctx *) rsa->meth->app_data);
  osip_free ((void *) rsa->meth);
  rsa->meth = NULL;
  return 1;
}

#endif

static X509 *
_tls_set_certificate (SSL_CTX * ctx, const char *cn)
{
#ifdef HAVE_WINCRYPT_H
  PCCERT_CONTEXT pCertCtx;
  X509 *cert = NULL;
  HCERTSTORE hStore = CertOpenSystemStore (0, L"CA");

  for (pCertCtx = CertEnumCertificatesInStore (hStore, NULL); pCertCtx != NULL; pCertCtx = CertEnumCertificatesInStore (hStore, pCertCtx)) {
    char peer_CN[65];

    cert = d2i_X509 (NULL, (const unsigned char **) &pCertCtx->pbCertEncoded, pCertCtx->cbCertEncoded);
    if (cert == NULL) {
      continue;
    }

    memset (peer_CN, 0, sizeof (peer_CN));
    X509_NAME_get_text_by_NID (X509_get_subject_name (cert), NID_commonName, peer_CN, sizeof (peer_CN));
    if (osip_strcasecmp (cn, peer_CN) == 0) {
      break;
    }
    X509_free (cert);
    cert = NULL;
  }

  CertCloseStore (hStore, 0);

  if (cert == NULL) {
    hStore = CertOpenSystemStore (0, L"ROOT");
    for (pCertCtx = CertEnumCertificatesInStore (hStore, NULL); pCertCtx != NULL; pCertCtx = CertEnumCertificatesInStore (hStore, pCertCtx)) {
      char peer_CN[65];

      cert = d2i_X509 (NULL, (const unsigned char **) &pCertCtx->pbCertEncoded, pCertCtx->cbCertEncoded);
      if (cert == NULL) {
        continue;
      }

      memset (peer_CN, 0, sizeof (peer_CN));
      X509_NAME_get_text_by_NID (X509_get_subject_name (cert), NID_commonName, peer_CN, sizeof (peer_CN));
      if (osip_strcasecmp (cn, peer_CN) == 0) {
        break;
      }
      X509_free (cert);
      cert = NULL;
    }

    CertCloseStore (hStore, 0);
  }

  if (cert == NULL) {
    hStore = CertOpenSystemStore (0, L"MY");

    for (pCertCtx = CertEnumCertificatesInStore (hStore, NULL); pCertCtx != NULL; pCertCtx = CertEnumCertificatesInStore (hStore, pCertCtx)) {
      char peer_CN[65];

      cert = d2i_X509 (NULL, (const unsigned char **) &pCertCtx->pbCertEncoded, pCertCtx->cbCertEncoded);
      if (cert == NULL) {
        continue;
      }

      memset (peer_CN, 0, sizeof (peer_CN));
      X509_NAME_get_text_by_NID (X509_get_subject_name (cert), NID_commonName, peer_CN, sizeof (peer_CN));
      if (osip_strcasecmp (cn, peer_CN) == 0) {
        break;
      }
      X509_free (cert);
      cert = NULL;
    }

    CertCloseStore (hStore, 0);
  }

  if (cert == NULL) {
    hStore = CertOpenSystemStore (0, L"Trustedpublisher");

    for (pCertCtx = CertEnumCertificatesInStore (hStore, NULL); pCertCtx != NULL; pCertCtx = CertEnumCertificatesInStore (hStore, pCertCtx)) {
      char peer_CN[65];

      cert = d2i_X509 (NULL, (const unsigned char **) &pCertCtx->pbCertEncoded, pCertCtx->cbCertEncoded);
      if (cert == NULL) {
        continue;
      }

      memset (peer_CN, 0, sizeof (peer_CN));
      X509_NAME_get_text_by_NID (X509_get_subject_name (cert), NID_commonName, peer_CN, sizeof (peer_CN));
      if (osip_strcasecmp (cn, peer_CN) == 0) {
        break;
      }
      X509_free (cert);
      cert = NULL;
    }

    CertCloseStore (hStore, 0);
  }

  if (cert == NULL)
    return NULL;

  {
    RSA *rsa = NULL, *pub_rsa;
    struct rsa_ctx *priv;
    RSA_METHOD *rsa_meth;

    priv = osip_malloc (sizeof (*priv));
    rsa_meth = osip_malloc (sizeof (*rsa_meth));
    if (priv == NULL || rsa_meth == NULL) {
      CertFreeCertificateContext (pCertCtx);
      osip_free (priv);
      osip_free (rsa_meth);
      X509_free (cert);
      return NULL;
    }
    memset (priv, 0, sizeof (*priv));
    memset (rsa_meth, 0, sizeof (*rsa_meth));

    priv->cert = pCertCtx;

    if (CryptAcquireCertificatePrivateKey (pCertCtx, CRYPT_ACQUIRE_COMPARE_KEY_FLAG, NULL, &priv->crypt_prov, &priv->key_spec, &priv->free_crypt_prov) == 0) {
      CertFreeCertificateContext (priv->cert);
      osip_free (priv);
      osip_free (rsa_meth);
      X509_free (cert);
      return NULL;
    }

    if (!CryptGetUserKey (priv->crypt_prov, priv->key_spec, &priv->hCryptKey)) {
      CertFreeCertificateContext (priv->cert);
      if (priv->crypt_prov && priv->free_crypt_prov)
        CryptReleaseContext (priv->crypt_prov, 0);
      osip_free (priv);
      osip_free (rsa_meth);
      X509_free (cert);
      return NULL;
    }

    rsa_meth->name = "Microsoft CryptoAPI RSA Method";
    rsa_meth->rsa_pub_enc = rsa_pub_enc;
    rsa_meth->rsa_pub_dec = rsa_pub_dec;
    rsa_meth->rsa_priv_enc = rsa_priv_enc;
    rsa_meth->rsa_priv_dec = rsa_priv_dec;
    rsa_meth->finish = rsa_finish;
    rsa_meth->flags = RSA_METHOD_FLAG_NO_CHECK;
    rsa_meth->app_data = (char *) priv;

    rsa = RSA_new ();
    if (rsa == NULL) {
      CertFreeCertificateContext (priv->cert);
      if (priv->crypt_prov && priv->free_crypt_prov)
        CryptReleaseContext (priv->crypt_prov, 0);
      osip_free (priv);
      osip_free (rsa_meth);
      X509_free (cert);
      RSA_free (rsa);
      return NULL;
    }

    if (!SSL_CTX_use_certificate (ctx, cert)) {
      CertFreeCertificateContext (priv->cert);
      if (priv->crypt_prov && priv->free_crypt_prov)
        CryptReleaseContext (priv->crypt_prov, 0);
      osip_free (priv);
      osip_free (rsa_meth);
      X509_free (cert);
      return NULL;
    }

    pub_rsa = cert->cert_info->key->pkey->pkey.rsa;

    rsa->n = BN_dup (pub_rsa->n);
    rsa->e = BN_dup (pub_rsa->e);
    if (!RSA_set_method (rsa, rsa_meth)) {
      CertFreeCertificateContext (priv->cert);
      if (priv->crypt_prov && priv->free_crypt_prov)
        CryptReleaseContext (priv->crypt_prov, 0);
      osip_free (priv);
      osip_free (rsa_meth);
      RSA_free (rsa);
      X509_free (cert);
      SSL_CTX_free (ctx);
      return NULL;
    }

    if (!SSL_CTX_use_RSAPrivateKey (ctx, rsa)) {
      RSA_free (rsa);
      X509_free (cert);
      SSL_CTX_free (ctx);
      return NULL;
    }
    RSA_free (rsa);

    return cert;
  }

#endif
  return NULL;
}

int
verify_cb (int preverify_ok, X509_STORE_CTX * store)
{
  char buf[256];
  X509 *err_cert;
  int err, depth;

  err_cert = X509_STORE_CTX_get_current_cert (store);
  err = X509_STORE_CTX_get_error (store);
  depth = X509_STORE_CTX_get_error_depth (store);

  X509_NAME_oneline (X509_get_subject_name (err_cert), buf, 256);

  if (depth > ex_verify_depth /* depth -1 */ ) {
    preverify_ok = 0;
    err = X509_V_ERR_CERT_CHAIN_TOO_LONG;
    X509_STORE_CTX_set_error (store, err);
  }
  if (!preverify_ok) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "verify error:num=%d:%s:depth=%d:%s\n", err, X509_verify_cert_error_string (err), depth, buf));
  }
  else {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "depth=%d:%s\n", depth, buf));
  }
  /*
   * At this point, err contains the last verification error. We can use
   * it for something special
   */
  if (!preverify_ok && (err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT)) {
    X509_NAME_oneline (X509_get_issuer_name (store->current_cert), buf, 256);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "issuer= %s\n", buf));
  }

  return preverify_ok;

#if 0
  if (!preverify_ok && (err == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN)) {
    X509_NAME_oneline (X509_get_issuer_name (store->current_cert), buf, 256);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "issuer= %s\n", buf));
    preverify_ok = 1;
    X509_STORE_CTX_set_error (store, X509_V_OK);
  }

  if (!preverify_ok && (err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT)) {
    X509_NAME_oneline (X509_get_issuer_name (store->current_cert), buf, 256);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "issuer= %s\n", buf));
    preverify_ok = 1;
    X509_STORE_CTX_set_error (store, X509_V_OK);
  }

  if (!preverify_ok && (err == X509_V_ERR_CERT_HAS_EXPIRED)) {
    X509_NAME_oneline (X509_get_issuer_name (store->current_cert), buf, 256);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "issuer= %s\n", buf));
    preverify_ok = 1;
    X509_STORE_CTX_set_error (store, X509_V_OK);
  }

  if (!preverify_ok && (err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY)) {
    X509_NAME_oneline (X509_get_issuer_name (store->current_cert), buf, 256);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "issuer= %s\n", buf));
    preverify_ok = 1;
    X509_STORE_CTX_set_error (store, X509_V_OK);
  }

  if (!preverify_ok && (err == X509_V_ERR_CERT_UNTRUSTED)) {
    X509_NAME_oneline (X509_get_issuer_name (store->current_cert), buf, 256);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "issuer= %s\n", buf));
    preverify_ok = 1;
    X509_STORE_CTX_set_error (store, X509_V_OK);
  }

  if (!preverify_ok && (err == X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE)) {
    X509_NAME_oneline (X509_get_issuer_name (store->current_cert), buf, 256);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "issuer= %s\n", buf));
    preverify_ok = 1;
    X509_STORE_CTX_set_error (store, X509_V_OK);
  }

  preverify_ok = 1;             /* configured to accept anyway! */
  return preverify_ok;
#endif
}

static int
password_cb (char *buf, int num, int rwflag, void *userdata)
{
  char *passwd = (char *) userdata;

  if (passwd == NULL || passwd[0] == '\0') {
    return OSIP_SUCCESS;
  }
  strncpy (buf, passwd, num);
  buf[num - 1] = '\0';
  return (int) strlen (buf);
}

static void
load_dh_params (SSL_CTX * ctx, char *file)
{
  DH *ret = 0;
  BIO *bio;

  if ((bio = BIO_new_file (file, "r")) == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't open DH file!\n"));
  }
  else {
    ret = PEM_read_bio_DHparams (bio, NULL, NULL, NULL);
    BIO_free (bio);
    if (SSL_CTX_set_tmp_dh (ctx, ret) < 0)
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't set DH param!\n"));
  }
}

static void
build_dh_params (SSL_CTX * ctx)
{
  int codes = 0;
  DH *dh = DH_new ();

  if (!dh) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: DH_new failed!\n"));
    return;
  }
  if (!DH_generate_parameters_ex (dh, 2, DH_GENERATOR_2, 0)) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: DH_generate_parameters_ex failed!\n"));
    DH_free (dh);
    return;
  }

  if (!DH_check (dh, &codes)) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: DH_check failed!\n"));
    DH_free (dh);
    return;
  }
  if (!DH_generate_key (dh)) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: DH_generate_key failed!\n"));
    DH_free (dh);
    return;
  }
  SSL_CTX_set_tmp_dh (ctx, dh);
  DH_free (dh);

  return;
}

#if (OPENSSL_VERSION_NUMBER < 0x00908000l || defined(ANDROID))
RSA *
RSA_generate_key (int bits, unsigned long e_value, void (*callback) (int, int, void *), void *cb_arg)
{
  BN_GENCB cb;
  int i;
  RSA *rsa = RSA_new ();
  BIGNUM *e = BN_new ();

  if (!rsa || !e)
    goto err;

  /* The problem is when building with 8, 16, or 32 BN_ULONG,                                                                                                  
   * unsigned long can be larger */
  for (i = 0; i < (int) sizeof (unsigned long) * 8; i++) {
    if (e_value & (1UL << i))
      if (BN_set_bit (e, i) == 0)
        goto err;
  }

  BN_GENCB_set_old (&cb, callback, cb_arg);

  if (RSA_generate_key_ex (rsa, bits, e, &cb)) {
    BN_free (e);
    return rsa;
  }
err:
  if (e)
    BN_free (e);
  if (rsa)
    RSA_free (rsa);
  return 0;
}
#endif

static void
generate_eph_rsa_key (SSL_CTX * ctx)
{
  RSA *rsa;

  rsa = RSA_generate_key (512, RSA_F4, NULL, NULL);

  if (rsa != NULL) {
    if (!SSL_CTX_set_tmp_rsa (ctx, rsa))
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't set RSA key!\n"));

    RSA_free (rsa);
  }
}

eXosip_tls_ctx_error
eXosip_set_tls_ctx (struct eXosip_t *excontext, eXosip_tls_ctx_t * ctx)
{
  eXosip_tls_credentials_t *ownClient = &excontext->eXosip_tls_ctx_params.client;
  eXosip_tls_credentials_t *ownServer = &excontext->eXosip_tls_ctx_params.server;

  eXosip_tls_credentials_t *client = &ctx->client;
  eXosip_tls_credentials_t *server = &ctx->server;

  /* check if public AND private keys are valid */
  if (client->cert[0] == '\0' && client->priv_key[0] != '\0') {
    /* no, one is missing */
    return TLS_ERR_MISSING_AUTH_PART;
  }
  /* check if a password is set, when a private key is present */
  if (client->priv_key[0] != '\0' && client->priv_key_pw[0] == '\0') {
    return TLS_ERR_NO_PW;
  }
  /* check if public AND private keys are valid */
  if (server->cert[0] == '\0' && server->priv_key[0] != '\0') {
    /* no, one is missing */
    return TLS_ERR_MISSING_AUTH_PART;
  }
  /* check if a password is set, when a private key is present */
  if (server->priv_key[0] != '\0' && server->priv_key_pw[0] == '\0') {
    return TLS_ERR_NO_PW;
  }
  /* check if the file for diffie hellman is present */
  /*if(ctx->dh_param[0] == '\0') {
     return TLS_ERR_NO_DH_PARAM;
     } */

  /* check if a file with random data is present --> will be verified when random file is needed (see tls_tl_open) */
  /*if(ctx->random_file[0] == '\0') {
     return TLS_ERR_NO_RAND;
     } */

  /* check if a file with the list of possible rootCAs is available */
  /*if(ctx->root_ca_cert[0] == '\0') {
     return TLS_ERR_NO_ROOT_CA;
     } */

  /* clean up configuration */
  memset (&excontext->eXosip_tls_ctx_params, 0, sizeof (eXosip_tls_ctx_t));

  /* check if client has own certificate */
  if (client->cert[0] != '\0') {
    snprintf (ownClient->cert, sizeof (ownClient->cert), "%s", client->cert);
    snprintf (ownClient->priv_key, sizeof (ownClient->priv_key), "%s", client->priv_key);
    snprintf (ownClient->priv_key_pw, sizeof (ownClient->priv_key_pw), "%s", client->priv_key_pw);
  }
  /* check if server has own certificate */
  if (server->cert[0] != '\0') {
    snprintf (ownServer->cert, sizeof (ownServer->cert), "%s", server->cert);
    snprintf (ownServer->priv_key, sizeof (ownServer->priv_key), "%s", server->priv_key);
    snprintf (ownServer->priv_key_pw, sizeof (ownServer->priv_key_pw), "%s", server->priv_key_pw);
  }

  snprintf (excontext->eXosip_tls_ctx_params.dh_param, sizeof (ctx->dh_param), "%s", ctx->dh_param);
  snprintf (excontext->eXosip_tls_ctx_params.random_file, sizeof (ctx->random_file), "%s", ctx->random_file);
  snprintf (excontext->eXosip_tls_ctx_params.root_ca_cert, sizeof (ctx->root_ca_cert), "%s", ctx->root_ca_cert);

  return TLS_OK;
}

eXosip_tls_ctx_error
eXosip_tls_use_server_certificate (struct eXosip_t * excontext, const char *local_certificate_cn)
{
  memset (&excontext->tls_local_cn_name, 0, sizeof (excontext->tls_local_cn_name));
  if (local_certificate_cn == NULL)
    return TLS_OK;
  osip_strncpy (excontext->tls_local_cn_name, local_certificate_cn, sizeof (excontext->tls_local_cn_name) - 1);
  return TLS_OK;
}

eXosip_tls_ctx_error
eXosip_tls_use_client_certificate (struct eXosip_t * excontext, const char *local_certificate_cn)
{
  memset (&excontext->tls_client_local_cn_name, 0, sizeof (excontext->tls_client_local_cn_name));
  if (local_certificate_cn == NULL)
    return TLS_OK;
  osip_strncpy (excontext->tls_client_local_cn_name, local_certificate_cn, sizeof (excontext->tls_client_local_cn_name) - 1);
  return TLS_OK;
}

eXosip_tls_ctx_error
eXosip_tls_verify_certificate (struct eXosip_t * excontext, int _tls_verify_client_certificate)
{
  excontext->tls_verify_client_certificate = _tls_verify_client_certificate;
  return TLS_OK;
}

SSL_CTX *
initialize_client_ctx (struct eXosip_t * excontext, const char *certif_client_local_cn_name, eXosip_tls_ctx_t * client_ctx, int transport, const char *sni_servernameindication)
{
  const SSL_METHOD *meth = NULL;
  X509 *cert = NULL;
  SSL_CTX *ctx;

  if (transport == IPPROTO_UDP) {
#if !(OPENSSL_VERSION_NUMBER < 0x10002000L)
    meth = DTLS_client_method ();
#elif !(OPENSSL_VERSION_NUMBER < 0x00908000L)
    meth = DTLSv1_client_method ();
#endif
  }
  else if (transport == IPPROTO_TCP) {
    meth = SSLv23_client_method ();
  }
  else {
    return NULL;
  }

  ctx = SSL_CTX_new (meth);

  if (ctx == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't create SSL_CTX!\n"));
    return NULL;
  }

  if (client_ctx->client.priv_key_pw[0] != '\0') {
    SSL_CTX_set_default_passwd_cb_userdata (ctx, (void *) client_ctx->client.priv_key_pw);
    SSL_CTX_set_default_passwd_cb (ctx, password_cb);
  }

  if (certif_client_local_cn_name[0] != '\0') {
    cert = _tls_set_certificate (ctx, certif_client_local_cn_name);
  }

  if (cert == NULL && client_ctx->client.cert[0] != '\0') {
    /* Load our keys and certificates */
    if (client_ctx->root_ca_cert[0] != '\0') {
      if (!(SSL_CTX_use_certificate_file (ctx, client_ctx->client.cert, SSL_FILETYPE_PEM))) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't read client certificate file %s!\n", client_ctx->client.cert));
      }

      if (!(SSL_CTX_use_PrivateKey_file (ctx, client_ctx->client.priv_key, SSL_FILETYPE_PEM)))
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't read client pkey file %s!\n", client_ctx->client.priv_key));

      if (!(SSL_CTX_use_RSAPrivateKey_file (ctx, client_ctx->client.priv_key, SSL_FILETYPE_PEM)))
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't read client RSA key file %s!\n", client_ctx->client.priv_key));
    }
  }
  else {
    BIO *bio = BIO_new_file (client_ctx->client.cert, "r");

    if (bio != NULL) {
      PEM_read_bio_X509 (bio, &cert, 0, NULL);
      if (cert == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't read client certificate file %s!\n", client_ctx->client.cert));
      }
      else {
        /* this is used to add a trusted certificate */
        X509_STORE_add_cert (ctx->cert_store, cert);
      }
      BIO_free (bio);
    }
  }

  if (cert != NULL) {
    X509_free (cert);
    cert = NULL;
  }

  /* Load the CAs we trust */
  {
    char *caFile = 0, *caFolder = 0;

#ifdef WIN32
    WIN32_FIND_DATA FileData;
    HANDLE hSearch;
    char szDirPath[1024];
    WCHAR wUnicodeDirPath[2048];

    snprintf (szDirPath, sizeof (szDirPath), "%s", client_ctx->root_ca_cert);

    MultiByteToWideChar (CP_UTF8, 0, szDirPath, -1, wUnicodeDirPath, 2048);
    hSearch = FindFirstFileEx (wUnicodeDirPath, FindExInfoStandard, &FileData, FindExSearchNameMatch, NULL, 0);
    if (hSearch != INVALID_HANDLE_VALUE) {
      if ((FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
        caFolder = client_ctx->root_ca_cert;
      else
        caFile = client_ctx->root_ca_cert;
    }
    else {
      caFile = client_ctx->root_ca_cert;
    }
#else
    int fd = open (client_ctx->root_ca_cert, O_RDONLY);

    if (fd >= 0) {
      struct stat fileStat;

      if (fstat (fd, &fileStat) < 0) {

      }
      else {
        if (S_ISDIR (fileStat.st_mode)) {
          caFolder = client_ctx->root_ca_cert;
        }
        else {
          caFile = client_ctx->root_ca_cert;
        }
      }
      close (fd);
    }
#endif


    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "eXosip: Trusted CA %s : '%s'\n", caFolder ? "folder" : "file", client_ctx->root_ca_cert));

    if ((caFile!=NULL && caFile[0] != '\0') || (caFolder!=NULL && caFolder[0] != '\0')) {
      if (!(SSL_CTX_load_verify_locations (ctx, caFile, caFolder))) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't read client root_ca_cert list ('%s')\n", client_ctx->root_ca_cert));
      }
    }

    {
#if !(OPENSSL_VERSION_NUMBER < 0x10002000L)
      int verify_mode = SSL_VERIFY_NONE;

      if (excontext->tls_verify_client_certificate > 0 && sni_servernameindication!=NULL) {
	X509_STORE *pkix_validation_store = SSL_CTX_get_cert_store (ctx);
	const X509_VERIFY_PARAM *param = X509_VERIFY_PARAM_lookup ("ssl_server");
	
	if (param != NULL) { /* const value, we have to copy (inherit) */
	  if (X509_VERIFY_PARAM_inherit (pkix_validation_store->param, param)) {
	    X509_STORE_set_flags (pkix_validation_store, X509_V_FLAG_TRUSTED_FIRST);
	    X509_STORE_set_flags (pkix_validation_store, X509_V_FLAG_PARTIAL_CHAIN);
	  } else {
	    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "PARAM_inherit: failed for ssl_server\n"));
	  }
	  if (X509_VERIFY_PARAM_set1_host (pkix_validation_store->param, sni_servernameindication, 0)) {
	    X509_VERIFY_PARAM_set_hostflags (pkix_validation_store->param, X509_CHECK_FLAG_NO_WILDCARDS);
	  } else {
	    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "PARAM_set1_host: %s failed\n", sni_servernameindication));
	  }
	} else {
	  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "PARAM_lookup: failed for ssl_server\n"));
	}
      }

      if (excontext->tls_verify_client_certificate > 0)
        verify_mode = SSL_VERIFY_PEER;
      SSL_CTX_set_verify (ctx, verify_mode, &verify_cb);
#else
      int verify_mode = SSL_VERIFY_NONE;

      if (excontext->tls_verify_client_certificate > 0)
        verify_mode = SSL_VERIFY_PEER;

      SSL_CTX_set_verify (ctx, verify_mode, &verify_cb);
#endif
      SSL_CTX_set_verify_depth (ctx, ex_verify_depth + 1);
    }
  }

  SSL_CTX_set_options (ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);

  if (!SSL_CTX_set_cipher_list (ctx, "HIGH:-COMPLEMENTOFDEFAULT")) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "set_cipher_list: using DEFAULT list now\n"));
  }

  if (_tls_add_certificates (ctx) <= 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "no system certificate loaded\n"));
  }

  return ctx;
}

SSL_CTX *
initialize_server_ctx (struct eXosip_t * excontext, const char *certif_local_cn_name, eXosip_tls_ctx_t * srv_ctx, int transport)
{
  const SSL_METHOD *meth = NULL;
  SSL_CTX *ctx;
  X509 *cert = NULL;

  int s_server_session_id_context = 1;

  if (transport == IPPROTO_UDP) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "DTLS-UDP server method\n"));
#if !(OPENSSL_VERSION_NUMBER < 0x10002000L)
    meth = DTLS_server_method ();
#elif !(OPENSSL_VERSION_NUMBER < 0x00908000L)
    meth = DTLSv1_server_method ();
#endif
  }
  else if (transport == IPPROTO_TCP) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "TLS server method\n"));
    meth = SSLv23_server_method ();
  }
  else {
    return NULL;
  }

  ctx = SSL_CTX_new (meth);

  if (ctx == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't create SSL_CTX!\n"));
    SSL_CTX_free (ctx);
    return NULL;
  }

  if (transport == IPPROTO_UDP) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "DTLS-UDP read ahead\n"));
    SSL_CTX_set_read_ahead (ctx, 1);
  }

  if (certif_local_cn_name[0] != '\0') {
    cert = _tls_set_certificate (ctx, certif_local_cn_name);
  }
  if (cert == NULL && srv_ctx->server.cert[0] != '\0') {
    if (!(SSL_CTX_use_certificate_file (ctx, srv_ctx->server.cert, SSL_FILETYPE_PEM))) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't read certificate file!\n"));
      SSL_CTX_free (ctx);
      return NULL;
    }
    /* THIS CODE IS WRONG??? */
    /* SSL_CTX_free(ctx); */
    /* return NULL; */
  }

  if (_tls_add_certificates (ctx) <= 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "no system certificate loaded\n"));
  }

  generate_eph_rsa_key (ctx);

  SSL_CTX_set_session_id_context (ctx, (void *) &s_server_session_id_context, sizeof s_server_session_id_context);

  if (srv_ctx->server.priv_key_pw[0] != '\0') {
    SSL_CTX_set_default_passwd_cb_userdata (ctx, (void *) srv_ctx->server.priv_key_pw);
    SSL_CTX_set_default_passwd_cb (ctx, password_cb);
  }

  /* Load the CAs we trust */
  if (srv_ctx->root_ca_cert[0]!='\0') {
    if (!(SSL_CTX_load_verify_locations (ctx, srv_ctx->root_ca_cert, 0))) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't read server root_ca_cert ('%s')\n", srv_ctx->root_ca_cert));
    }
  }
  {
    int verify_mode = SSL_VERIFY_NONE;

    /*verify_mode = SSL_VERIFY_PEER; */

    SSL_CTX_set_verify (ctx, verify_mode, &verify_cb);
    SSL_CTX_set_verify_depth (ctx, ex_verify_depth + 1);
  }

  SSL_CTX_set_options (ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION | SSL_OP_SINGLE_ECDH_USE | SSL_OP_SINGLE_DH_USE | SSL_OP_CIPHER_SERVER_PREFERENCE);

  if (!SSL_CTX_set_cipher_list (ctx, "HIGH:-COMPLEMENTOFDEFAULT")) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "set_cipher_list: using DEFAULT list now\n"));
  }

  if (cert == NULL && srv_ctx->server.priv_key[0] != '\0') {
    if (!(SSL_CTX_use_PrivateKey_file (ctx, srv_ctx->server.priv_key, SSL_FILETYPE_PEM))) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Couldn't read key file: %s\n", srv_ctx->server.priv_key));
      SSL_CTX_free (ctx);
      return NULL;
    }
  }

  if (cert != NULL || srv_ctx->server.cert[0] != '\0') {
    if (!SSL_CTX_check_private_key (ctx)) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "check_private_key: Key does not match the public key of the certificate\n"));
      SSL_CTX_free (ctx);
      return NULL;
    }
  }

  if (cert == NULL && srv_ctx->server.cert[0] == '\0') {
    if (!SSL_CTX_set_cipher_list (ctx, "ADH")) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "set_cipher_list: cannot set anonymous DH cipher\n"));
      SSL_CTX_free (ctx);
      return NULL;
    }
  }

  if (cert != NULL) {
    X509_free (cert);
    cert = NULL;
  }

  if (cert == NULL && srv_ctx->server.cert[0] == '\0')
    build_dh_params (ctx);
  else
    load_dh_params (ctx, srv_ctx->dh_param);

#ifndef SSL_CTRL_SET_ECDH_AUTO
  #define SSL_CTRL_SET_ECDH_AUTO 94
#endif

  /* SSL_CTX_set_ecdh_auto (ctx, on) requires OpenSSL 1.0.2 which wraps: */
  if (SSL_CTX_ctrl (ctx, SSL_CTRL_SET_ECDH_AUTO, 1, NULL)) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "ctrl_set_ecdh_auto: faster PFS ciphers enabled\n"));
#if !defined(OPENSSL_NO_ECDH) && !(OPENSSL_VERSION_NUMBER < 0x10000000L)
  } else {
    /* enables AES-128 ciphers, to get AES-256 use NID_secp384r1 */
    EC_KEY *ecdh = EC_KEY_new_by_curve_name (NID_X9_62_prime256v1);
    if (ecdh != NULL) {
      if (SSL_CTX_set_tmp_ecdh (ctx, ecdh)) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "set_tmp_ecdh: faster PFS ciphers enabled (secp256r1)\n"));
      }
      EC_KEY_free(ecdh);
    }
#endif
  }

  generate_eph_rsa_key (ctx);

  SSL_CTX_set_session_id_context (ctx, (void *) &s_server_session_id_context, sizeof s_server_session_id_context);

  return ctx;
}

/**
* @brief Initializes the OpenSSL lib and the client/server contexts.
* Depending on the previously initialized eXosip TLS context (see eXosip_set_tls_ctx() ), only the necessary contexts will be initialized.
* The client context will be ALWAYS initialized, the server context only if certificates are available. The following chart should illustrate
* the behaviour.
*
* possible certificates  | Client initialized			  | Server initialized
* -------------------------------------------------------------------------------------
* no certificate		 | yes, no cert used			  | not initialized
* only client cert		 | yes, own cert (client) used    | yes, client cert used
* only server cert		 | yes, server cert used		  | yes, own cert (server) used
* server and client cert | yes, own cert (client) used    | yes, own cert (server) used
*
* The file for seeding the PRNG is only needed on Windows machines. If you compile under a Windows environment, please set W32 oder _WINDOWS as
* Preprocessor directives.
*@return < 0 if an error occured
**/
static int
tls_tl_open (struct eXosip_t *excontext)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;
  int res;
  struct addrinfo *addrinfo = NULL;
  struct addrinfo *curinfo;
  int sock = -1;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  excontext->eXtl_transport.proto_local_port = excontext->eXtl_transport.proto_port;
  if (excontext->eXtl_transport.proto_local_port < 0)
    excontext->eXtl_transport.proto_local_port = 5061;

  /* initialization (outside initialize_server_ctx) */
  SSL_library_init ();
  SSL_load_error_strings ();

  reserved->server_ctx = initialize_server_ctx (excontext, excontext->tls_local_cn_name, &excontext->eXosip_tls_ctx_params, IPPROTO_TCP);

  /* always initialize the client */
  reserved->client_ctx = initialize_client_ctx (excontext, excontext->tls_client_local_cn_name, &excontext->eXosip_tls_ctx_params, IPPROTO_TCP, NULL);

/*only necessary under Windows-based OS, unix-like systems use /dev/random or /dev/urandom */
#if defined(WIN32) || defined(_WINDOWS)

#if 0
  /* check if a file with random data is present --> will be verified when random file is needed */
  if (reserved->eXosip_tls_ctx_params.random_file[0] == '\0') {
    return TLS_ERR_NO_RAND;
  }
#endif

  /* Load randomness */
  if (!(RAND_load_file (excontext->eXosip_tls_ctx_params.random_file, 1024 * 1024)))
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "eXosip: Couldn't load randomness\n"));
#endif

  res = _eXosip_get_addrinfo (excontext, &addrinfo, excontext->eXtl_transport.proto_ifs, excontext->eXtl_transport.proto_local_port, excontext->eXtl_transport.proto_num);
  if (res)
    return -1;

  for (curinfo = addrinfo; curinfo; curinfo = curinfo->ai_next) {
    socklen_t len;

    if (curinfo->ai_protocol && curinfo->ai_protocol != excontext->eXtl_transport.proto_num) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "eXosip: Skipping protocol %d\n", curinfo->ai_protocol));
      continue;
    }

    sock = (int) socket (curinfo->ai_family, curinfo->ai_socktype, curinfo->ai_protocol);
    if (sock < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot create socket %s!\n", strerror (ex_errno)));
      continue;
    }

    if (curinfo->ai_family == AF_INET6) {
#ifdef IPV6_V6ONLY
      if (setsockopt_ipv6only (sock)) {
        _eXosip_closesocket (sock);
        sock = -1;
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot set socket option %s!\n", strerror (ex_errno)));
        continue;
      }
#endif /* IPV6_V6ONLY */
    }

    {
      int valopt = 1;

      setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (void *) &valopt, sizeof (valopt));
    }

#ifndef DISABLE_MAIN_SOCKET
    res = bind (sock, curinfo->ai_addr, (socklen_t)curinfo->ai_addrlen);
    if (res < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot bind socket node:%s family:%d %s\n", excontext->eXtl_transport.proto_ifs, curinfo->ai_family, strerror (ex_errno)));
      _eXosip_closesocket (sock);
      sock = -1;
      continue;
    }
    len = sizeof (reserved->ai_addr);
    res = getsockname (sock, (struct sockaddr *) &reserved->ai_addr, &len);
    if (res != 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot get socket name (%s)\n", strerror (ex_errno)));
      memcpy (&reserved->ai_addr, curinfo->ai_addr, curinfo->ai_addrlen);
    }
    reserved->ai_addr_len=len;

    if (excontext->eXtl_transport.proto_num == IPPROTO_TCP) {
      res = listen (sock, SOMAXCONN);
      if (res < 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot bind socket node:%s family:%d %s\n", excontext->eXtl_transport.proto_ifs, curinfo->ai_family, strerror (ex_errno)));
        _eXosip_closesocket (sock);
        sock = -1;
        continue;
      }
    }
#endif

    break;
  }

  _eXosip_freeaddrinfo (addrinfo);

  if (sock < 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "eXosip: Cannot bind on port: %i\n", excontext->eXtl_transport.proto_local_port));
    return -1;
  }

  reserved->tls_socket = sock;

  if (excontext->eXtl_transport.proto_local_port == 0) {
    /* get port number from socket */
    if (excontext->eXtl_transport.proto_family == AF_INET)
      excontext->eXtl_transport.proto_local_port = ntohs (((struct sockaddr_in *) &reserved->ai_addr)->sin_port);
    else
      excontext->eXtl_transport.proto_local_port = ntohs (((struct sockaddr_in6 *) &reserved->ai_addr)->sin6_port);
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "eXosip: Binding on port %i!\n", excontext->eXtl_transport.proto_local_port));
  }
  return OSIP_SUCCESS;
}

static int
tls_tl_set_fdset (struct eXosip_t *excontext, fd_set * osip_fdset, fd_set * osip_wrset, int *fd_max)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;
  int pos;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

#ifndef DISABLE_MAIN_SOCKET
  if (reserved->tls_socket <= 0)
    return -1;

  eXFD_SET (reserved->tls_socket, osip_fdset);

  if (reserved->tls_socket > *fd_max)
    *fd_max = reserved->tls_socket;
#endif

  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    if (reserved->socket_tab[pos].socket > 0) {
      eXFD_SET (reserved->socket_tab[pos].socket, osip_fdset);
      if (reserved->socket_tab[pos].socket > *fd_max)
        *fd_max = reserved->socket_tab[pos].socket;
      if (reserved->socket_tab[pos].sendbuflen > 0)
        eXFD_SET (reserved->socket_tab[pos].socket, osip_wrset);
    }
  }

  return OSIP_SUCCESS;
}

int static
print_ssl_error (int err)
{
  switch (err) {
  case SSL_ERROR_NONE:
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "SSL ERROR NONE - OK\n"));
    break;
  case SSL_ERROR_ZERO_RETURN:
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "SSL ERROR ZERO RETURN - SHUTDOWN\n"));
    break;
  case SSL_ERROR_WANT_READ:
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "SSL want read\n"));
    break;
  case SSL_ERROR_WANT_WRITE:
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "SSL want write\n"));
    break;
  case SSL_ERROR_SSL:
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "SSL ERROR\n"));
    break;
  case SSL_ERROR_SYSCALL:
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "SSL ERROR SYSCALL\n"));
    break;
  default:
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "SSL problem\n"));
  }
  return OSIP_SUCCESS;
}


static void
tls_dump_verification_failure (long verification_result)
{
  char tmp[64];

  snprintf (tmp, sizeof (tmp), "unknown errror");
  switch (verification_result) {
  case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
    snprintf (tmp, sizeof (tmp), "unable to get issuer certificate");
    break;
  case X509_V_ERR_UNABLE_TO_GET_CRL:
    snprintf (tmp, sizeof (tmp), "unable to get certificate CRL");
    break;
  case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
    snprintf (tmp, sizeof (tmp), "unable to decrypt certificate's signature");
    break;
  case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
    snprintf (tmp, sizeof (tmp), "unable to decrypt CRL's signature");
    break;
  case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
    snprintf (tmp, sizeof (tmp), "unable to decode issuer public key");
    break;
  case X509_V_ERR_CERT_SIGNATURE_FAILURE:
    snprintf (tmp, sizeof (tmp), "certificate signature failure");
    break;
  case X509_V_ERR_CRL_SIGNATURE_FAILURE:
    snprintf (tmp, sizeof (tmp), "CRL signature failure");
    break;
  case X509_V_ERR_CERT_NOT_YET_VALID:
    snprintf (tmp, sizeof (tmp), "certificate is not yet valid");
    break;
  case X509_V_ERR_CERT_HAS_EXPIRED:
    snprintf (tmp, sizeof (tmp), "certificate has expired");
    break;
  case X509_V_ERR_CRL_NOT_YET_VALID:
    snprintf (tmp, sizeof (tmp), "CRL is not yet valid");
    break;
  case X509_V_ERR_CRL_HAS_EXPIRED:
    snprintf (tmp, sizeof (tmp), "CRL has expired");
    break;
  case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
    snprintf (tmp, sizeof (tmp), "format error in certificate's notBefore field");
    break;
  case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
    snprintf (tmp, sizeof (tmp), "format error in certificate's notAfter field");
    break;
  case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
    snprintf (tmp, sizeof (tmp), "format error in CRL's lastUpdate field");
    break;
  case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
    snprintf (tmp, sizeof (tmp), "format error in CRL's nextUpdate field");
    break;
  case X509_V_ERR_OUT_OF_MEM:
    snprintf (tmp, sizeof (tmp), "out of memory");
    break;
  case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
    snprintf (tmp, sizeof (tmp), "self signed certificate");
    break;
  case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
    snprintf (tmp, sizeof (tmp), "self signed certificate in certificate chain");
    break;
  case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
    snprintf (tmp, sizeof (tmp), "unable to get local issuer certificate");
    break;
  case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
    snprintf (tmp, sizeof (tmp), "unable to verify the first certificate");
    break;
  case X509_V_ERR_CERT_CHAIN_TOO_LONG:
    snprintf (tmp, sizeof (tmp), "certificate chain too long");
    break;
  case X509_V_ERR_CERT_REVOKED:
    snprintf (tmp, sizeof (tmp), "certificate revoked");
    break;
  case X509_V_ERR_INVALID_CA:
    snprintf (tmp, sizeof (tmp), "invalid CA certificate");
    break;
  case X509_V_ERR_PATH_LENGTH_EXCEEDED:
    snprintf (tmp, sizeof (tmp), "path length constraint exceeded");
    break;
  case X509_V_ERR_INVALID_PURPOSE:
    snprintf (tmp, sizeof (tmp), "unsupported certificate purpose");
    break;
  case X509_V_ERR_CERT_UNTRUSTED:
    snprintf (tmp, sizeof (tmp), "certificate not trusted");
    break;
  case X509_V_ERR_CERT_REJECTED:
    snprintf (tmp, sizeof (tmp), "certificate rejected");
    break;
  case X509_V_ERR_SUBJECT_ISSUER_MISMATCH:
    snprintf (tmp, sizeof (tmp), "subject issuer mismatch");
    break;
  case X509_V_ERR_AKID_SKID_MISMATCH:
    snprintf (tmp, sizeof (tmp), "authority and subject key identifier mismatch");
    break;
  case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
    snprintf (tmp, sizeof (tmp), "authority and issuer serial number mismatch");
    break;
  case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
    snprintf (tmp, sizeof (tmp), "key usage does not include certificate signing");
    break;
  case X509_V_ERR_APPLICATION_VERIFICATION:
    snprintf (tmp, sizeof (tmp), "application verification failure");
    break;
  }

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "verification failure: %s\n", tmp));
}

static int
_tls_tl_is_connected (int sock)
{
  int res;
  struct timeval tv;
  fd_set wrset;
  int valopt;
  socklen_t sock_len;

  tv.tv_sec = SOCKET_TIMEOUT / 1000;
  tv.tv_usec = (SOCKET_TIMEOUT % 1000) * 1000;

  FD_ZERO (&wrset);
  FD_SET (sock, &wrset);

  res = select (sock + 1, NULL, &wrset, NULL, &tv);
  if (res > 0) {
    sock_len = sizeof (int);
    if (getsockopt (sock, SOL_SOCKET, SO_ERROR, (void *) (&valopt), &sock_len)
        == 0) {
      if (valopt) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot connect socket node / %s[%d]\n", strerror (ex_errno), valopt));
        return -1;
      }
      else {
        return 0;
      }
    }
    else {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot connect socket node / error in getsockopt %s[%d]\n", strerror (ex_errno), ex_errno));
      return -1;
    }
  }
  else if (res < 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot connect socket node / error in select %s[%d]\n", strerror (ex_errno), ex_errno));
    return -1;
  }
  else {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot connect socket node / select timeout (%d ms)\n", SOCKET_TIMEOUT));
    return 1;
  }
}

static int
_tls_tl_check_connected (struct eXosip_t *excontext)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;
  int pos;
  int res;

  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    if (reserved->socket_tab[pos].invalid > 0) {
      OSIP_TRACE (osip_trace
                  (__FILE__, __LINE__, OSIP_INFO2, NULL,
                   "_tls_tl_check_connected: socket node is in invalid state:%s:%i, socket %d [pos=%d], family:%d\n",
                   reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos, reserved->socket_tab[pos].ai_addr.sa_family));
      _tls_tl_close_sockinfo (&reserved->socket_tab[pos]);
      continue;
    }

    if (reserved->socket_tab[pos].socket > 0 && reserved->socket_tab[pos].ai_addrlen > 0) {
      if (reserved->socket_tab[pos].ssl_state > 0) {
        /* already connected */
        reserved->socket_tab[pos].ai_addrlen = 0;
        continue;
      }

      res = _tls_tl_is_connected (reserved->socket_tab[pos].socket);
      if (res > 0) {
#if 0
        /* bug: calling connect several times for TCP is not allowed by specification */
        res = connect (reserved->socket_tab[pos].socket, &reserved->socket_tab[pos].ai_addr, reserved->socket_tab[pos].ai_addrlen);
        if (res<0) {
          int status = ex_errno;
          OSIP_TRACE (osip_trace
                      (__FILE__, __LINE__, OSIP_INFO2, NULL,
                       "_tls_tl_check_connected: connect being called again (res=%i) (errno=%i) (%s)\n", res, status, strerror (status)));
        }
#endif
        OSIP_TRACE (osip_trace
                    (__FILE__, __LINE__, OSIP_INFO2, NULL,
                     "_tls_tl_check_connected: socket node:%s:%i, socket %d [pos=%d], family:%d, in progress\n",
                     reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos, reserved->socket_tab[pos].ai_addr.sa_family));
        continue;
      }
      else if (res == 0) {
        OSIP_TRACE (osip_trace
                    (__FILE__, __LINE__, OSIP_INFO1, NULL,
                     "_tls_tl_check_connected: socket node:%s:%i , socket %d [pos=%d], family:%d, connected\n",
                     reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos, reserved->socket_tab[pos].ai_addr.sa_family));
        /* stop calling "connect()" */
        reserved->socket_tab[pos].ai_addrlen = 0;
        reserved->socket_tab[pos].ssl_state = 1;
        continue;
      }
      else {
        OSIP_TRACE (osip_trace
                    (__FILE__, __LINE__, OSIP_INFO2, NULL,
                     "_tls_tl_check_connected: socket node:%s:%i, socket %d [pos=%d], family:%d, error\n",
                     reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port, reserved->socket_tab[pos].socket, pos, reserved->socket_tab[pos].ai_addr.sa_family));
        _tls_tl_close_sockinfo (&reserved->socket_tab[pos]);
        continue;
      }
    }
  }
  return 0;
}

static int
_tls_tl_ssl_connect_socket (struct eXosip_t *excontext, struct _tls_stream *sockinfo)
{
  X509 *cert;
  BIO *sbio;
  int res;
  int tries_left = 100;

  if (sockinfo->ssl_ctx == NULL) {
    sockinfo->ssl_ctx = initialize_client_ctx (excontext, excontext->tls_client_local_cn_name, &excontext->eXosip_tls_ctx_params, IPPROTO_TCP, sockinfo->sni_servernameindication);

    /* FIXME: changed parameter from ctx to client_ctx -> works now */
    sockinfo->ssl_conn = SSL_new (sockinfo->ssl_ctx);
    if (sockinfo->ssl_conn == NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "SSL_new error\n"));
      return -1;
    }
    sbio = BIO_new_socket (sockinfo->socket, BIO_NOCLOSE);

    if (sbio == NULL) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "BIO_new_socket error\n"));
      return -1;
    }
    SSL_set_bio (sockinfo->ssl_conn, sbio, sbio);

#ifndef OPENSSL_NO_TLSEXT
    if (!SSL_set_tlsext_host_name (sockinfo->ssl_conn, sockinfo->sni_servernameindication /* "host.name.after.dns.srv.com" */)) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "set_tlsext_host_name (SNI): no servername gets indicated\n"));
    }
#endif
  }

  if (SSL_is_init_finished (sockinfo->ssl_conn)) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "SSL_is_init_finished already done\n"));
  }
  else {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "SSL_is_init_finished not already done\n"));
  }

  do {
    struct timeval tv;
    int fd;
    fd_set readfds;

    res = SSL_connect (sockinfo->ssl_conn);
    res = SSL_get_error (sockinfo->ssl_conn, res);
    if (res == SSL_ERROR_NONE) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "SSL_connect succeeded\n"));
      break;
    }

    if (res != SSL_ERROR_WANT_READ && res != SSL_ERROR_WANT_WRITE) {
      print_ssl_error (res);
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "SSL_connect error\n"));

      /* any transaction here should fail? --> but this is called from _tls_tl_recv not the send side... */

      return -1;
    }

    tv.tv_sec = SOCKET_TIMEOUT / 1000;
    tv.tv_usec = (SOCKET_TIMEOUT % 1000) * 1000;
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "SSL_connect retry\n"));

    fd = SSL_get_fd (sockinfo->ssl_conn);
    FD_ZERO (&readfds);
    FD_SET (fd, &readfds);
    res = select (fd + 1, &readfds, NULL, NULL, &tv);
    if (res < 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "SSL_connect select(read) error (%s)\n", strerror (ex_errno)));
      return -1;
    }
    else if (res > 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "SSL_connect (read done)\n"));
    }
    else {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "SSL_connect (timeout not data to read) (%d ms)\n", SOCKET_TIMEOUT));
      return 1;
    }
  } while (!SSL_is_init_finished (sockinfo->ssl_conn) && ((tries_left--) > 0));

  if (SSL_is_init_finished (sockinfo->ssl_conn)) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "SSL_is_init_finished done\n"));
  }
  else {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "SSL_is_init_finished failed\n"));
  }

  cert = SSL_get_peer_certificate (sockinfo->ssl_conn);
  if (cert != 0) {
    long cert_err;

    tls_dump_cert_info ("tls_connect: remote certificate: ", cert);

    cert_err = SSL_get_verify_result (sockinfo->ssl_conn);
    if (cert_err != X509_V_OK) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "Failed to verify remote certificate\n"));
      tls_dump_verification_failure (cert_err);

    }
    X509_free (cert);
  }
  else {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "No certificate received\n"));
    /* X509_free is not necessary because no cert-object was created -> cert == NULL */
    if (excontext->eXosip_tls_ctx_params.server.cert[0] == '\0') {
#ifdef ENABLE_ADH
      /* how can we guess a user want ADH... specific APIs.. */
      sockinfo->ssl_state = 3;
      return 0;
#endif
    }

    return -1;
  }

  sockinfo->ssl_state = 3;
  return 0;
}

/* Like strstr, but works for haystack that may contain binary data and is
   not NUL-terminated. */
static char *
buffer_find (const char *haystack, size_t haystack_len, const char *needle)
{
  const char *search = haystack, *end = haystack + haystack_len;
  char *p;
  size_t len = strlen (needle);

  while (search < end && (p = memchr (search, *needle, end - search)) != NULL) {
    if (p + len > end)
      break;
    if (memcmp (p, needle, len) == 0)
      return (p);
    search = p + 1;
  }

  return (NULL);
}

#define END_HEADERS_STR "\r\n\r\n"
#define CLEN_HEADER_STR "\r\ncontent-length:"
#define CLEN_HEADER_COMPACT_STR "\r\nl:"
#define CLEN_HEADER_STR2 "\r\ncontent-length "
#define CLEN_HEADER_COMPACT_STR2 "\r\nl "
#define const_strlen(x) (sizeof((x)) - 1)

/* consume any complete messages in sockinfo->buf and
   return the total number of bytes consumed */
static int
handle_messages (struct eXosip_t *excontext, struct _tls_stream *sockinfo)
{
  int consumed = 0;
  char *buf = sockinfo->buf;
  size_t buflen = sockinfo->buflen;
  char *end_headers;

  while (buflen > 0 && (end_headers = buffer_find (buf, buflen, END_HEADERS_STR)) != NULL) {
    int clen, msglen;
    char *clen_header;

    if (buf == end_headers) {
      /* skip tcp standard keep-alive */
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "socket %s:%i: standard keep alive received (CRLFCRLF)\n", sockinfo->remote_ip, sockinfo->remote_port, buf));
      consumed += 4;
      buflen -= 4;
      buf += 4;
      continue;
    }

    /* stuff a nul in so we can use osip_strcasestr */
    *end_headers = '\0';

    /* ok we have complete headers, find content-length: or l: */
    clen_header = osip_strcasestr (buf, CLEN_HEADER_STR);
    if (!clen_header)
      clen_header = osip_strcasestr (buf, CLEN_HEADER_STR2);
    if (!clen_header)
      clen_header = osip_strcasestr (buf, CLEN_HEADER_COMPACT_STR);
    if (!clen_header)
      clen_header = osip_strcasestr (buf, CLEN_HEADER_COMPACT_STR2);
    if (clen_header != NULL) {
      clen_header = strchr (clen_header, ':');
      clen_header++;
    }
    if (!clen_header) {
      /* Oops, no content-length header.      Presume 0 (below) so we
         consume the headers and make forward progress.  This permits
         server-side keepalive of "\r\n\r\n". */
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "socket %s:%i: message has no content-length: <%s>\n", sockinfo->remote_ip, sockinfo->remote_port, buf));
    }
    clen = clen_header ? atoi (clen_header) : 0;
    if (clen<0)
      return (int)sockinfo->buflen; /* discard data */
    /* undo our overwrite and advance end_headers */
    *end_headers = END_HEADERS_STR[0];
    end_headers += const_strlen (END_HEADERS_STR);

    /* do we have the whole message? */
    msglen = (int) (end_headers - buf + clen);
    if (msglen > buflen) {
      /* nope */
      return consumed;
    }
    /* yep; handle the message */
    _eXosip_handle_incoming_message (excontext, buf, msglen, sockinfo->socket, sockinfo->remote_ip, sockinfo->remote_port, sockinfo->natted_ip, &sockinfo->natted_port);
    consumed += msglen;
    buflen -= msglen;
    buf += msglen;
  }

  return consumed;
}

static int
_tls_tl_recv (struct eXosip_t *excontext, struct _tls_stream *sockinfo)
{
  int r;
  int rlen, err;

  if (!sockinfo->buf) {
    sockinfo->buf = (char *) osip_malloc (SIP_MESSAGE_MAX_LENGTH);
    if (sockinfo->buf == NULL)
      return OSIP_NOMEM;
    sockinfo->bufsize = SIP_MESSAGE_MAX_LENGTH;
    sockinfo->buflen = 0;
  }

  /* buffer is 100% full -> realloc with more size */
  if (sockinfo->bufsize - sockinfo->buflen <= 0) {
    sockinfo->buf = (char *) osip_realloc (sockinfo->buf, sockinfo->bufsize + 5000);
    if (sockinfo->buf == NULL)
      return OSIP_NOMEM;
    sockinfo->bufsize = sockinfo->bufsize + 5000;
  }

  /* buffer is 100% empty-> realloc with initial size */
  if (sockinfo->buflen == 0 && sockinfo->bufsize > SIP_MESSAGE_MAX_LENGTH) {
    osip_free (sockinfo->buf);
    sockinfo->buf = (char *) osip_malloc (SIP_MESSAGE_MAX_LENGTH);
    if (sockinfo->buf == NULL)
      return OSIP_NOMEM;
    sockinfo->bufsize = SIP_MESSAGE_MAX_LENGTH;
  }


  /* do TLS handshake? */

  if (sockinfo->ssl_state == 0) {
    r = _tls_tl_is_connected (sockinfo->socket);
    if (r > 0) {
      return OSIP_SUCCESS;
    }
    else if (r == 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "socket node:%s , socket %d [pos=%d], connected\n", sockinfo->remote_ip, sockinfo->socket, -1));
      sockinfo->ssl_state = 1;
      sockinfo->ai_addrlen = 0;
    }
    else {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "socket node:%s, socket %d [pos=%d], socket error\n", sockinfo->remote_ip, sockinfo->socket, -1));
      _tls_tl_close_sockinfo (sockinfo);
      return OSIP_SUCCESS;
    }
  }

  if (sockinfo->ssl_state == 1) {
    r = _tls_tl_ssl_connect_socket (excontext, sockinfo);
    if (r < 0) {
      _tls_tl_close_sockinfo (sockinfo);
      return OSIP_SUCCESS;
    }
  }

  if (sockinfo->ssl_state == 2) {
    r = SSL_do_handshake (sockinfo->ssl_conn);
    if (r <= 0) {
      r = SSL_get_error (sockinfo->ssl_conn, r);
      print_ssl_error (r);

      _tls_tl_close_sockinfo (sockinfo);
      return OSIP_SUCCESS;
    }
    sockinfo->ssl_state = 3;
  }

  if (sockinfo->ssl_state != 3)
    return OSIP_SUCCESS;



  r = 0;
  rlen = 0;

  do {
    r = SSL_read (sockinfo->ssl_conn, sockinfo->buf + sockinfo->buflen + rlen, (int) (sockinfo->bufsize - sockinfo->buflen - rlen));
    if (r <= 0) {
      err = SSL_get_error (sockinfo->ssl_conn, r);
      if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
        break;
      }
      else {
        print_ssl_error (err);
        /*
           The TLS/SSL connection has been closed.  If the protocol version
           is SSL 3.0 or TLS 1.0, this result code is returned only if a
           closure alert has occurred in the protocol, i.e. if the
           connection has been closed cleanly. Note that in this case
           SSL_ERROR_ZERO_RETURN does not necessarily indicate that the
           underlying transport has been closed. */
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "TLS closed\n"));

        _eXosip_mark_registration_expired (excontext, sockinfo->reg_call_id);
        _tls_tl_close_sockinfo (sockinfo);

        rlen = 0;               /* discard any remaining data ? */
        break;
      }
    }
    else {
      rlen += r;
      break;
    }
  }
  while (SSL_pending (sockinfo->ssl_conn));

  if (r == 0) {
    return OSIP_UNDEFINED_ERROR;
  }
  else if (r < 0) {
    return OSIP_UNDEFINED_ERROR;
  }
  else {
    int consumed;
    int err=OSIP_SUCCESS;

    if (SSL_pending (sockinfo->ssl_conn))
      err = -999;

    sockinfo->tcp_max_timeout=0;
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "socket %s:%i: read %d bytes\n", sockinfo->remote_ip, sockinfo->remote_port, r));
    sockinfo->buflen += rlen;
    consumed = handle_messages (excontext, sockinfo);
    if (consumed != 0) {
      if (sockinfo->buflen > consumed) {
        memmove (sockinfo->buf, sockinfo->buf + consumed, sockinfo->buflen - consumed);
        sockinfo->buflen -= consumed;
      }
      else {
        sockinfo->buflen = 0;
      }
    }

    return err; /* if -999 is returned, internal buffer of SSL still contains some data */
  }
}

static int
tls_tl_read_message (struct eXosip_t *excontext, fd_set * osip_fdset, fd_set * osip_wrset)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;
  int pos = 0;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  if (FD_ISSET (reserved->tls_socket, osip_fdset)) {
    /* accept incoming connection */
    char src6host[NI_MAXHOST];
    int recvport = 0;
    struct sockaddr_storage sa;
    int sock;
    int i;

    socklen_t slen;

    SSL *ssl = NULL;
    BIO *sbio;


    if (excontext->eXtl_transport.proto_family == AF_INET)
      slen = sizeof (struct sockaddr_in);
    else
      slen = sizeof (struct sockaddr_in6);

    for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
      if (reserved->socket_tab[pos].socket <= 0)
        break;
    }
    if (pos == EXOSIP_MAX_SOCKETS) {
      /* delete an old one! */
      pos = 0;
      if (reserved->socket_tab[pos].socket > 0) {
        _tls_tl_close_sockinfo (&reserved->socket_tab[pos]);
      }
      memset (&reserved->socket_tab[pos], 0, sizeof (struct _tls_stream));
    }

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO3, NULL, "creating TLS socket at index: %i\n", pos));

    sock = (int)accept (reserved->tls_socket, (struct sockaddr *) &sa, (socklen_t*)&slen);
    if (sock < 0) {
#if defined(EBADF)
      int status = ex_errno;
#endif
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Error accepting TLS socket\n"));
#if defined(EBADF)
      if (status == EBADF) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Error accepting TLS socket: EBADF\n"));
        memset (&reserved->ai_addr, 0, sizeof (struct sockaddr_storage));
        if (reserved->tls_socket > 0) {
          _eXosip_closesocket (reserved->tls_socket);
          for (i = 0; i < EXOSIP_MAX_SOCKETS; i++) {
            if (reserved->socket_tab[i].socket > 0 && reserved->socket_tab[i].is_server > 0)
              _tls_tl_close_sockinfo (&reserved->socket_tab[i]);
          }
        }
        tls_tl_open (excontext);
      }
#endif
    }
    else {
      if (reserved->server_ctx == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "TLS connection rejected\n"));
        _eXosip_closesocket (sock);
        return -1;
      }

      if (!SSL_CTX_check_private_key (reserved->server_ctx)) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "SSL CTX private key check error\n"));
      }

      ssl = SSL_new (reserved->server_ctx);
      if (ssl == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Cannot create ssl connection context\n"));
        return -1;
      }

      if (!SSL_check_private_key (ssl)) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "SSL private key check error\n"));
      }

      sbio = BIO_new_socket (sock, BIO_NOCLOSE);
      if (sbio == NULL) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "BIO_new_socket error\n"));
      }

      SSL_set_bio (ssl, sbio, sbio);    /* cannot fail */

      i = SSL_accept (ssl);
      if (i <= 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "SSL_accept error: %s\n", ERR_error_string (ERR_get_error (), NULL)));
        i = SSL_get_error (ssl, i);
        print_ssl_error (i);


        SSL_shutdown (ssl);
        _eXosip_closesocket (sock);
        SSL_free (ssl);
        return -1;
      }

      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "New TLS connection accepted\n"));

      reserved->socket_tab[pos].socket = sock;
      reserved->socket_tab[pos].is_server = 1;
      reserved->socket_tab[pos].ssl_conn = ssl;
      reserved->socket_tab[pos].ssl_state = 2;

      {
        int valopt = 1;

        setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (void *) &valopt, sizeof (valopt));
      }

      memset (src6host, 0, NI_MAXHOST);
      recvport = _eXosip_getport((struct sockaddr *) &sa, slen);
      _eXosip_getnameinfo((struct sockaddr *) &sa, slen, src6host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

      _eXosip_transport_set_dscp (excontext, excontext->eXtl_transport.proto_family, sock);

      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Message received from: %s:%i\n", src6host, recvport));
      osip_strncpy (reserved->socket_tab[pos].remote_ip, src6host, sizeof (reserved->socket_tab[pos].remote_ip) - 1);
      reserved->socket_tab[pos].remote_port = recvport;
    }
  }

  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    if (reserved->socket_tab[pos].socket > 0) {
      if (FD_ISSET (reserved->socket_tab[pos].socket, osip_fdset)) {
        int err = -999;
        int max = 5;
        while (err == -999 && max>0) {
          err = _tls_tl_recv (excontext, &reserved->socket_tab[pos]);
          max--;
        }
      }
    }
  }

  return OSIP_SUCCESS;
}


static int
_tls_tl_find_socket (struct eXosip_t *excontext, char *host, int port)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;
  int pos;

  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    if (reserved->socket_tab[pos].socket != 0) {
      if (0 == osip_strcasecmp (reserved->socket_tab[pos].remote_ip, host)
          && port == reserved->socket_tab[pos].remote_port)
        return pos;
    }
  }
  return -1;
}


static int
_tls_tl_connect_socket (struct eXosip_t *excontext, char *host, int port, int retry)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;
  int pos;
  int res;
  struct addrinfo *addrinfo = NULL;
  struct addrinfo *curinfo;
  int sock = -1;
  int ssl_state = 0;
  struct sockaddr selected_ai_addr;
  socklen_t selected_ai_addrlen;

  char src6host[NI_MAXHOST];

  memset (src6host, 0, sizeof (src6host));

  selected_ai_addrlen = 0;
  memset (&selected_ai_addr, 0, sizeof (struct sockaddr));

  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
    if (reserved->socket_tab[pos].socket == 0) {
      break;
    }
  }

  if (pos == EXOSIP_MAX_SOCKETS)
    return -1;

  res = _eXosip_get_addrinfo (excontext, &addrinfo, host, port, IPPROTO_TCP);
  if (res)
    return -1;

  for (curinfo = addrinfo; curinfo; curinfo = curinfo->ai_next) {
    int i;

    if (curinfo->ai_protocol && curinfo->ai_protocol != IPPROTO_TCP)
      continue;

    res = getnameinfo ((struct sockaddr *) curinfo->ai_addr, (socklen_t)curinfo->ai_addrlen, src6host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
    if (res != 0)
      continue;

    i = _tls_tl_find_socket (excontext, src6host, port);
    if (i >= 0) {
      _eXosip_freeaddrinfo (addrinfo);
      return i;
    }
  }

  if (retry>0)
    return -1;

  for (curinfo = addrinfo; curinfo; curinfo = curinfo->ai_next) {
    if (curinfo->ai_protocol && curinfo->ai_protocol != IPPROTO_TCP) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: Skipping protocol %d\n", curinfo->ai_protocol));
      continue;
    }

    res = getnameinfo ((struct sockaddr *) curinfo->ai_addr, (socklen_t)curinfo->ai_addrlen, src6host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

    if (res == 0) {
      int i = _tls_tl_find_socket (excontext, src6host, port);

      if (i >= 0) {
        _eXosip_freeaddrinfo (addrinfo);
        return i;
      }
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "New binding with %s\n", src6host));
    }

    sock = (int) socket (curinfo->ai_family, curinfo->ai_socktype, curinfo->ai_protocol);
    if (sock < 0) {
#if defined(OSIP_MONOTHREAD) || defined(_WIN32_WCE)
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: Cannot create socket!\n"));
#else
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: Cannot create socket %s!\n", strerror (ex_errno)));
#endif
      continue;
    }

    if (curinfo->ai_family == AF_INET6) {
#ifdef IPV6_V6ONLY
      if (setsockopt_ipv6only (sock)) {
        _eXosip_closesocket (sock);
        sock = -1;
#if defined(OSIP_MONOTHREAD) || defined(_WIN32_WCE)
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: Cannot set socket option!\n"));
#else
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: Cannot set socket option %s!\n", strerror (ex_errno)));
#endif
        continue;
      }
#endif /* IPV6_V6ONLY */
    }

    if (reserved->ai_addr_len>0)
    {
      if (excontext->reuse_tcp_port>0) {
        struct sockaddr_storage ai_addr;
        int valopt = 1;

        setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, (void *) &valopt, sizeof (valopt));

        memcpy(&ai_addr, &reserved->ai_addr, reserved->ai_addr_len);
        if (ai_addr.ss_family == AF_INET)
          ((struct sockaddr_in *) &ai_addr)->sin_port = htons(excontext->eXtl_transport.proto_local_port);
        else
          ((struct sockaddr_in6 *) &ai_addr)->sin6_port = htons(excontext->eXtl_transport.proto_local_port);
        res = bind (sock, (const struct sockaddr *)&ai_addr, reserved->ai_addr_len);
        if (res < 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "Cannot bind socket node:%s family:%d %s\n", excontext->eXtl_transport.proto_ifs, ai_addr.ss_family, strerror (ex_errno)));
        }
      } else if (excontext->oc_local_address[0]=='\0') {
        struct sockaddr_storage ai_addr;
        int count=0;
        memcpy(&ai_addr, &reserved->ai_addr, reserved->ai_addr_len);
        while (count<100) {
          if (excontext->oc_local_port_range[0]<1024) {
            if (ai_addr.ss_family == AF_INET)
              ((struct sockaddr_in *) &ai_addr)->sin_port = htons(0);
            else
              ((struct sockaddr_in6 *) &ai_addr)->sin6_port = htons(0);
          } else {
            if (excontext->oc_local_port_current==0)
              excontext->oc_local_port_current = excontext->oc_local_port_range[0];
            /* reset value */
            if (excontext->oc_local_port_current>=excontext->oc_local_port_range[1])
              excontext->oc_local_port_current = excontext->oc_local_port_range[0];

            if (ai_addr.ss_family == AF_INET)
              ((struct sockaddr_in *) &ai_addr)->sin_port = htons(excontext->oc_local_port_current);
            else
              ((struct sockaddr_in6 *) &ai_addr)->sin6_port = htons(excontext->oc_local_port_current);

          }
          res = bind (sock, (const struct sockaddr *)&ai_addr, reserved->ai_addr_len);
          if (res < 0) {
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "Cannot bind socket node:%s family:%d (port=%i) %s\n", excontext->eXtl_transport.proto_ifs, ai_addr.ss_family, excontext->oc_local_port_current, strerror (ex_errno)));
            count++;
            if (excontext->oc_local_port_range[0]>=1024)
              excontext->oc_local_port_current++;
            continue;
          }
          if (excontext->oc_local_port_range[0]>=1024)
            excontext->oc_local_port_current++;
          break;
        }
      } else {
        int count=0;

        if (excontext->oc_local_port_range[0]<1024)
          excontext->oc_local_port_range[0]=0;
        while (count<100) {
          struct addrinfo *oc_addrinfo = NULL;
          struct addrinfo *oc_curinfo;
          if (excontext->oc_local_port_current==0)
            excontext->oc_local_port_current = excontext->oc_local_port_range[0];
          if (excontext->oc_local_port_current>=excontext->oc_local_port_range[1])
            excontext->oc_local_port_current = excontext->oc_local_port_range[0];

          _eXosip_get_addrinfo(excontext, &oc_addrinfo, excontext->oc_local_address, excontext->oc_local_port_current, IPPROTO_TCP);
            
          for (oc_curinfo = oc_addrinfo; oc_curinfo; oc_curinfo = oc_curinfo->ai_next) {
            if  (oc_curinfo->ai_protocol && oc_curinfo->ai_protocol != IPPROTO_TCP) {
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Skipping protocol %d\n", oc_curinfo->ai_protocol));
              continue;
            }
            break;
          }
          if (oc_curinfo==NULL) {
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "not able to find any address to bind\n"));
            _eXosip_freeaddrinfo (oc_addrinfo);
            break;
          }
          res = bind (sock, (const struct sockaddr *)oc_curinfo->ai_addr, (socklen_t)oc_curinfo->ai_addrlen);
          if (res < 0) {
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "Cannot bind socket node:%s family:%d (port=%i) %s\n", excontext->oc_local_address, curinfo->ai_addr->sa_family, excontext->oc_local_port_current, strerror (ex_errno)));
            count++;
            if (excontext->oc_local_port_range[0]!=0)
              excontext->oc_local_port_current++;
            continue;
          }
          _eXosip_freeaddrinfo (oc_addrinfo);
          if (excontext->oc_local_port_range[0]!=0)
            excontext->oc_local_port_current++;
          break;
        }
      }
    }


#if defined(_WIN32_WCE) || defined(WIN32)
    {
      unsigned long nonBlock = 1;
      int val;

      ioctlsocket (sock, FIONBIO, &nonBlock);

      val = 1;
      if (setsockopt (sock, SOL_SOCKET, SO_KEEPALIVE, (char *) &val, sizeof (val)) == -1) {
        _eXosip_closesocket (sock);
        sock = -1;
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot get socket flag!\n"));
        continue;
      }
    }
#ifdef HAVE_MSTCPIP_H
    {
      DWORD err = 0L;
      DWORD dwBytes = 0L;
      struct tcp_keepalive kalive = { 0 };
      struct tcp_keepalive kaliveOut = { 0 };
      kalive.onoff = 1;
      kalive.keepalivetime = 30000;     /* Keep Alive in 5.5 sec. */
      kalive.keepaliveinterval = 3000;  /* Resend if No-Reply */
      err = WSAIoctl (sock, SIO_KEEPALIVE_VALS, &kalive, sizeof (kalive), &kaliveOut, sizeof (kaliveOut), &dwBytes, NULL, NULL);
      if (err != 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_WARNING, NULL, "Cannot set keepalive interval!\n"));
      }
    }
#endif
#else
    {
      int val;

      val = fcntl (sock, F_GETFL);
      if (val < 0) {
        _eXosip_closesocket (sock);
        sock = -1;
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot get socket flag!\n"));
        continue;
      }
      val |= O_NONBLOCK;
      if (fcntl (sock, F_SETFL, val) < 0) {
        _eXosip_closesocket (sock);
        sock = -1;
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot set socket flag!\n"));
        continue;
      }
#if 0
      val = 1;
      if (setsockopt (sock, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof (val)) == -1) {
      }
      val = 30;                 /* 30 sec before starting probes */
      setsockopt (sock, SOL_TCP, TCP_KEEPIDLE, &val, sizeof (val));
      val = 2;                  /* 2 probes max */
      setsockopt (sock, SOL_TCP, TCP_KEEPCNT, &val, sizeof (val));
      val = 10;                 /* 10 seconds between each probe */
      setsockopt (sock, SOL_TCP, TCP_KEEPINTVL, &val, sizeof (val));
#endif
#if SO_NOSIGPIPE
      val = 1;
      setsockopt (sock, SOL_SOCKET, SO_NOSIGPIPE, (void *) &val, sizeof (int));
#endif

#if TCP_NODELAY
      val = 1;
      if (setsockopt (sock, IPPROTO_TCP, TCP_NODELAY, (char *) &val, sizeof (int)) != 0) {
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot set socket flag (TCP_NODELAY)\n"));
      }
#endif
    }
#endif

    _eXosip_transport_set_dscp (excontext, excontext->eXtl_transport.proto_family, sock);

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "eXosip: socket node:%s , socket %d, family:%d set to non blocking mode\n", host, sock, curinfo->ai_family));
    res = connect (sock, curinfo->ai_addr, (socklen_t)curinfo->ai_addrlen);
    if (res < 0) {
#ifdef WIN32
      if (ex_errno != WSAEWOULDBLOCK) {
#else
      if (ex_errno != EINPROGRESS) {
#endif
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Cannot connect socket node:%s family:%d %s[%d]\n", host, curinfo->ai_family, strerror (ex_errno), ex_errno));

        _eXosip_closesocket (sock);
        sock = -1;
        continue;
      }
      else {
        res = _tls_tl_is_connected (sock);
        if (res > 0) {
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "socket node:%s, socket %d [pos=%d], family:%d, in progress\n", host, sock, pos, curinfo->ai_family));
          selected_ai_addrlen = (socklen_t)curinfo->ai_addrlen;
          memcpy (&selected_ai_addr, curinfo->ai_addr, sizeof (struct sockaddr));
          break;
        }
        else if (res == 0) {
#ifdef MULTITASKING_ENABLED
          reserved->socket_tab[pos].readStream = NULL;
          reserved->socket_tab[pos].writeStream = NULL;
          CFStreamCreatePairWithSocket (kCFAllocatorDefault, sock, &reserved->socket_tab[pos].readStream, &reserved->socket_tab[pos].writeStream);
          if (reserved->socket_tab[pos].readStream != NULL)
            CFReadStreamSetProperty (reserved->socket_tab[pos].readStream, kCFStreamNetworkServiceType, kCFStreamNetworkServiceTypeVoIP);
          if (reserved->socket_tab[pos].writeStream != NULL)
            CFWriteStreamSetProperty (reserved->socket_tab[pos].writeStream, kCFStreamNetworkServiceType, kCFStreamNetworkServiceTypeVoIP);
          if (CFReadStreamOpen (reserved->socket_tab[pos].readStream)) {
            OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "CFReadStreamOpen Succeeded!\n"));
          }

          CFWriteStreamOpen (reserved->socket_tab[pos].writeStream);
#endif
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "socket node:%s , socket %d [pos=%d], family:%d, connected\n", host, sock, pos, curinfo->ai_family));
          selected_ai_addrlen = 0;
          memcpy (&selected_ai_addr, curinfo->ai_addr, sizeof (struct sockaddr));
          ssl_state = 1;
          break;
        }
        else {
          _eXosip_closesocket (sock);
          sock = -1;
          continue;
        }
      }
    }

    break;
  }

  _eXosip_freeaddrinfo (addrinfo);

  if (sock > 0) {
    reserved->socket_tab[pos].socket = sock;

    reserved->socket_tab[pos].ai_addrlen = selected_ai_addrlen;
    memset (&reserved->socket_tab[pos].ai_addr, 0, sizeof (struct sockaddr));
    if (selected_ai_addrlen > 0)
      memcpy (&reserved->socket_tab[pos].ai_addr, &selected_ai_addr, selected_ai_addrlen);

    if (src6host[0] == '\0')
      osip_strncpy (reserved->socket_tab[pos].remote_ip, host, sizeof (reserved->socket_tab[pos].remote_ip) - 1);
    else
      osip_strncpy (reserved->socket_tab[pos].remote_ip, src6host, sizeof (reserved->socket_tab[pos].remote_ip) - 1);

    reserved->socket_tab[pos].remote_port = port;
    reserved->socket_tab[pos].ssl_conn = NULL;
    reserved->socket_tab[pos].ssl_state = ssl_state;
    reserved->socket_tab[pos].ssl_ctx = NULL;

    osip_strncpy (reserved->socket_tab[pos].sni_servernameindication, host, sizeof (reserved->socket_tab[pos].sni_servernameindication) - 1);
    
    {
      struct sockaddr_storage local_ai_addr;
      socklen_t selected_ai_addrlen;

      memset (&local_ai_addr, 0, sizeof (struct sockaddr_storage));
      selected_ai_addrlen = sizeof (struct sockaddr_storage);
      res = getsockname (sock, (struct sockaddr *) &local_ai_addr, &selected_ai_addrlen);
      if (res == 0) {
        if (local_ai_addr.ss_family == AF_INET)
          reserved->socket_tab[pos].ephemeral_port = ntohs (((struct sockaddr_in *) &local_ai_addr)->sin_port);
        else
          reserved->socket_tab[pos].ephemeral_port = ntohs (((struct sockaddr_in6 *) &local_ai_addr)->sin6_port);
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "Outgoing socket created on port %i!\n", reserved->socket_tab[pos].ephemeral_port));
      }
    }

    reserved->socket_tab[pos].tcp_inprogress_max_timeout = osip_getsystemtime (NULL) + 32;

    if (reserved->socket_tab[pos].ssl_state == 1) {     /* TCP connected but not TLS connected */
      res = _tls_tl_ssl_connect_socket (excontext, &reserved->socket_tab[pos]);
      if (res < 0) {
        _tls_tl_close_sockinfo (&reserved->socket_tab[pos]);
        return -1;
      }
    }
    return pos;
  }

  return -1;
}

static int
_tls_tl_update_contact (struct eXosip_t *excontext, osip_message_t * req, char *natted_ip, int natted_port)
{
  if (req->application_data != (void*) 0x1)
    return OSIP_SUCCESS;

  if ((natted_ip != NULL && natted_ip[0] != '\0') || natted_port > 0) {
    osip_list_iterator_t it;
    osip_contact_t* co = (osip_contact_t *)osip_list_get_first(&req->contacts, &it);
    while (co != NULL) {
      if (co != NULL && co->url != NULL && co->url->host != NULL) {
        if (natted_port > 0) {
          if (co->url->port)
            osip_free (co->url->port);
          co->url->port = (char *) osip_malloc (10);
          snprintf (co->url->port, 9, "%i", natted_port);
          osip_message_force_update (req);
        }
        if (natted_ip != NULL && natted_ip[0] != '\0') {
          osip_free (co->url->host);
          co->url->host = osip_strdup (natted_ip);
          osip_message_force_update (req);
        }
      }
      co = (osip_contact_t *)osip_list_get_next(&it);
    }
  }

  return OSIP_SUCCESS;
}

static int
tls_tl_send_message (struct eXosip_t *excontext, osip_transaction_t * tr, osip_message_t * sip, char *host, int port, int out_socket)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;
  size_t length = 0;
  char *message;
  char *ptr;
  int i;

  int pos;
  osip_naptr_t *naptr_record = NULL;

  SSL *ssl = NULL;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  if (host == NULL) {
    host = sip->req_uri->host;
    if (sip->req_uri->port != NULL)
      port = osip_atoi (sip->req_uri->port);
    else
      port = 5061;
  }

  if (port == 5060)
    port = 5061;

  i = -1;
  if (tr == NULL) {
    _eXosip_srv_lookup (excontext, sip, &naptr_record);

    if (naptr_record != NULL) {
      eXosip_dnsutils_dns_process (naptr_record, 1);
      if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
        eXosip_dnsutils_dns_process (naptr_record, 1);
    }

    if (naptr_record != NULL && naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVDONE) {
      /* 4: check if we have the one we want... */
      if (naptr_record->siptls_record.name[0] != '\0' && naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].srv[0] != '\0') {
        /* always choose the first here.
           if a network error occur, remove first entry and
           replace with next entries.
         */
        osip_srv_entry_t *srv;

        srv = &naptr_record->siptls_record.srventry[naptr_record->siptls_record.index];
        if (srv->ipaddress[0]) {
          host = srv->ipaddress;
          port = srv->port;
        }
        else {
          host = srv->srv;
          port = srv->port;
        }
      }
    }

    if (naptr_record != NULL && naptr_record->keep_in_cache == 0)
      osip_free (naptr_record);
    naptr_record = NULL;
  }
  else {
    naptr_record = tr->naptr_record;
  }

  if (naptr_record != NULL) {
    /* 1: make sure there is no pending DNS */
    eXosip_dnsutils_dns_process (naptr_record, 0);
    if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE || naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS)
      eXosip_dnsutils_dns_process (naptr_record, 0);

    if (naptr_record->naptr_state == OSIP_NAPTR_STATE_UNKNOWN) {
      /* fallback to DNS A */
      if (naptr_record->keep_in_cache == 0)
        osip_free (naptr_record);
      naptr_record = NULL;
      if (tr != NULL)
        tr->naptr_record = NULL;
      /* must never happen? */
    }
    else if (naptr_record->naptr_state == OSIP_NAPTR_STATE_INPROGRESS) {
      /* 2: keep waiting (naptr answer not received) */
      return OSIP_SUCCESS + 1;
    }
    else if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NAPTRDONE) {
      /* 3: keep waiting (naptr answer received/no srv answer received) */
      return OSIP_SUCCESS + 1;
    }
    else if (naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVINPROGRESS) {
      /* 3: keep waiting (naptr answer received/no srv answer received) */
      return OSIP_SUCCESS + 1;
    }
    else if (naptr_record->naptr_state == OSIP_NAPTR_STATE_SRVDONE) {
      /* 4: check if we have the one we want... */
      if (naptr_record->siptls_record.name[0] != '\0' && naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].srv[0] != '\0') {
        /* always choose the first here.
           if a network error occur, remove first entry and
           replace with next entries.
         */
        osip_srv_entry_t *srv;

        if (MSG_IS_REGISTER (sip) || MSG_IS_OPTIONS (sip)) {
          /* activate the failover capability: for no answer OR 503 */
          if (naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].srv_is_broken.tv_sec>0) {
            naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].srv_is_broken.tv_sec=0;
            naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].srv_is_broken.tv_usec=0;
            if (eXosip_dnsutils_rotate_srv (&naptr_record->siptls_record) > 0) {
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                      "Doing TLS failover: %s:%i->%s:%i\n", host, port, naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].srv, naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].port));
            }
          }
        }

        srv = &naptr_record->siptls_record.srventry[naptr_record->siptls_record.index];
        if (srv->ipaddress[0]) {
          host = srv->ipaddress;
          port = srv->port;
        }
        else {
          host = srv->srv;
          port = srv->port;
        }
      }
    }
    else if (naptr_record->naptr_state == OSIP_NAPTR_STATE_NOTSUPPORTED || naptr_record->naptr_state == OSIP_NAPTR_STATE_RETRYLATER) {
      /* 5: fallback to DNS A */
      if (naptr_record->keep_in_cache == 0)
        osip_free (naptr_record);
      naptr_record = NULL;
      if (tr != NULL)
        tr->naptr_record = NULL;
    }
  }

  /* verify all current connections */
  _tls_tl_check_connected (excontext);

  if (out_socket > 0) {
    for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {
      if (reserved->socket_tab[pos].socket != 0) {
        if (reserved->socket_tab[pos].socket == out_socket) {
          out_socket = reserved->socket_tab[pos].socket;
          ssl = reserved->socket_tab[pos].ssl_conn;
          OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "reusing REQUEST connection (to dest=%s:%i)\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port));
          break;
        }
      }
    }
    if (pos == EXOSIP_MAX_SOCKETS)
      out_socket = 0;

    if (out_socket > 0) {
      int pos2;

      /* If we have SEVERAL sockets to same destination with different port
         number, we search for the one with "SAME port" number.
         The specification is not clear about re-using the existing transaction
         in that use-case...
         Such test, will help mainly with server having 2 sockets: one for
         incoming transaction and one for outgoing transaction?
       */
      pos2 = _tls_tl_find_socket (excontext, host, port);
      if (pos2 >= 0) {
        out_socket = reserved->socket_tab[pos2].socket;
        pos = pos2;
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "reusing connection --with exact port--: (to dest=%s:%i)\n", reserved->socket_tab[pos].remote_ip, reserved->socket_tab[pos].remote_port));
      }
    }
  }

  /* Step 1: find existing socket to send message */
  if (out_socket <= 0) {
    pos = _tls_tl_find_socket (excontext, host, port);

    /* Step 2: create new socket with host:port */
    if (pos < 0) {
      if (tr==NULL) {
        pos = _tls_tl_connect_socket (excontext, host, port, 0);
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                "message out of transaction: trying to send to %s:%i\n", host, port));
        if (pos<0) {
          return -1;
        }
      } else {
        pos = _tls_tl_connect_socket (excontext, host, port, 0);
        if (pos<0) {
          if (naptr_record != NULL && MSG_IS_REGISTER (sip)) {
            if (eXosip_dnsutils_rotate_srv (&naptr_record->siptls_record) > 0) {
              /* reg_call_id is not set! */
              _eXosip_mark_registration_expired (excontext, sip->call_id->number);
              if (pos >= 0) _tls_tl_close_sockinfo (&reserved->socket_tab[pos]);
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                      "Doing TLS failover: %s:%i->%s:%i\n", host, port, naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].srv, naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].port));
            }
          }
          return -1;
        }
      }
    }
    if (pos >= 0) {

      if (MSG_IS_REGISTER (sip)) {
        /* this value is saved: when a connection breaks, we will ask to retry the registration */
        snprintf(reserved->socket_tab[pos].reg_call_id, sizeof(reserved->socket_tab[pos].reg_call_id), "%s", sip->call_id->number);
      }
      out_socket = reserved->socket_tab[pos].socket;
      ssl = reserved->socket_tab[pos].ssl_conn;
    }
  }

  if (out_socket <= 0) {
    if (naptr_record != NULL && MSG_IS_REGISTER (sip)) {
      if (eXosip_dnsutils_rotate_srv (&naptr_record->siptls_record) > 0) {
        /* reg_call_id is not set! */
        _eXosip_mark_registration_expired (excontext, sip->call_id->number);
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                "Doing TLS failover: %s:%i->%s:%i\n", host, port, naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].srv, naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].port));
      }
    }
    return -1;
  }

  if (reserved->socket_tab[pos].ssl_state == 0) {
    i = _tls_tl_is_connected (out_socket);
    if (i > 0) {
      time_t now;
      if (tr!=NULL) {
        now = osip_getsystemtime (NULL);
        OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "socket node:%s, socket %d [pos=%d], in progress\n", host, out_socket, pos));
        if (tr != NULL && now - tr->birth_time > 10) {
          if (naptr_record != NULL && (MSG_IS_REGISTER (sip) || MSG_IS_OPTIONS (sip))) {
            if (eXosip_dnsutils_rotate_srv (&naptr_record->siptls_record) > 0) {
              _eXosip_mark_registration_expired (excontext, reserved->socket_tab[pos].reg_call_id);
              if (pos >= 0) _tls_tl_close_sockinfo (&reserved->socket_tab[pos]);
              OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL,
                                      "Doing TLS failover: %s:%i->%s:%i\n", host, port, naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].srv, naptr_record->siptls_record.srventry[naptr_record->siptls_record.index].port));
              return -1;
            }
          }
          return -1;
        }
      }
      return 1;
    }
    else if (i == 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "socket node:%s , socket %d [pos=%d], connected\n", host, out_socket, pos));
      reserved->socket_tab[pos].ssl_state = 1;
      reserved->socket_tab[pos].ai_addrlen = 0;
    }
    else {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "socket node:%s, socket %d [pos=%d], socket error\n", host, out_socket, pos));
      _tls_tl_close_sockinfo (&reserved->socket_tab[pos]);
      return -1;
    }
  }

  if (reserved->socket_tab[pos].ssl_state == 1) {       /* TCP connected but not TLS connected */
    i = _tls_tl_ssl_connect_socket (excontext, &reserved->socket_tab[pos]);
    if (i < 0) {
      _tls_tl_close_sockinfo (&reserved->socket_tab[pos]);
      return -1;
    }
    else if (i > 0) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "socket node:%s, socket %d [pos=%d], connected (ssl in progress)\n", host, out_socket, pos));
      return 1;
    }
    ssl = reserved->socket_tab[pos].ssl_conn;
  }

  if (ssl == NULL) {
    return -1;
  }

#ifdef MULTITASKING_ENABLED
  if (reserved->socket_tab[pos].readStream == NULL) {
    reserved->socket_tab[pos].readStream = NULL;
    reserved->socket_tab[pos].writeStream = NULL;
    CFStreamCreatePairWithSocket (kCFAllocatorDefault, reserved->socket_tab[pos].socket, &reserved->socket_tab[pos].readStream, &reserved->socket_tab[pos].writeStream);
    if (reserved->socket_tab[pos].readStream != NULL)
      CFReadStreamSetProperty (reserved->socket_tab[pos].readStream, kCFStreamNetworkServiceType, kCFStreamNetworkServiceTypeVoIP);
    if (reserved->socket_tab[pos].writeStream != NULL)
      CFWriteStreamSetProperty (reserved->socket_tab[pos].writeStream, kCFStreamNetworkServiceType, kCFStreamNetworkServiceTypeVoIP);
    if (CFReadStreamOpen (reserved->socket_tab[pos].readStream)) {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO2, NULL, "CFReadStreamOpen Succeeded!\n"));
    }

    CFWriteStreamOpen (reserved->socket_tab[pos].writeStream);
  }
#endif

  _eXosip_request_viamanager(excontext, tr, sip, IPPROTO_TCP, NULL, reserved->socket_tab[pos].ephemeral_port, reserved->socket_tab[pos].socket, host);
  if (excontext->use_ephemeral_port==1)
    _eXosip_message_contactmanager(excontext, tr, sip, IPPROTO_TCP, NULL, reserved->socket_tab[pos].ephemeral_port, reserved->socket_tab[pos].socket, host);
  else
    _eXosip_message_contactmanager(excontext, tr, sip, IPPROTO_TCP, NULL, excontext->eXtl_transport.proto_local_port, reserved->socket_tab[pos].socket, host);
  if (excontext->tls_firewall_ip[0] != '\0' || excontext->auto_masquerade_contact > 0)
    _tls_tl_update_contact (excontext, sip, reserved->socket_tab[pos].natted_ip, reserved->socket_tab[pos].natted_port);

  /* remove preloaded route if there is no tag in the To header
   */
  {
    osip_route_t *route = NULL;
    osip_generic_param_t *tag = NULL;

    if (excontext->remove_prerouteset>0) {
      osip_message_get_route (sip, 0, &route);
      osip_to_get_tag (sip->to, &tag);
      if (tag == NULL && route != NULL && route->url != NULL) {
        osip_list_remove (&sip->routes, 0);
        osip_message_force_update(sip);
      }
    }
    i = osip_message_to_str (sip, &message, &length);
    if (tag == NULL && route != NULL && route->url != NULL) {
      osip_list_add (&sip->routes, route, 0);
    }
  }

  if (i != 0 || length <= 0) {
    return -1;
  }

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO1, NULL, "Message sent: (to dest=%s:%i) \n%s\n", host, port, message));

  if (pos >= 0 && excontext->enable_dns_cache == 1 && osip_strcasecmp (host, reserved->socket_tab[pos].remote_ip) != 0 && MSG_IS_REQUEST (sip)) {
    if (MSG_IS_REGISTER (sip)) {
      struct eXosip_dns_cache entry;

      memset (&entry, 0, sizeof (struct eXosip_dns_cache));
      snprintf (entry.host, sizeof (entry.host), "%s", host);
      snprintf (entry.ip, sizeof (entry.ip), "%s", reserved->socket_tab[pos].remote_ip);
      eXosip_set_option (excontext, EXOSIP_OPT_ADD_DNS_CACHE, (void *) &entry);
    }
  }

  SSL_set_mode (ssl, SSL_MODE_AUTO_RETRY);

  ptr = message;
  while (length > 0) {
#if TARGET_OS_IPHONE            /* avoid ssl error on large message */
    int max = (length > 500) ? 500 : length;

    i = SSL_write (ssl, (const void *) ptr, (int) max);
#else
    i = SSL_write (ssl, (const void *) ptr, (int) length);
#endif
    if (i <= 0) {
      i = SSL_get_error (ssl, i);
      if (i == SSL_ERROR_WANT_READ || i == SSL_ERROR_WANT_WRITE)
        continue;
      print_ssl_error (i);

      osip_free (message);
      if (pos >= 0) _tls_tl_close_sockinfo (&reserved->socket_tab[pos]);
      return -1;
    }
    length = length - i;
    ptr += i;
  }

  osip_free (message);

  if (tr!=NULL && MSG_IS_REGISTER(sip) && pos>=0) {
    /* start a timeout to destroy connection if no answer */
    reserved->socket_tab[pos].tcp_max_timeout = osip_getsystemtime (NULL) + 32;
  }

  return OSIP_SUCCESS;
}

static int
tls_tl_keepalive (struct eXosip_t *excontext)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;
  char buf[5] = "\r\n\r\n";
  int pos;
  int i;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  if (reserved->tls_socket <= 0)
    return OSIP_UNDEFINED_ERROR;

  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {

    if (excontext->ka_interval > 0) {
      if (reserved->socket_tab[pos].socket > 0 && reserved->socket_tab[pos].ssl_state > 2) {
        SSL_set_mode (reserved->socket_tab[pos].ssl_conn, SSL_MODE_AUTO_RETRY);

        while (1) {
          i = SSL_write (reserved->socket_tab[pos].ssl_conn, (const void *) buf, 4);

          if (i <= 0) {
            i = SSL_get_error (reserved->socket_tab[pos].ssl_conn, i);
            if (i == SSL_ERROR_WANT_READ || i == SSL_ERROR_WANT_WRITE)
              continue;
            print_ssl_error (i);
          }
          break;
        }
      }
    }

  }
  return OSIP_SUCCESS;
}

static int
tls_tl_set_socket (struct eXosip_t *excontext, int socket)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  reserved->tls_socket = socket;

  return OSIP_SUCCESS;
}

static int
tls_tl_masquerade_contact (struct eXosip_t *excontext, const char *public_address, int port)
{
  if (public_address == NULL || public_address[0] == '\0') {
    memset (excontext->tls_firewall_ip, '\0', sizeof (excontext->tls_firewall_ip));
    memset (excontext->tls_firewall_port, '\0', sizeof (excontext->tls_firewall_port));
    return OSIP_SUCCESS;
  }
  snprintf (excontext->tls_firewall_ip, sizeof (excontext->tls_firewall_ip), "%s", public_address);
  if (port > 0) {
    snprintf (excontext->tls_firewall_port, sizeof (excontext->tls_firewall_port), "%i", port);
  }
  return OSIP_SUCCESS;
}

static int
tls_tl_get_masquerade_contact (struct eXosip_t *excontext, char *ip, int ip_size, char *port, int port_size)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;

  memset (ip, 0, ip_size);
  memset (port, 0, port_size);

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  if (excontext->tls_firewall_ip[0] != '\0')
    snprintf (ip, ip_size, "%s", excontext->tls_firewall_ip);

  if (excontext->tls_firewall_port[0] != '\0')
    snprintf (port, port_size, "%s", excontext->tls_firewall_port);
  return OSIP_SUCCESS;
}


static int
tls_tl_update_contact (struct eXosip_t *excontext, osip_message_t * req)
{
  req->application_data = (void*) 0x1; /* request for masquerading */
  return OSIP_SUCCESS;
}

static int
tls_tl_check_connection (struct eXosip_t *excontext)
{
  struct eXtltls *reserved = (struct eXtltls *) excontext->eXtltls_reserved;
  int pos;

  if (reserved == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "wrong state: create transport layer first\n"));
    return OSIP_WRONG_STATE;
  }

  if (reserved->tls_socket <= 0)
    return OSIP_UNDEFINED_ERROR;

  for (pos = 0; pos < EXOSIP_MAX_SOCKETS; pos++) {

    if (reserved->socket_tab[pos].socket > 0 && reserved->socket_tab[pos].ssl_state > 2)
      reserved->socket_tab[pos].tcp_inprogress_max_timeout=0; /* reset value */

    if (reserved->socket_tab[pos].socket > 0 && reserved->socket_tab[pos].ssl_state <= 2
      && reserved->socket_tab[pos].tcp_inprogress_max_timeout>0) {
      time_t now = osip_getsystemtime (NULL);
      if (now > reserved->socket_tab[pos].tcp_inprogress_max_timeout) {
        OSIP_TRACE (osip_trace
          (__FILE__, __LINE__, OSIP_INFO2, NULL, "tls_tl_check_connection socket is in progress since 32 seconds / close socket\n"));
        reserved->socket_tab[pos].tcp_inprogress_max_timeout=0;
        _eXosip_mark_registration_expired (excontext, reserved->socket_tab[pos].reg_call_id);
        _tls_tl_close_sockinfo (&reserved->socket_tab[pos]);
        continue;
      }
    }

    if (reserved->socket_tab[pos].socket > 0 && reserved->socket_tab[pos].ssl_state > 2
      && reserved->socket_tab[pos].tcp_max_timeout>0) {
      time_t now = osip_getsystemtime (NULL);
      if (now > reserved->socket_tab[pos].tcp_max_timeout) {
        OSIP_TRACE (osip_trace
          (__FILE__, __LINE__, OSIP_INFO2, NULL, "tls_tl_check_connection we expected a reply on established sockets / close socket\n"));
        reserved->socket_tab[pos].tcp_max_timeout=0;
        _eXosip_mark_registration_expired (excontext, reserved->socket_tab[pos].reg_call_id);
        _tls_tl_close_sockinfo (&reserved->socket_tab[pos]);
        continue;
      }
    }

  }
  return OSIP_SUCCESS;
}

static struct eXtl_protocol eXtl_tls = {
  1,
  5061,
  "TLS",
  "0.0.0.0",
  IPPROTO_TCP,
  AF_INET,
  0,
  0,
  0,

  &tls_tl_init,
  &tls_tl_free,
  &tls_tl_open,
  &tls_tl_set_fdset,
  &tls_tl_read_message,
  &tls_tl_send_message,
  &tls_tl_keepalive,
  &tls_tl_set_socket,
  &tls_tl_masquerade_contact,
  &tls_tl_get_masquerade_contact,
  &tls_tl_update_contact,
  &tls_tl_reset,
  &tls_tl_check_connection
};

void
eXosip_transport_tls_init (struct eXosip_t *excontext)
{
  memcpy (&excontext->eXtl_transport, &eXtl_tls, sizeof (struct eXtl_protocol));
}

#else

eXosip_tls_ctx_error
eXosip_tls_verify_certificate (struct eXosip_t *excontext, int
                               _tls_verify_client_certificate)
{
  return -1;                    /* NOT IMPLEMENTED */
}

eXosip_tls_ctx_error
eXosip_tls_use_server_certificate (struct eXosip_t * excontext, const char *local_certificate_cn)
{
  return -1;                    /* NOT IMPLEMENTED */
}

eXosip_tls_ctx_error
eXosip_tls_use_client_certificate (struct eXosip_t * excontext, const char *local_certificate_cn)
{
  return -1;                    /* NOT IMPLEMENTED */
}


eXosip_tls_ctx_error
eXosip_set_tls_ctx (struct eXosip_t * excontext, eXosip_tls_ctx_t * ctx)
{
  return -1;                    /* NOT IMPLEMENTED */
}

#endif
