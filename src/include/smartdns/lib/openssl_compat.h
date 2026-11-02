/*************************************************************************
 *
 * Copyright (C) 2018-2025 Ruilin Peng (Nick) <pymumu@gmail.com>.
 *
 * smartdns is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * smartdns is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SMARTDNS_OPENSSL_COMPAT_H_
#define _SMARTDNS_OPENSSL_COMPAT_H_

/*
 * Single point of truth for OpenSSL version handling.
 *
 * smartdns must build with OpenSSL 1.0.2 (OpenWrt 19.07 class) through the
 * current release. Every version check must go through the macros below rather
 * than comparing OPENSSL_VERSION_NUMBER or OPENSSL_API_COMPAT inline.
 *
 * OPENSSL_API_COMPAT in particular is the wrong thing to test: it describes the
 * API level the *caller* asked to hide and can be overridden on the command
 * line, so keying library initialisation on it can skip SSL_library_init() on a
 * real 1.0.x build, leaving OpenSSL uninitialised and non-thread-safe.
 */

#include <openssl/opensslv.h>

#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <stdint.h>
#include <sys/types.h>

#define SMARTDNS_OPENSSL_V_1_0_0 0x10000000L
#define SMARTDNS_OPENSSL_V_1_0_2 0x10002000L
#define SMARTDNS_OPENSSL_V_1_1_0 0x10100000L
#define SMARTDNS_OPENSSL_V_1_1_1 0x10101000L
#define SMARTDNS_OPENSSL_V_3_0_0 0x30000000L

#define SMARTDNS_OPENSSL_AT_LEAST(v) (OPENSSL_VERSION_NUMBER >= (v))
#define SMARTDNS_OPENSSL_BEFORE(v)   (OPENSSL_VERSION_NUMBER <  (v))

/*
 * OpenSSL before 1.1.0 requires explicit library initialisation and per-lock
 * callbacks; 1.1.0 removed both entry points and became thread safe by itself.
 */
#define SMARTDNS_NEED_LEGACY_INIT SMARTDNS_OPENSSL_BEFORE(SMARTDNS_OPENSSL_V_1_1_0)

/*
 * const-qualifier adapter.
 *
 * X509_NAME_get_index_by_NID(), X509_NAME_get_entry(),
 * X509_NAME_ENTRY_get_data(), i2d_X509_PUBKEY(), X509_set_issuer_name() and
 * X509V3_EXT_conf_nid() only gained const parameters in OpenSSL 3.0 -- both
 * 1.0.x and 1.1.x declare them without. Passing a const pointer to them raises
 * -Wdiscarded-qualifiers there, so the qualifier is dropped exactly where the
 * library lacks it and kept everywhere else.
 */
#if SMARTDNS_OPENSSL_BEFORE(SMARTDNS_OPENSSL_V_3_0_0)
# define dns_unconst(type, p) ((type)(uintptr_t)(const void *)(p))
#else
# define dns_unconst(type, p) (p)
#endif

/*
 * TLS_*_method() (protocol flexible) replaced SSLv23_*_method() in 1.1.0.
 */
#if SMARTDNS_OPENSSL_AT_LEAST(SMARTDNS_OPENSSL_V_1_1_0)
# define dns_TLS_client_method() TLS_client_method()
# define dns_TLS_server_method() TLS_server_method()
#else
# define dns_TLS_client_method() SSLv23_client_method()
# define dns_TLS_server_method() SSLv23_server_method()
#endif

/*
 * Unified SSL write.
 *
 * QUIC needs SSL_write_ex2() to carry stream flags, 1.1.1+ reports partial
 * writes through SSL_write_ex(), and older releases only have SSL_write().
 * All three end up returning the number of bytes consumed, or <= 0 on error,
 * so call sites no longer need their own version ladder.
 */
static inline ssize_t dns_SSL_write(SSL *ssl, const void *buff, int num, uint64_t flags, size_t *written)
{
	int ret = 0;

	*written = 0;

#if defined(OSSL_QUIC1_VERSION) && !defined(OPENSSL_NO_QUIC)
	ret = SSL_write_ex2(ssl, buff, (size_t)num, flags, written);
#elif SMARTDNS_OPENSSL_AT_LEAST(SMARTDNS_OPENSSL_V_1_1_1)
	ret = SSL_write_ex(ssl, buff, (size_t)num, written);
#else
	ret = (int)SSL_write(ssl, buff, num);
	if (ret > 0) {
		*written = (size_t)ret;
	}
#endif

	if (ret <= 0) {
		return ret;
	}

	return (ssize_t)*written;
}

#endif /* _SMARTDNS_OPENSSL_COMPAT_H_ */
