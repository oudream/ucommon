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
#include "hmac256.cpp"
#include "hmac384.cpp"

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

void HMAC::set(const char *digest, secure::keybytes key)
{
    set(digest, (const char *)*key, key.size());
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

const uint8_t *HMAC::get(void)
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
