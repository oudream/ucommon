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

sstream::sstream(secure::client_t context) :
tcpstream()
{
    ssl = NULL;
    bio = NULL;
    cert = NULL;
    server = false;
    verified = secure::NONE;
}

sstream::sstream(const TCPServer *tcp, secure::server_t context, size_t size) :
tcpstream(tcp, (unsigned)size)
{
    ssl = NULL;
    bio = NULL;
    cert = NULL;
    server = true;
    verified = secure::NONE;
}

sstream::~sstream()
{
}

void sstream::open(const char *host, const char *service, size_t bufsize)
{
    if(server)
        return;

    tcpstream::open(host, service, (unsigned)bufsize);
}

void sstream::close(void)
{
    if(server)
        return;

    tcpstream::close();
}

void sstream::release(void)
{
    tcpstream::close();
}

ssize_t sstream::_write(const char *address, size_t size)
{
    return tcpstream::_write(address, size);
}

ssize_t sstream::_read(char *address, size_t size)
{
    return tcpstream::_read(address, size);
}

bool sstream::_wait(void)
{
    return tcpstream::_wait();
}

int sstream::sync()
{
    return tcpstream::sync();
}

} // namespace ucommon

#endif
