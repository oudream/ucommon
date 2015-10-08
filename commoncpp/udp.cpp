// Copyright (C) 1999-2005 Open Source Telecom Corporation.
// Copyright (C) 2006-2014 David Sugar, Tycho Softworks.
// Copyright (C) 2015 Cherokees of Idaho.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.
//
// This exception applies only to the code released under the name GNU
// Common C++.  If you copy code from other releases into a copy of GNU
// Common C++, as the General Public License permits, the exception does
// not apply to the code that you add in this way.  To avoid misleading
// anyone as to the status of such modified files, you must delete
// this exception notice from them.
//
// If you write modifications of your own for GNU Common C++, it is your choice
// whether to permit this exception to apply to your modifications.
// If you do not wish that, delete this exception notice.
//

#include <ucommon-config.h>
#include <commoncpp/config.h>
#include <commoncpp/export.h>
#include <commoncpp/string.h>
#include <commoncpp/socket.h>
#include <commoncpp/udp.h>

#ifdef _MSWINDOWS_
#include <io.h>
#define _IOLEN64    (unsigned)
#define _IORET64    (int)
typedef int socklen_t;

#define socket_errno    WSAGetLastError()
#else
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#ifdef HAVE_NET_IP6_H
#include <netinet/ip6.h>
#endif
#define socket_errno errno
# ifndef  O_NONBLOCK
#  define O_NONBLOCK    O_NDELAY
# endif
# ifdef IPPROTO_IP
#  ifndef  SOL_IP
#   define SOL_IP   IPPROTO_IP
#  endif // !SOL_IP
# endif  // IPPROTO_IP
#endif   // !WIN32

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK (unsigned long)0x7f000001
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if defined(__hpux)
#define _XOPEN_SOURCE_EXTENDED
#endif

#ifdef  HAVE_NET_IF_H
#include <net/if.h>
#endif

#ifndef _IOLEN64
#define _IOLEN64
#endif

#ifndef _IORET64
#define _IORET64
#endif

namespace ost {

#ifdef  HAVE_GETADDRINFO

UDPSocket::UDPSocket(const char *name, Family fam) :
Socket(fam, SOCK_DGRAM, IPPROTO_UDP)
{
    char namebuf[128], *cp;
    struct addrinfo hint, *list = NULL, *first;

    family = fam;
    peer.setAny(fam);

    snprintf(namebuf, sizeof(namebuf), "%s", name);
    cp = strrchr(namebuf, '/');
    if(!cp && family == IPV4)
        cp = strrchr(namebuf, ':');

    if(!cp) {
        cp = namebuf;
        name = NULL;
    }
    else {
        name = namebuf;
        *(cp++) = 0;
        if(!strcmp(name, "*"))
            name = NULL;
    }

    memset(&hint, 0, sizeof(hint));

    hint.ai_family = family;
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_protocol = IPPROTO_UDP;
    hint.ai_flags = AI_PASSIVE;

    if(getaddrinfo(name, cp, &hint, &list) || !list) {
        error(errBindingFailed, (char *)"Could not find service", errno);
        endSocket();
        return;
    }

#if defined(SO_REUSEADDR)
    int opt = 1;
    setsockopt(so, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
        (socklen_t)sizeof(opt));
#endif

    first = list;

    while(list) {
        if(!bind(so, list->ai_addr, (socklen_t)list->ai_addrlen)) {
            state = BOUND;
            break;
        }
        list = list->ai_next;
    }
    freeaddrinfo(first);

    if(state != BOUND) {
        endSocket();
        error(errBindingFailed, (char *)"Count not bind socket", errno);
        return;
    }
}

#else

UDPSocket::UDPSocket(const char *name, Family fam) :
Socket(fam, SOCK_DGRAM, IPPROTO_UDP)
{
    char namebuf[128], *cp;
    family = fam;
    peer.setAny(fam);

    snprintf(namebuf, sizeof(namebuf), "%s", name);
    cp = strrchr(namebuf, '/');
    if(!cp && family == IPV4)
        cp = strrchr(namebuf, ':');

    if(!cp) {
        cp = namebuf;
        name = "*";
    }
    else {
        name = namebuf;
        *(cp++) = 0;
    }

    Socket::address addr(namebuf, cp, SOCK_DGRAM);

#if defined(SO_REUSEADDR)
    int opt = 1;
    setsockopt(so, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
        (socklen_t)sizeof(opt));
#endif

    if(addr && !bind(so, addr, addr.getLength()))
        state = BOUND;

    if(state != BOUND) {
        endSocket();
        error(errBindingFailed, (char *)"Count not bind socket", errno);
        return;
    }
}

#endif

UDPSocket::UDPSocket(Family fam) :
Socket(fam, SOCK_DGRAM, IPPROTO_UDP)
{
    family = fam;
    peer.setAny(fam);
}

UDPSocket::UDPSocket(const ucommon::Socket::address &ia) :
Socket(ia.family(), SOCK_DGRAM, IPPROTO_UDP)
{
    family = ia.family() == AF_INET6 ? IPV6 : IPV4;
    peer = ia;
#if defined(SO_REUSEADDR)
    int opt = 1;
    setsockopt(so, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, (socklen_t)sizeof(opt));
#endif
    if(bind(so, peer, (socklen_t)peer.getLength())) {
        endSocket();
        error(errBindingFailed,(char *)"Could not bind socket",socket_errno);
        return;
    }
    state = BOUND;
}

UDPSocket::UDPSocket(const IPV4Address &ia, in_port_t port) :
Socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP), peer(ia.getAddress(), port), family(IPV4)
{
#if defined(SO_REUSEADDR)
    int opt = 1;
    setsockopt(so, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, (socklen_t)sizeof(opt));
#endif
    if(bind(so, peer, sizeof(sockaddr_in))) {
        endSocket();
        error(errBindingFailed,(char *)"Could not bind socket",socket_errno);
        return;
    }
    state = BOUND;
}

#ifdef  CCXX_IPV6
UDPSocket::UDPSocket(const IPV6Address &ia, in_port_t port) :
Socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP), peer(ia.getAddress(), port), family(IPV6)
{
#if defined(SO_REUSEADDR)
    int opt = 1;
    setsockopt(so, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, (socklen_t)sizeof(opt));
#endif
    if(bind(so, peer, sizeof(sockaddr_in6))) {
        endSocket();
        error(errBindingFailed,(char *)"Could not bind socket",socket_errno);
        return;
    }
    state = BOUND;
}
#endif

UDPSocket::~UDPSocket()
{
    endSocket();
}

ssize_t UDPSocket::send(const void *buf, size_t len)
{
    const struct sockaddr *addr = peer;
    socklen_t alen = (socklen_t)peer.getLength();
    if(isConnected()) {
        addr = NULL;
        alen = 0;
    }

    return _IORET64 ::sendto(so, (const char *)buf, _IOLEN64 len, MSG_NOSIGNAL, addr, alen);
}

ssize_t UDPSocket::receive(void *buf, size_t len, bool reply)
{
    struct sockaddr *addr = peer;
    socklen_t alen = (socklen_t)peer.getLength();
    struct sockaddr_in6 senderAddress;  // DMC 2/7/05 ADD for use below.

    if(isConnected() || !reply) {
        // DMC 2/7/05 MOD to use senderAddress instead of NULL, to prevent 10014 error
        // from recvfrom.
        //addr = NULL;
        //alen = 0;
        addr = (struct sockaddr*)(&senderAddress);
        alen = sizeof(struct sockaddr_in6);
    }

    int bytes = ::recvfrom(so, (char *)buf, _IOLEN64 len, 0, addr, &alen);

#ifdef  _MSWINDOWS_

    if (bytes == SOCKET_ERROR) {
        WSAGetLastError();
    }
#endif

    return _IORET64 bytes;
}

Socket::Error UDPSocket::join(const IPV4Multicast &ia,int InterfaceIndex)
{
    return join(Socket::address(getaddress(ia)), InterfaceIndex);
}



Socket::Error UDPSocket::join(const ucommon::Socket::address &ia, int InterfaceIndex)
{
    return Socket::join(ia, InterfaceIndex);
}


Socket::Error UDPSocket::getInterfaceIndex(const char *DeviceName,int& InterfaceIndex)
{
#ifndef _MSWINDOWS_
#if defined(IP_ADD_MEMBERSHIP) && defined(SIOCGIFINDEX) && !defined(__FreeBSD__) && !defined(__FreeBSD_kernel__) && !defined(_OSF_SOURCE) && !defined(__hpux) && !defined(__GNU__)

    struct ip_mreqn  mreqn;
    struct ifreq       m_ifreq;
    int            i;
    sockaddr_in*          in4 = (sockaddr_in*)peer.get(AF_INET);
    InterfaceIndex = -1;

    memset(&mreqn, 0, sizeof(mreqn));
    memcpy(&mreqn.imr_multiaddr.s_addr, &in4->sin_addr, sizeof(mreqn.imr_multiaddr.s_addr));

    for (i = 0; i < IFNAMSIZ && DeviceName[i]; ++i)
        m_ifreq.ifr_name[i] = DeviceName[i];
    for (; i < IFNAMSIZ; ++i)
        m_ifreq.ifr_name[i] = 0;

    if (ioctl (so, SIOCGIFINDEX, &m_ifreq))
        return error(errServiceUnavailable);

#if defined(__FreeBSD__) || defined(__GNU__)
    InterfaceIndex = m_ifreq.ifr_ifru.ifru_index;
#else
    InterfaceIndex = m_ifreq.ifr_ifindex;
#endif
    return errSuccess;
#else
    return error(errServiceUnavailable);
#endif
#else
    return error(errServiceUnavailable);
#endif // WIN32
}

#ifdef  AF_UNSPEC
Socket::Error UDPSocket::disconnect(void)
{
    struct sockaddr_in addr;
    int len = sizeof(addr);

    if(so == INVALID_SOCKET)
        return errSuccess;

    Socket::state = BOUND;

    memset(&addr, 0, len);
#ifndef _MSWINDOWS_
    addr.sin_family = AF_UNSPEC;
#else
    addr.sin_family = AF_INET;
    memset(&addr.sin_addr, 0, sizeof(addr.sin_addr));
#endif
    if(::connect(so, (sockaddr *)&addr, len))
        return connectError();
    return errSuccess;
}
#else
Socket::Error UDPSocket::disconnect(void)
{
    if(so == INVALID_SOCKET)
        return errSuccess;

    Socket::state = BOUND;
    return connect(getLocal());
}
#endif

void UDPSocket::setPeer(const ucommon::Socket::address &host)
{
    peer = host;
}

void UDPSocket::connect(const ucommon::Socket::address &host)
{
    peer = host;
    if(so == INVALID_SOCKET)
        return;

    if(!::connect(so, host.get(), (socklen_t)host.getLength()))
        Socket::state = CONNECTED;
}

void UDPSocket::setPeer(const IPV4Host &ia, in_port_t port)
{
    peer = ucommon::Socket::address(ia.getAddress(), port);
}

void UDPSocket::connect(const IPV4Host &ia, in_port_t port)
{
    setPeer(ia, port);
    if(so == INVALID_SOCKET)
        return;

    if(!::connect(so, (struct sockaddr *)peer.get(AF_INET), sizeof(struct sockaddr_in)))
        Socket::state = CONNECTED;
}

#ifdef  CCXX_IPV6
void UDPSocket::setPeer(const IPV6Host &ia, in_port_t port)
{
    peer = ucommon::Socket::address(ia.getAddress(), port);
}

void UDPSocket::connect(const IPV6Host &ia, in_port_t port)
{
    setPeer(ia, port);

    if(so == INVALID_SOCKET)
        return;

    if(!::connect(so, (struct sockaddr *)peer.get(AF_INET6), sizeof(struct sockaddr_in6)))
        Socket::state = CONNECTED;

}

#endif

void UDPSocket::setPeer(const char *name)
{
    struct addrinfo *list = ucommon::Socket::query(name, NULL, SOCK_DGRAM, IPPROTO_UDP);
    peer = ucommon::Socket::address(list);
    freeaddrinfo(list);
}

void UDPSocket::connect(const char *service)
{
    int rtn = -1;

    setPeer(service);

    if(so == INVALID_SOCKET)
        return;

    rtn = ::connect(so, peer, (socklen_t)peer.getLength());
    if(!rtn)
        Socket::state = CONNECTED;
}

ucommon::Socket::address UDPSocket::getPeer()
{
    ucommon::Socket::address addr = Socket::getPeer();
    setPeer(addr);
    return addr;
}

IPV4Host UDPSocket::getIPV4Peer(in_port_t *port)
{
    ucommon::Socket::address addr = getPeer();
    if (is(addr)) {
        if(port)
            *port = peer.getPort();
    } else {
        peer.setAny();
        if(port)
            *port = 0;
    }

    return IPV4Host(ucommon::Socket::address::ipv4(addr)->sin_addr);
}

#ifdef  CCXX_IPV6
IPV6Host UDPSocket::getIPV6Peer(in_port_t *port)
{
    ucommon::Socket::address addr = getPeer();
    if (is(addr)) {
        if(port)
            *port = peer.getPort();
    } else {
        peer.setAny();
        if(port)
            *port = 0;
    }

    return IPV6Host(ucommon::Socket::address::ipv6(addr)->sin6_addr);
}
#endif

UDPBroadcast::UDPBroadcast(const IPV4Address &ia, in_port_t port) :
UDPSocket(ia, port)
{
    if(so != INVALID_SOCKET)
        setBroadcast(true);
}

void UDPBroadcast::setPeer(const IPV4Broadcast &ia, in_port_t port)
{
    peer = ucommon::Socket::address(ia.getAddress(), port);
}

UDPTransmit::UDPTransmit(const ucommon::Socket::address &ia) :
UDPSocket(ia)
{
    disconnect();   // assure not started live
    ::shutdown(so, 0);
    receiveBuffer(0);
}

UDPTransmit::UDPTransmit(const IPV4Address &ia, in_port_t port) :
UDPSocket(ia, port)
{
    disconnect();   // assure not started live
    ::shutdown(so, 0);
    receiveBuffer(0);
}

#ifdef  CCXX_IPV6
UDPTransmit::UDPTransmit(const IPV6Address &ia, in_port_t port) :
UDPSocket(ia, port)
{
    disconnect();   // assure not started live
    ::shutdown(so, 0);
    receiveBuffer(0);
}
#endif

UDPTransmit::UDPTransmit(Family family) : UDPSocket(family)
{
    disconnect();
    ::shutdown(so, 0);
    receiveBuffer(0);
}

Socket::Error UDPTransmit::connect(const ucommon::Socket::address &host)
{
    peer = host;
    if(peer.isAny())
        peer.setLoopback();
    if(::connect(so, peer, (socklen_t)peer.getLength()))
        return connectError();
    return errSuccess;
}

Socket::Error UDPTransmit::cConnect(const IPV4Address &ia, in_port_t port)
{
    return connect(ucommon::Socket::address(ia.getAddress(), port));
}

#ifdef  CCXX_IPV6

Socket::Error UDPTransmit::connect(const IPV6Address &ia, in_port_t port)
{
    return connect(ucommon::Socket::address(ia.getAddress(), port));
}
#endif

Socket::Error UDPTransmit::connect(const IPV4Host &ia, in_port_t port)
{
    if(isBroadcast())
        setBroadcast(false);

    return cConnect((IPV4Address)ia,port);
}

Socket::Error UDPTransmit::connect(const IPV4Broadcast &subnet, in_port_t  port)
{
    if(!isBroadcast())
        setBroadcast(true);

    return cConnect((IPV4Address)subnet,port);
}

Socket::Error UDPTransmit::connect(const IPV4Multicast &group, in_port_t port)
{
    Error err;
    if(!( err = UDPSocket::setMulticast(true) ))
        return err;

    return cConnect((IPV4Address)group,port);
}

#ifdef  CCXX_IPV6
Socket::Error UDPTransmit::connect(const IPV6Multicast &group, in_port_t port)
{
    Error error;
    if(!( error = UDPSocket::setMulticast(true) ))
        return error;

    return connect((IPV6Address)group,port);
}
#endif

UDPReceive::UDPReceive(const ucommon::Socket::address &ia) :
UDPSocket(ia)
{
    ::shutdown(so, 1);
    sendBuffer(0);
}

UDPReceive::UDPReceive(const IPV4Address &ia, in_port_t port) :
UDPSocket(ia, port)
{
    ::shutdown(so, 1);
    sendBuffer(0);
}

#ifdef  CCXX_IPV6
UDPReceive::UDPReceive(const IPV6Address &ia, in_port_t port) :
UDPSocket(ia, port)
{
    ::shutdown(so, 1);
    sendBuffer(0);
}
#endif

Socket::Error UDPReceive::connect(const ucommon::Socket::address &ia)
{
    ucommon::Socket::address host = ia;
    setPeer(host);

    // Win32 will crash if you try to connect to INADDR_ANY.
    if(host.isAny())
        host.setLoopback();

    if(::connect(so, host, (socklen_t)host.getLength()))
        return connectError();
    return errSuccess;
}

Socket::Error UDPReceive::connect(const IPV4Host &ia, in_port_t port)
{
    return connect(ucommon::Socket::address(ia.getAddress(), port));
}

#ifdef  CCXX_IPV6
Socket::Error UDPReceive::connect(const IPV6Host &ia, in_port_t port)
{
    return connect(ucommon::Socket::address(ia.getAddress(), port));
}
#endif

UDPDuplex::UDPDuplex(const ucommon::Socket::address &bind) :
UDPTransmit(bind.withPort(bind.getPort() + 1)), UDPReceive(bind)
{}

UDPDuplex::UDPDuplex(const IPV4Address &bind, in_port_t port) :
UDPTransmit(bind, port + 1), UDPReceive(bind, port)
{}

#ifdef  CCXX_IPV6
UDPDuplex::UDPDuplex(const IPV6Address &bind, in_port_t port) :
UDPTransmit(bind, port + 1), UDPReceive(bind, port)
{}
#endif

Socket::Error UDPDuplex::connect(const ucommon::Socket::address &host)
{
    Error rtn = UDPTransmit::connect(host);
    if(rtn) {
        UDPTransmit::disconnect();
        UDPReceive::disconnect();
        return rtn;
    }
    return UDPReceive::connect(host.withPort(host.getPort() + 1));
}

Socket::Error UDPDuplex::connect(const IPV4Host &host, in_port_t port)
{
    Error rtn = UDPTransmit::connect(host, port);
    if(rtn) {
        UDPTransmit::disconnect();
        UDPReceive::disconnect();
        return rtn;
    }
    return UDPReceive::connect(host, port + 1);
}

#ifdef  CCXX_IPV6
Socket::Error UDPDuplex::connect(const IPV6Host &host, in_port_t port)
{
    Error rtn = UDPTransmit::connect(host, port);
    if(rtn) {
        UDPTransmit::disconnect();
        UDPReceive::disconnect();
        return rtn;
    }
    return UDPReceive::connect(host, port + 1);
}
#endif

Socket::Error UDPDuplex::disconnect(void)
{
    Error rtn = UDPTransmit::disconnect();
    Error rtn2 = UDPReceive::disconnect();
    if (rtn) return rtn;
        return rtn2;
}

} // namespace ost
