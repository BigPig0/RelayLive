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
#include <eXosip2/eXosip.h>

#include <osip2/osip_mt.h>
#include <osip2/osip_condv.h>

/* #include <osip2/global.h> */
#include <osipparser2/osip_md5.h>
#include "milenage.h"

/* TAKEN from rcf2617.txt */

#define HASHLEN 16
typedef char HASH[HASHLEN];

#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN + 1];

#define IN
#define OUT

/* AKA */
#define MAX_HEADER_LEN  2049
#define KLEN 16
typedef u_char K[KLEN];

#define RANDLEN 16
typedef u_char RAND[RANDLEN];

#define AUTNLEN 16
typedef u_char AUTN[AUTNLEN];

#define AKLEN 6
typedef u_char AK[AKLEN];

#define AMFLEN 2
typedef u_char AMF[AMFLEN];

#define MACLEN 8
typedef u_char MAC[MACLEN];

#define CKLEN 16
typedef u_char CK[CKLEN];

#define IKLEN 16
typedef u_char IK[IKLEN];

#define SQNLEN 6
typedef u_char SQN[SQNLEN];

#define AUTSLEN 14
typedef char AUTS[AUTSLEN];

#define AUTS64LEN 21
typedef char AUTS64[AUTS64LEN];

#define RESLEN 8
typedef unsigned char RES[RESLEN];

#define RESHEXLEN 17
typedef char RESHEX[RESHEXLEN];
typedef char RESHEXAKA2[RESHEXLEN + IKLEN * 2 + CKLEN * 2];

AMF amf = "\0\0";
AMF amfstar = "\0\0";

/* end AKA */

/* Private functions */
void CvtHex (IN HASH Bin, OUT HASHHEX Hex);
static void DigestCalcHA1 (IN const char *pszAlg, IN const char *pszUserName, IN const char *pszRealm, IN const char *pszPassword, IN const char *pszNonce, IN const char *pszCNonce, OUT HASHHEX SessionKey);
static void DigestCalcResponse (IN HASHHEX HA1, IN const char *pszNonce,
                                IN const char *pszNonceCount, IN const char *pszCNonce, IN const char *pszQop, IN int Aka, IN const char *pszMethod, IN const char *pszDigestUri, IN HASHHEX HEntity, OUT HASHHEX Response);
static void DigestCalcResponseAka (IN const char *pszPassword, IN const char *pszNonce, IN const char *pszCNonce, IN const char *pszQop, IN const char *pszMethod, IN const char *pszDigestUri, IN int version, OUT HASHHEX Response);

void
CvtHex (IN HASH Bin, OUT HASHHEX Hex)
{
  unsigned short i;
  unsigned char j;

  for (i = 0; i < HASHLEN; i++) {
    j = (Bin[i] >> 4) & 0xf;
    if (j <= 9)
      Hex[i * 2] = (j + '0');
    else
      Hex[i * 2] = (j + 'a' - 10);
    j = Bin[i] & 0xf;
    if (j <= 9)
      Hex[i * 2 + 1] = (j + '0');
    else
      Hex[i * 2 + 1] = (j + 'a' - 10);
  };
  Hex[HASHHEXLEN] = '\0';
}

/* calculate H(A1) as per spec */
static void
DigestCalcHA1 (IN const char *pszAlg, IN const char *pszUserName, IN const char *pszRealm, IN const char *pszPassword, IN const char *pszNonce, IN const char *pszCNonce, OUT HASHHEX SessionKey)
{
  osip_MD5_CTX Md5Ctx;
  HASH HA1;

  osip_MD5Init (&Md5Ctx);
  osip_MD5Update (&Md5Ctx, (unsigned char *) pszUserName, (unsigned int) strlen (pszUserName));
  osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
  osip_MD5Update (&Md5Ctx, (unsigned char *) pszRealm, (unsigned int) strlen (pszRealm));
  osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
  osip_MD5Update (&Md5Ctx, (unsigned char *) pszPassword, (unsigned int) strlen (pszPassword));
  osip_MD5Final ((unsigned char *) HA1, &Md5Ctx);
  if ((pszAlg != NULL) && osip_strcasecmp (pszAlg, "md5-sess") == 0) {
    osip_MD5Init (&Md5Ctx);
    osip_MD5Update (&Md5Ctx, (unsigned char *) HA1, HASHLEN);
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonce, (unsigned int) strlen (pszNonce));
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszCNonce, (unsigned int) strlen (pszCNonce));
    osip_MD5Final ((unsigned char *) HA1, &Md5Ctx);
  }
  CvtHex (HA1, SessionKey);
}

/* calculate request-digest/response-digest as per HTTP Digest spec */
static void
DigestCalcResponse (IN HASHHEX HA1,     /* H(A1) */
                    IN const char *pszNonce,    /* nonce from server */
                    IN const char *pszNonceCount,       /* 8 hex digits */
                    IN const char *pszCNonce,   /* client nonce */
                    IN const char *pszQop,      /* qop-value: "", "auth", "auth-int" */
                    IN int Aka, /* Calculating AKAv1-MD5 response */
                    IN const char *pszMethod,   /* method from the request */
                    IN const char *pszDigestUri,        /* requested URL */
                    IN HASHHEX HEntity, /* H(entity body) if qop="auth-int" */
                    OUT HASHHEX Response
                    /* request-digest or response-digest */ )
{
  osip_MD5_CTX Md5Ctx;
  HASH HA2;
  HASH RespHash;
  HASHHEX HA2Hex;

  /* calculate H(A2) */
  osip_MD5Init (&Md5Ctx);
  osip_MD5Update (&Md5Ctx, (unsigned char *) pszMethod, (unsigned int) strlen (pszMethod));
  osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
  osip_MD5Update (&Md5Ctx, (unsigned char *) pszDigestUri, (unsigned int) strlen (pszDigestUri));

  if (pszQop == NULL) {
    goto auth_withoutqop;
  }
  else if (0 == osip_strcasecmp (pszQop, "auth-int")) {
    goto auth_withauth_int;
  }
  else if (0 == osip_strcasecmp (pszQop, "auth")) {
    goto auth_withauth;
  }

auth_withoutqop:
  osip_MD5Final ((unsigned char *) HA2, &Md5Ctx);
  CvtHex (HA2, HA2Hex);

  /* calculate response */
  osip_MD5Init (&Md5Ctx);
  osip_MD5Update (&Md5Ctx, (unsigned char *) HA1, HASHHEXLEN);
  osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
  osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonce, (unsigned int) strlen (pszNonce));
  osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);

  goto end;

auth_withauth_int:

  osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
  osip_MD5Update (&Md5Ctx, (unsigned char *) HEntity, HASHHEXLEN);

auth_withauth:
  osip_MD5Final ((unsigned char *) HA2, &Md5Ctx);
  CvtHex (HA2, HA2Hex);

  /* calculate response */
  osip_MD5Init (&Md5Ctx);
  osip_MD5Update (&Md5Ctx, (unsigned char *) HA1, HASHHEXLEN);
  osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
  osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonce, (unsigned int) strlen (pszNonce));
  osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
  if (Aka == 0) {
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszNonceCount, (unsigned int) strlen (pszNonceCount));
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszCNonce, (unsigned int) strlen (pszCNonce));
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
    osip_MD5Update (&Md5Ctx, (unsigned char *) pszQop, (unsigned int) strlen (pszQop));
    osip_MD5Update (&Md5Ctx, (unsigned char *) ":", 1);
  }
end:
  osip_MD5Update (&Md5Ctx, (unsigned char *) HA2Hex, HASHHEXLEN);
  osip_MD5Final ((unsigned char *) RespHash, &Md5Ctx);
  CvtHex (RespHash, Response);
}

/*"
ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";*/
static int
base64_val (char x)
{
  switch (x) {
  case '=':
    return -1;
  case 'A':
    return 0;
  case 'B':
    return 1;
  case 'C':
    return 2;
  case 'D':
    return 3;
  case 'E':
    return 4;
  case 'F':
    return 5;
  case 'G':
    return 6;
  case 'H':
    return 7;
  case 'I':
    return 8;
  case 'J':
    return 9;
  case 'K':
    return 10;
  case 'L':
    return 11;
  case 'M':
    return 12;
  case 'N':
    return 13;
  case 'O':
    return 14;
  case 'P':
    return 15;
  case 'Q':
    return 16;
  case 'R':
    return 17;
  case 'S':
    return 18;
  case 'T':
    return 19;
  case 'U':
    return 20;
  case 'V':
    return 21;
  case 'W':
    return 22;
  case 'X':
    return 23;
  case 'Y':
    return 24;
  case 'Z':
    return 25;
  case 'a':
    return 26;
  case 'b':
    return 27;
  case 'c':
    return 28;
  case 'd':
    return 29;
  case 'e':
    return 30;
  case 'f':
    return 31;
  case 'g':
    return 32;
  case 'h':
    return 33;
  case 'i':
    return 34;
  case 'j':
    return 35;
  case 'k':
    return 36;
  case 'l':
    return 37;
  case 'm':
    return 38;
  case 'n':
    return 39;
  case 'o':
    return 40;
  case 'p':
    return 41;
  case 'q':
    return 42;
  case 'r':
    return 43;
  case 's':
    return 44;
  case 't':
    return 45;
  case 'u':
    return 46;
  case 'v':
    return 47;
  case 'w':
    return 48;
  case 'x':
    return 49;
  case 'y':
    return 50;
  case 'z':
    return 51;
  case '0':
    return 52;
  case '1':
    return 53;
  case '2':
    return 54;
  case '3':
    return 55;
  case '4':
    return 56;
  case '5':
    return 57;
  case '6':
    return 58;
  case '7':
    return 59;
  case '8':
    return 60;
  case '9':
    return 61;
  case '+':
    return 62;
  case '/':
    return 63;
  }
  return OSIP_SUCCESS;
}


static char *
base64_decode_string (const char *buf, unsigned int len, int *newlen)
{
  unsigned int i, j;
  int x1, x2, x3, x4;
  char *out;

  out = (char *) osip_malloc ((len * 3 / 4) + 8);
  if (out == NULL)
    return NULL;
  for (i = 0, j = 0; i + 4 < len; i += 4) {
    x1 = base64_val (buf[i]);
    x2 = base64_val (buf[i + 1]);
    x3 = base64_val (buf[i + 2]);
    x4 = base64_val (buf[i + 3]);
    out[j++] = (x1 << 2) | ((x2 & 0x30) >> 4);
    out[j++] = ((x2 & 0x0F) << 4) | ((x3 & 0x3C) >> 2);
    out[j++] = ((x3 & 0x03) << 6) | (x4 & 0x3F);
  }
  if (i <= len) {
    x1 = base64_val (buf[i]);
    if (i + 1 < len)
      x2 = base64_val (buf[i + 1]);
    else
      x2 = -1;
    if (i + 2 < len)
      x3 = base64_val (buf[i + 2]);
    else
      x3 = -1;
    if (i + 3 < len)
      x4 = base64_val (buf[i + 3]);
    else
      x4 = -1;
    if (x2 != -1) {
      out[j++] = (x1 << 2) | ((x2 & 0x30) >> 4);
      if (x3 != -1) {
        out[j++] = ((x2 & 0x0F) << 4) | ((x3 & 0x3C) >> 2);
        if (x4 != -1) {
          out[j++] = ((x3 & 0x03) << 6) | (x4 & 0x3F);
        }
      }
    }

  }

  out[j++] = 0;
  *newlen = j - 1;
  return out;
}

#if 0
char base64[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char *
base64_encode_string (const char *buf, unsigned int len, int *newlen)
{
  int i, k;
  int triplets, rest;
  char *out, *ptr;

  triplets = len / 3;
  rest = len % 3;
  out = (char *) osip_malloc ((triplets * 4) + 8);
  if (out == NULL)
    return NULL;

  ptr = out;
  for (i = 0; i < triplets * 3; i += 3) {
    k = (((unsigned char) buf[i]) & 0xFC) >> 2;
    *ptr = base64[k];
    ptr++;

    k = (((unsigned char) buf[i]) & 0x03) << 4;
    k |= (((unsigned char) buf[i + 1]) & 0xF0) >> 4;
    *ptr = base64[k];
    ptr++;

    k = (((unsigned char) buf[i + 1]) & 0x0F) << 2;
    k |= (((unsigned char) buf[i + 2]) & 0xC0) >> 6;
    *ptr = base64[k];
    ptr++;

    k = (((unsigned char) buf[i + 2]) & 0x3F);
    *ptr = base64[k];
    ptr++;
  }
  i = triplets * 3;
  switch (rest) {
  case 0:
    break;
  case 1:
    k = (((unsigned char) buf[i]) & 0xFC) >> 2;
    *ptr = base64[k];
    ptr++;

    k = (((unsigned char) buf[i]) & 0x03) << 4;
    *ptr = base64[k];
    ptr++;

    *ptr = '=';
    ptr++;

    *ptr = '=';
    ptr++;
    break;
  case 2:
    k = (((unsigned char) buf[i]) & 0xFC) >> 2;
    *ptr = base64[k];
    ptr++;

    k = (((unsigned char) buf[i]) & 0x03) << 4;
    k |= (((unsigned char) buf[i + 1]) & 0xF0) >> 4;
    *ptr = base64[k];
    ptr++;

    k = (((unsigned char) buf[i + 1]) & 0x0F) << 2;
    *ptr = base64[k];
    ptr++;

    *ptr = '=';
    ptr++;
    break;
  }

  *ptr = '\0';
  *newlen = (int) (ptr - out);
  return out;
}
#endif


char hexa[16] = "0123456789abcdef";

/* calculate AKA request-digest/response-digest as per HTTP Digest spec */
void
DigestCalcResponseAka (IN const char *pszPassword, IN const char *pszNonce,     /* nonce from server */
                       IN const char *pszCNonce,        /* client nonce */
                       IN const char *pszQop,   /* qop-value: "", "auth", "auth-int" */
                       IN const char *pszMethod,        /* method from the request */
                       IN const char *pszDigestUri,     /* requested URL */
                       IN int version,  /* AKA version */
                       OUT RESHEXAKA2 resp_hex
                       /* request-digest or response-digest */ )
{

  char tmp[MAX_HEADER_LEN];

  /* static unsigned int mync = 1; */
  /* int has_opaque = 0; */
  char *nonce64, *nonce;
  int noncelen;
  RAND rnd;
  MAC mac, xmac;
  SQN sqn_he;
  K k;
  RES res;
  CK ck;
  IK ik;
  AK ak;
  int i, j;


  /* Compute the AKA response */
  resp_hex[0] = 0;
  snprintf (tmp, MAX_HEADER_LEN - 1, "%s", pszNonce);
  tmp[MAX_HEADER_LEN - 1] = 0;
  nonce64 = tmp;
  nonce = base64_decode_string (nonce64, (unsigned int) strlen (tmp), &noncelen);
  if (nonce == NULL)
    return;

  if (noncelen < RANDLEN + AUTNLEN) {
    /* Nonce is too short */
    osip_free (nonce);
    goto done;
  }
  memcpy (rnd, nonce, RANDLEN);
  /* memcpy(autn,nonce+RANDLEN,AUTNLEN); */
  memcpy (sqn_he, nonce + RANDLEN, SQNLEN);
  memcpy (mac, nonce + RANDLEN + SQNLEN + AMFLEN, MACLEN);

  osip_free (nonce);

  j = (int) strlen (pszPassword);
  memcpy (k, pszPassword, j);
  memset (k + j, 0, KLEN - j);

  /* compute XMAC */
  f1 (k, rnd, sqn_he, amf, xmac);
  if (memcmp (mac, xmac, MACLEN) != 0) {
    /*
       createAuthHeaderAKAv1MD5 : MAC != eXpectedMAC
       -> Server might not know the secret (man-in-the-middle attack?)
       return OSIP_SUCCESS;
     */
  }

  /* compute the response and keys */
  f2345 (k, rnd, res, ck, ik, ak);
  /* no check for sqn is performed, so no AUTS synchronization performed */

  /* Format data for output in the SIP message */
  for (i = 0; i < RESLEN; i++) {
    resp_hex[2 * i] = hexa[(res[i] & 0xF0) >> 4];
    resp_hex[2 * i + 1] = hexa[res[i] & 0x0F];
  }
  resp_hex[RESHEXLEN - 1] = 0;

done:
  switch (version) {
  case 1:
    /* AKA v1 */
    /* Nothing to do */
    break;
  case 2:
    /* AKA v2 */
    resp_hex[RESHEXLEN + IKLEN * 2 + CKLEN * 2 - 1] = 0;
    for (i = 0; i < IKLEN; i++) {
      resp_hex[RESLEN * 2 + 2 * i] = hexa[(ik[i] & 0xF0) >> 4];
      resp_hex[RESLEN * 2 + 2 * i + 1] = hexa[ik[i] & 0x0F];
    }
    for (i = 0; i < CKLEN; i++) {
      resp_hex[RESLEN * 2 + IKLEN * 2 + 2 * i] = hexa[(ck[i] & 0xF0) >> 4];
      resp_hex[RESLEN * 2 + IKLEN * 2 + 2 * i + 1] = hexa[ck[i] & 0x0F];
    }
    break;
  }
}

int
_eXosip_create_proxy_authorization_header (osip_proxy_authenticate_t * wa, const char *rquri, const char *username, const char *passwd, const char *ha1, osip_proxy_authorization_t ** auth, const char *method, const char *pCNonce, int iNonceCount)
{
  osip_proxy_authorization_t *aut;

  char *qop = NULL;
  char *Alg = "MD5";
  int version = 0;
  int i;

  /* make some test */
  if (passwd == NULL)
    return OSIP_BADPARAMETER;
  if (wa == NULL)
    return OSIP_BADPARAMETER;

  if (wa->auth_type == NULL || (wa->nonce == NULL)) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "www_authenticate header is not acceptable.\n"));
    return OSIP_SYNTAXERROR;
  }

  if (wa->realm == NULL) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "www_authenticate header contains an empty realm: contact your admin!\n"));
  }

  if (0 != osip_strcasecmp ("Digest", wa->auth_type)) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Authentication method not supported. (Digest only).\n"));
    return OSIP_UNDEFINED_ERROR;
  }

  /* "MD5" is invalid, but some servers use it. */
  if (wa->algorithm != NULL) {
    if (0 == osip_strcasecmp ("MD5", wa->algorithm)
        || 0 == osip_strcasecmp ("\"MD5\"", wa->algorithm)) {
    }
    else if (0 == osip_strcasecmp ("AKAv1-MD5", wa->algorithm)
             || 0 == osip_strcasecmp ("\"AKAv1-MD5\"", wa->algorithm)) {
      Alg = "AKAv1-MD5";
    }
    else if (0 == osip_strcasecmp ("AKAv2-MD5", wa->algorithm)
             || 0 == osip_strcasecmp ("\"AKAv2-MD5\"", wa->algorithm)) {
      Alg = "AKAv2-MD5";
    }
    else {
      OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Authentication method not supported. (MD5, AKAv1-MD5, AKAv2-MD5)\n"));
      return OSIP_UNDEFINED_ERROR;
    }
  }

  i = osip_proxy_authorization_init (&aut);
  if (i != 0) {
    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "allocation with authorization_init failed.\n"));
    return i;
  }

  /* just copy some feilds from response to new request */
  osip_proxy_authorization_set_auth_type (aut, osip_strdup ("Digest"));
  osip_proxy_authorization_set_realm (aut, osip_strdup (osip_proxy_authenticate_get_realm (wa)));
  if (aut->realm == NULL) {
    aut->realm = (char *) osip_malloc (3);
    aut->realm[0] = '"';
    aut->realm[1] = '"';
    aut->realm[2] = '\0';
  }
  osip_proxy_authorization_set_nonce (aut, osip_strdup (osip_proxy_authenticate_get_nonce (wa)));
  if (osip_proxy_authenticate_get_opaque (wa) != NULL)
    osip_proxy_authorization_set_opaque (aut, osip_strdup (osip_proxy_authenticate_get_opaque (wa)));
  /* copy the username field in new request */
  aut->username = osip_malloc (strlen (username) + 3);
  if (aut->username == NULL) {
    osip_authorization_free (aut);
    return OSIP_NOMEM;
  }
  sprintf (aut->username, "\"%s\"", username);

  {
    char *tmp = osip_malloc (strlen (rquri) + 3);

    if (tmp == NULL) {
      osip_authorization_free (aut);
      return OSIP_NOMEM;
    }

    sprintf (tmp, "\"%s\"", rquri);
    osip_proxy_authorization_set_uri (aut, tmp);
  }
  osip_proxy_authorization_set_algorithm (aut, osip_strdup (Alg));

  qop = osip_www_authenticate_get_qop_options (wa);
  if (qop == NULL || qop[0] == '\0' || strlen (qop) < 4)
    qop = NULL;

  {
    char *pszNonce = NULL;
    char *pszCNonce = NULL;
    const char *pszUser = username;
    char *pszRealm = NULL;
    const char *pszPass = NULL;
    char *szNonceCount = NULL;
    char *pszMethod = (char *) method;  /* previous_answer->cseq->method; */
    char *pszQop = NULL;
    const char *pszURI = rquri;

    HASHHEX HA1;
    HASHHEX HA2 = "";
    HASHHEX Response;
    RESHEXAKA2 Response2;
    const char *pha1 = NULL;

    if (osip_proxy_authorization_get_realm (aut) == NULL) {
      pszRealm = osip_strdup ("");
    }
    else {
      pszRealm = osip_strdup_without_quote (osip_proxy_authorization_get_realm (aut));
    }

    pszPass = passwd;

    if (osip_www_authenticate_get_nonce (wa) == NULL) {
      osip_authorization_free (aut);
      osip_free (pszRealm);
      return OSIP_SYNTAXERROR;
    }
    pszNonce = osip_strdup_without_quote (osip_www_authenticate_get_nonce (wa));

    if (qop != NULL) {
      /* only accept qop="auth" */
      pszQop = osip_strdup ("auth");
      if (pszQop == NULL) {
        osip_authorization_free (aut);
        osip_free (pszNonce);
        osip_free (pszRealm);
        return OSIP_NOMEM;
      }

      szNonceCount = osip_malloc (10);
      if (szNonceCount == NULL) {
        osip_authorization_free (aut);
        osip_free (pszNonce);
        osip_free (pszRealm);
        osip_free (pszQop);
        return OSIP_NOMEM;
      }
      snprintf (szNonceCount, 9, "%.8i", iNonceCount);

      pszCNonce = osip_strdup (pCNonce);
      if (pszCNonce == NULL) {
        osip_authorization_free (aut);
        osip_free (pszNonce);
        osip_free (pszRealm);
        osip_free (pszQop);
        osip_free (szNonceCount);
        return OSIP_NOMEM;
      }

      osip_proxy_authorization_set_message_qop (aut, osip_strdup ("auth"));
      osip_proxy_authorization_set_nonce_count (aut, osip_strdup (szNonceCount));

      {
        char *tmp = osip_malloc (strlen (pszCNonce) + 3);

        if (tmp == NULL) {
          osip_authorization_free (aut);
          osip_free (pszNonce);
          osip_free (pszCNonce);
          osip_free (pszRealm);
          osip_free (pszQop);
          osip_free (szNonceCount);
          return OSIP_NOMEM;
        }
        sprintf (tmp, "\"%s\"", pszCNonce);
        osip_proxy_authorization_set_cnonce (aut, tmp);
      }
    }

    if (0 == osip_strcasecmp (Alg, "MD5")) {
      if (ha1 && ha1[0]) {
        /* Depending on algorithm=md5 */
        pha1 = ha1;
      }
      else {
        DigestCalcHA1 ("MD5", pszUser, pszRealm, pszPass, pszNonce, pszCNonce, HA1);
        pha1 = HA1;
      }
      version = 0;
      DigestCalcResponse ((char *) pha1, pszNonce, szNonceCount, pszCNonce, pszQop, version, pszMethod, pszURI, HA2, Response);
    }
    else {
      if (0 == osip_strcasecmp (Alg, "AKAv1-MD5"))
        version = 1;
      else
        version = 2;

      DigestCalcResponseAka (pszPass, pszNonce, pszCNonce, pszQop, pszMethod, pszURI, version, Response2);
      if (ha1 && ha1[0]) {
        /* Depending on algorithm=md5 */
        pha1 = ha1;
      }
      else {
        DigestCalcHA1 ("MD5", pszUser, pszRealm, Response2, pszNonce, pszCNonce, HA1);
        pha1 = HA1;
      }

      DigestCalcResponse ((char *) pha1, pszNonce, szNonceCount, pszCNonce, pszQop, version, pszMethod, pszURI, HA2, Response);
    }

    OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_INFO4, NULL, "Response in proxy_authorization |%s|\n", Response));
    {
      char *resp = osip_malloc (35);

      if (resp == NULL) {
        osip_authorization_free (aut);
        osip_free (pszNonce);
        osip_free (pszCNonce);
        osip_free (pszRealm);
        osip_free (pszQop);
        osip_free (szNonceCount);
        return OSIP_NOMEM;
      }

      snprintf (resp, 35, "\"%s\"", Response);
      resp[34] = 0;
      osip_proxy_authorization_set_response (aut, resp);
    }
    osip_free (pszNonce);
    osip_free (pszCNonce);
    osip_free (pszRealm);
    osip_free (pszQop);
    osip_free (szNonceCount);
  }

  *auth = aut;
  return OSIP_SUCCESS;
}

int
_eXosip_store_nonce (struct eXosip_t *excontext, const char *call_id, osip_proxy_authenticate_t * wa, int answer_code)
{
  struct eXosip_http_auth *http_auth;
  int pos;

  /* update entries with same call_id */
  for (pos = 0; pos < MAX_EXOSIP_HTTP_AUTH; pos++) {
    http_auth = &excontext->http_auths[pos];
    if (http_auth->pszCallId[0] == '\0')
      continue;
    if (osip_strcasecmp (http_auth->pszCallId, call_id) == 0 && ((http_auth->wa->realm == NULL && wa->realm == NULL)
                                                                 || (http_auth->wa->realm != NULL && wa->realm != NULL && osip_strcasecmp (http_auth->wa->realm, wa->realm) == 0))) {
      osip_proxy_authenticate_free (http_auth->wa);
      http_auth->wa = NULL;
      osip_proxy_authenticate_clone (wa, &(http_auth->wa));
      http_auth->iNonceCount = 1;
      if (http_auth->wa == NULL)
        memset (http_auth, 0, sizeof (struct eXosip_http_auth));
      return OSIP_SUCCESS;
    }
  }

  /* not found */
  for (pos = 0; pos < MAX_EXOSIP_HTTP_AUTH; pos++) {
    http_auth = &excontext->http_auths[pos];
    if (http_auth->pszCallId[0] == '\0') {
      snprintf (http_auth->pszCallId, sizeof (http_auth->pszCallId), "%s", call_id);
      snprintf (http_auth->pszCNonce, sizeof (http_auth->pszCNonce), "0a4f113b");
      http_auth->iNonceCount = 1;
      osip_proxy_authenticate_clone (wa, &(http_auth->wa));
      http_auth->answer_code = answer_code;
      if (http_auth->wa == NULL)
        memset (http_auth, 0, sizeof (struct eXosip_http_auth));
      return OSIP_SUCCESS;
    }
  }

  OSIP_TRACE (osip_trace (__FILE__, __LINE__, OSIP_ERROR, NULL, "Compile with higher MAX_EXOSIP_HTTP_AUTH value (current=%i)\n", MAX_EXOSIP_HTTP_AUTH));
  return OSIP_UNDEFINED_ERROR;
}

int
_eXosip_delete_nonce (struct eXosip_t *excontext, const char *call_id)
{
  struct eXosip_http_auth *http_auth;
  int pos;

  /* update entries with same call_id */
  for (pos = 0; pos < MAX_EXOSIP_HTTP_AUTH; pos++) {
    http_auth = &excontext->http_auths[pos];
    if (http_auth->pszCallId[0] == '\0')
      continue;
    if (osip_strcasecmp (http_auth->pszCallId, call_id) == 0) {
      osip_proxy_authenticate_free (http_auth->wa);
      memset (http_auth, 0, sizeof (struct eXosip_http_auth));
      return OSIP_SUCCESS;
    }
  }
  return OSIP_NOTFOUND;
}
