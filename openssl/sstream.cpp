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

extern "C" {
    int verify_callback(X509_STORE_CTX *cert, void *obj) 
    {
        sstream *str = reinterpret_cast<sstream *>(obj);
        if(str->verify(cert))
            return 1;
        else
            return 0;
    }
}

sstream::sstream(secure::client_t scontext) :
tcpstream()
{
    __context *ctx = (__context *)scontext;
    ssl = NULL;
    bio = NULL;
    cert = NULL;
    server = false;
    verified = secure::NONE;

    if(ctx && ctx->ctx && ctx->err() == secure::OK) {
        ssl = SSL_new(ctx->ctx);
        cert = SSL_get_peer_certificate((SSL *)ssl);
    }
}

sstream::sstream(const TCPServer *tcp, secure::server_t scontext, size_t size) :
tcpstream(tcp, (unsigned)size)
{
    __context *ctx = (__context *)scontext;
    ssl = NULL;
    bio = NULL;
    cert = NULL;
    server = true;
    verified = secure::NONE;

    if(ctx && ctx->ctx && ctx->err() == secure::OK)
        ssl = SSL_new(ctx->ctx);

    if(!is_open() || !ssl)
        return;

    SSL_set_fd((SSL *)ssl, (int)getsocket());

    if(SSL_accept((SSL *)ssl) > 0) {
        bio = SSL_get_wbio((SSL *)ssl);
        cert = SSL_get_peer_certificate((SSL *)ssl);
        if(cert) {
            unsigned long res = SSL_get_verify_result((SSL *)ssl);
            switch(res) {
            case X509_V_OK:
                verified = secure::VERIFIED;
                break;
            case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
                verified = secure::SIGNED;
                break;
            default:
                break;
            }
        }
    }
}

sstream::~sstream()
{
    release();
}

void sstream::open(const char *host, const char *service, size_t size)
{
    if(server)
        return;

    close();
    tcpstream::open(host, service, (unsigned)size);

    if(!is_open() || !ssl)
        return;

    SSL_set_fd((SSL *)ssl, (int)getsocket());

    if(SSL_connect((SSL *)ssl) > 0) {
        bio = SSL_get_wbio((SSL *)ssl);
        cert = SSL_get_peer_certificate((SSL *)ssl);
        if(cert) {
            unsigned long res = SSL_get_verify_result((SSL *)ssl);
            switch(res) {
            case X509_V_OK:
                verified = secure::VERIFIED;
                break;
            case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
            case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
                verified = secure::SIGNED;
                break;
            default:
                break;
            }
        }
    }
}

void sstream::close(void)
{
    if(server)
        return;

    if(bio) {
        SSL_shutdown((SSL *)ssl);
        bio = NULL;
    }

    if(cert) {
        X509_free((X509*)cert);
        cert = NULL;
        verified = secure::NONE;
    }

    tcpstream::close();
}

void sstream::release(void)
{
    server = false;
    close();

    if(ssl) {
        SSL_free((SSL *)ssl);
        ssl = NULL;
    }
}

bool sstream::verify(void *cert)
{
    return true;
}

ssize_t sstream::_write(const char *address, size_t size)
{
    if(!bio)
        return tcpstream::_write(address, size);

    return SSL_write((SSL *)ssl, address, (int)size);
}

ssize_t sstream::_read(char *address, size_t size)
{
    if(!bio)
        return tcpstream::_read(address, size);

    return SSL_read((SSL *)ssl, address, (int)size);
}

bool sstream::_wait(void)
{
    if(so == INVALID_SOCKET)
        return false;

    if(ssl && SSL_pending((SSL *)ssl))
        return true;

    return tcpstream::_wait();
}

int sstream::sync()
{
    int rtn = tcpstream::sync();
    if(bio)
        rtn = BIO_flush((BIO *)bio);

    return rtn;
}

} // namespace ucommon

#endif
