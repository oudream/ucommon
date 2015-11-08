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

#ifndef UCOMMON_SYSRUNTIME

namespace ucommon {

sstream::sstream(secure::client_t scontext) :
tcpstream()
{
    ssl = __context::session((__context *)scontext);
    bio = NULL;
    cert = NULL;
    server = false;
    verified = secure::NONE;
}

sstream::sstream(const TCPServer *tcp, secure::server_t scontext, size_t size) :
tcpstream(tcp, size)
{

    ssl = __context::session((__context *)scontext);
    bio = NULL;
    cert = NULL;
    server = true;
    verified = secure::NONE;

    if(!is_open() || !ssl)
        return;

    gnutls_transport_set_ptr((SSL)ssl, reinterpret_cast<gnutls_transport_ptr_t>( so));
    int result = gnutls_handshake((SSL)ssl);

    if(result >= 0) {
        bio = ssl;
    }
}

sstream::~sstream()
{
    release();
}

void sstream::open(const char *host, const char *service, size_t bufsize)
{
    if(server)
        return;

    close();
    tcpstream::open(host, service, bufsize);

    if(!is_open() || !ssl)
        return;

    gnutls_transport_set_ptr((SSL)ssl, reinterpret_cast<gnutls_transport_ptr_t>(so));
    int result = gnutls_handshake((SSL)ssl);

    if(result >= 0) {
        bio = ssl;
    }
}

void sstream::close(void)
{
    if(server)
        return;

    if(bio) {
        gnutls_bye((SSL)ssl, GNUTLS_SHUT_RDWR);
        bio = NULL;
    }

    tcpstream::close();
}

void sstream::release(void)
{
    server = false;
    close();

    if(ssl) {
        gnutls_deinit((SSL)ssl);
        ssl = NULL;
    }
}

ssize_t sstream::_write(const char *address, size_t size)
{
    if(!bio)
        return tcpstream::_write(address, size);

    return gnutls_record_send((SSL)ssl, address, size);
}

ssize_t sstream::_read(char *address, size_t size)
{
    if(!bio)
        return tcpstream::_read(address, size);

    return gnutls_record_recv((SSL)ssl, address, size);
}

bool sstream::_wait(void)
{
    // we have no way to examine the pending queue in gnutls??
    if(ssl)
        return true;

    return tcpstream::_wait();
}

int sstream::sync()
{
    return tcpstream::sync();
}

} // namespace ucommon

#endif
