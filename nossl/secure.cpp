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

namespace ucommon {
#ifdef  _MSWINDOWS_
HCRYPTPROV __handle = (HCRYPTPROV)NULL;
#endif

secure::~secure()
{
}

bool secure::fips(void)
{
    return false;
}

bool secure::init(void)
{
    Thread::init();
    Socket::init();

#ifdef  _MSWINDOWS_
    if(__handle != (HCRYPTPROV)NULL)
        return false;

    if(CryptAcquireContext(&__handle, NULL, NULL, PROV_RSA_FULL, 0))
        return false;
    if(GetLastError() == (DWORD)NTE_BAD_KEYSET) {
        if(CryptAcquireContext(&__handle, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET))
            return false;
    }

    __handle = (HCRYPTPROV)NULL;
#endif

    return false;
}

secure::server_t secure::server(const char *cert, const char *key)
{
    return NULL;
}

secure::client_t secure::client(const char *ca)
{
    return NULL;
}

void secure::cipher(secure *context, const char *ciphers)
{
}

} // namespace ucommon
