// Copyright (C) 2010-2014 David Sugar, Tycho Softworks.
// Copyright (C) 2015 Cherokees of Idaho.
//
// This file is part of GNU uCommon C++.
//
// GNU uCommon C++ is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GNU uCommon C++ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with GNU uCommon C++.  If not, see <http://www.gnu.org/licenses/>.

#include "local.h"

/*
  Copyright (C) 2012 Werner Dittmann

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

 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */

/*
 * Authors: Werner Dittmann
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef struct _hmacSha256Context {
    sha256_ctx ctx;
    sha256_ctx innerCtx;
    sha256_ctx outerCtx;
} hmacSha256Context;

static int32_t hmacSha256Init(hmacSha256Context *ctx, const uint8_t *key, uint32_t kLength)
{
    int32_t i;
    uint8_t localPad[SHA256_BLOCK_SIZE] = {0};
    uint8_t localKey[SHA256_BLOCK_SIZE] = {0};

    if (key == NULL)
        return 0;

    memset(ctx, 0, sizeof(hmacSha256Context));

    /* check key length and reduce it if necessary */
    if (kLength > SHA256_BLOCK_SIZE) {
        sha256_begin(&ctx->ctx);
        sha256_hash(key, kLength, &ctx->ctx);
        sha256_end(localKey, &ctx->ctx);
    }
    else {
        memcpy(localKey, key, kLength);
    }
    /* prepare inner hash and hold the context */
    for (i = 0; i < SHA256_BLOCK_SIZE; i++)
        localPad[i] = localKey[i] ^ 0x36;

    sha256_begin(&ctx->innerCtx);
    sha256_hash(localPad, SHA256_BLOCK_SIZE, &ctx->innerCtx);

    /* prepare outer hash and hold the context */
    for (i = 0; i < SHA256_BLOCK_SIZE; i++)
        localPad[i] = localKey[i] ^ 0x5c;

    sha256_begin(&ctx->outerCtx);
    sha256_hash(localPad, SHA256_BLOCK_SIZE, &ctx->outerCtx);

    /* copy prepared inner hash to work hash - ready to process data */
    memcpy(&ctx->ctx, &ctx->innerCtx, sizeof(sha256_ctx));

    memset(localKey, 0, sizeof(localKey));

    return 1;
}

static void hmacSha256Update(hmacSha256Context *ctx, const uint8_t *data, uint32_t dLength)
{
    /* hash new data to work hash context */
    sha256_hash(data, dLength, &ctx->ctx);
}

static void hmacSha256Final(hmacSha256Context *ctx, uint8_t *mac)
{
    uint8_t tmpDigest[SHA256_DIGEST_SIZE];

    /* finalize work hash context */
    sha256_end(tmpDigest, &ctx->ctx);

    /* copy prepared outer hash to work hash */
    memcpy(&ctx->ctx, &ctx->outerCtx, sizeof(sha256_ctx));

    /* hash inner digest to work (outer) hash context */
    sha256_hash(tmpDigest, SHA256_DIGEST_SIZE, &ctx->ctx);

    /* finalize work hash context to get the hmac*/
    sha256_end(mac, &ctx->ctx);
}

/*
  Copyright (C) 2012 Werner Dittmann

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

 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */

/*
 * Authors: Werner Dittmann
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

typedef struct _hmacSha384Context {
    sha384_ctx ctx;
    sha384_ctx innerCtx;
    sha384_ctx outerCtx;
} hmacSha384Context;

static int32_t hmacSha384Init(hmacSha384Context *ctx, const uint8_t *key, uint32_t kLength)
{
    int32_t i;
    uint8_t localPad[SHA384_BLOCK_SIZE] = {0};
    uint8_t localKey[SHA384_BLOCK_SIZE] = {0};

    if (key == NULL)
        return 0;

    memset(ctx, 0, sizeof(hmacSha384Context));

    /* check key length and reduce it if necessary */
    if (kLength > SHA384_BLOCK_SIZE) {
        sha384_begin(&ctx->ctx);
        sha384_hash(key, kLength, &ctx->ctx);
        sha384_end(localKey, &ctx->ctx);
    }
    else {
        memcpy(localKey, key, kLength);
    }
    /* prepare inner hash and hold the context */
    for (i = 0; i < SHA384_BLOCK_SIZE; i++)
        localPad[i] = localKey[i] ^ 0x36;

    sha384_begin(&ctx->innerCtx);
    sha384_hash(localPad, SHA384_BLOCK_SIZE, &ctx->innerCtx);

    /* prepare outer hash and hold the context */
    for (i = 0; i < SHA384_BLOCK_SIZE; i++)
        localPad[i] = localKey[i] ^ 0x5c;

    sha384_begin(&ctx->outerCtx);
    sha384_hash(localPad, SHA384_BLOCK_SIZE, &ctx->outerCtx);

    /* copy prepared inner hash to work hash - ready to process data */
    memcpy(&ctx->ctx, &ctx->innerCtx, sizeof(sha384_ctx));

    memset(localKey, 0, sizeof(localKey));

    return 1;
}

static void hmacSha384Update(hmacSha384Context *ctx, const uint8_t *data, uint32_t dLength)
{
    /* hash new data to work hash context */
    sha384_hash(data, dLength, &ctx->ctx);
}

static void hmacSha384Final(hmacSha384Context *ctx, uint8_t *mac)
{
    uint8_t tmpDigest[SHA384_DIGEST_SIZE];

    /* finalize work hash context */
    sha384_end(tmpDigest, &ctx->ctx);

    /* copy prepared outer hash to work hash */
    memcpy(&ctx->ctx, &ctx->outerCtx, sizeof(sha384_ctx));

    /* hash inner digest to work (outer) hash context */
    sha384_hash(tmpDigest, SHA384_DIGEST_SIZE, &ctx->ctx);

    /* finalize work hash context to get the hmac*/
    sha384_end(mac, &ctx->ctx);
}

// start of our interfaces...

namespace ucommon {

bool HMAC::has(const char *id)
{
    if(eq_case(id, "sha256"))
        return true;

    if(eq_case(id, "sha384"))
        return true;

    return false;
}

void HMAC::set(const char *digest, const char *key, size_t len)
{
    release();

    if(eq_case(digest, "sha256")) {
        hmactype = "2";
        context = new hmacSha256Context;
        hmacSha256Init((hmacSha256Context*)context, (const uint8_t *)key, len);
    }
    else if(eq_case(digest, "sha384")) {
        hmactype = "3";
        context = new hmacSha384Context;
        hmacSha384Init((hmacSha384Context*)context, (const uint8_t *)key, len);
    }
}

void HMAC::set(const char *digest, const secure::keybytes& key)
{
    set(digest, (const char *)*key, key.size() / 8);
}

void HMAC::release(void)
{
    if(context && hmactype) {
        switch(*((char *)hmactype)) {
        case '2':
            memset(context, 0, sizeof(hmacSha256Context));
            delete (hmacSha256Context *)context;
            break;
        case '3':
            memset(context, 0, sizeof(hmacSha384Context));
            delete (hmacSha384Context *)context;
            break;
        default:
            break;
        }
    }

    bufsize = 0;
    textbuf[0] = 0;
    hmactype = NULL;
    context = NULL;
}

bool HMAC::put(const void *address, size_t size)
{
    if(!context || !hmactype)
        return false;

    switch(*((char *)hmactype)) {
    case '2':
        hmacSha256Update((hmacSha256Context*)context, (const uint8_t *)address, size);
        return true;
    case '3':
        hmacSha384Update((hmacSha384Context*)context, (const uint8_t *)address, size);
        return true;
    default:
        return false;
    }
}

const unsigned char *HMAC::get(void)
{
    if(bufsize)
        return buffer;

    if(!context || !hmactype)
        return NULL;

    switch(*((char *)hmactype)) {
    case '2':
        hmacSha256Final((hmacSha256Context *)context, buffer);
        bufsize = SHA256_BLOCK_SIZE;
        break;
    case '3':
        hmacSha384Final((hmacSha384Context *)context, buffer);
        bufsize = SHA384_BLOCK_SIZE;
        break;
    default:
        return NULL;
    }

    size_t count = 0;
    while(count < bufsize) {
        snprintf(textbuf + (count * 2), 3, "%2.2x", buffer[count]);
        ++count;
    }
    return buffer;
}

} // namespace ucommon
