// Copyright (C) 2006-2014 David Sugar, Tycho Softworks.
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
// along with GNU ucommon.  If not, see <http://www.gnu.org/licenses/>.

#ifndef DEBUG
#define DEBUG
#endif

#include <ucommon/ucommon.h>

#include <stdio.h>

using namespace ucommon;

static Socket::address testing("0.0.0.0");
static Socket::address localhost("127.0.0.1", 4444);
#ifdef  AF_INET6
static Socket::address any6("[::]:4242");
static Socket::address testing6("44:022:66::1", 4444);
static Socket::address localhost6("::1", 4444);
#endif

extern "C" int main()
{
    struct sockaddr_internet addr;

    char addrbuf[128];
    addrbuf[0] = 0;

    assert(localhost.get(AF_INET) != NULL);
    Socket::query(localhost.get(AF_INET), addrbuf, sizeof(addrbuf));
    assert(0 == strcmp(addrbuf, "127.0.0.1"));
    Socket::via((struct sockaddr *)&addr, localhost.get(AF_INET), sizeof(addr));
    Socket::query((struct sockaddr *)&addr, addrbuf, sizeof(addrbuf));
    assert(0 == strcmp(addrbuf, "127.0.0.1"));
    assert(eq((struct sockaddr *)&addr, localhost.get(AF_INET)));
    assert(eq_subnet((struct sockaddr *)&addr, localhost.get(AF_INET)));

    memset(addrbuf, 0, sizeof(addrbuf));
    size_t len = localhost.print(addrbuf, sizeof(addrbuf));
    assert(len == strlen(addrbuf));
    assert(0 == strcmp(addrbuf, "127.0.0.1"));
    assert(localhost.isLoopback());
    assert(localhost.getPort() == 4444);
    assert(testing.isAny());
    assert(testing.family() == AF_INET);
    testing.setLoopback();

    assert(testing.isLoopback());
    assert(testing.family() == AF_INET);

    testing.setPort(4444);
    assert(testing.getPort() == 4444);
    assert(testing == localhost);
    localhost.setAny();
    assert(localhost.isValid());
    assert(localhost.isAny());
    assert(localhost.family() == AF_INET);

#ifdef  AF_INET6
    // we can only test if interface/ipv6 support is actually running
    // so we use getinterface to find out first.  it will return -1 for
    // localhost interface if ipv6 is down...
    assert(any6.isAny());
    assert(any6.getPort() == 4242);
    assert(!testing6.isLoopback());
    assert(localhost6.isLoopback());
    assert(localhost6.getPort() == 4444);
    assert(localhost6.get(AF_INET6) != NULL);
    len = localhost6.print(addrbuf, sizeof(addrbuf), true);
    assert(len == 10);
    assert(0 == strcmp(addrbuf, "[::1]:4444"));
    len = localhost6.print(addrbuf, sizeof(addrbuf));
    assert(len == 3);
    assert(0 == strcmp(addrbuf, "::1"));
    len = localhost6.print(addrbuf, sizeof(addrbuf), false, true);
    assert(len == 5);
    assert(0 == strcmp(addrbuf, "[::1]"));
    if(!Socket::via((struct sockaddr *)&addr, localhost6.get(AF_INET6), sizeof(addr))) {
        Socket::query((struct sockaddr *)&addr, addrbuf, sizeof(addrbuf));
        assert(0 == strcmp(addrbuf, "::1"));
        assert(Socket::equal((struct sockaddr *)&addr, localhost6.get(AF_INET6)));
        Socket::query(testing6.get(AF_INET6), addrbuf, sizeof(addrbuf));
        assert(0 == strcmp(addrbuf, "44:22:66::1"));
    }
#endif
    return 0;
}
