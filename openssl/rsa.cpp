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

RSA::RSA(size_t keysize)
{
    keypair = NULL;

    unsigned long e = RSA_F4;
    BIGNUM *bn = BN_new();
    if(BN_set_word(bn, e) != 1) {
        BN_free(bn);
        return;
    }
    
    keypair = RSA_new();
    if(!keypair)
        return;

    if(RSA_generate_key_ex((::RSA*)keypair, keysize, bn, NULL) != 1) {
        BN_free(bn);
        RSA_free((::RSA *)keypair);
        keypair = NULL;
        return;
    }
    BN_free(bn);
}

RSA::RSA(secure::string privkey, secure::string pubkey)
{
    keypair = RSA_new();
    if(!keypair)
        return;

    if(privkey.size()) {
        const char *key = *privkey;
        BIO *io = BIO_new_mem_buf((void*)key, strlen(key));
        PEM_read_bio_RSAPrivateKey(io, (::RSA **)&keypair, 0, NULL);
        BIO_free(io);
    }
    if(pubkey.size()) {
        const char *key = *pubkey;
        BIO *io = BIO_new_mem_buf((void*)key, strlen(key));
        PEM_read_bio_RSAPublicKey(io, (::RSA **)&keypair, 0, NULL);
        BIO_free(io);
    }
}

RSA::~RSA()
{
    if(keypair) {
        RSA_free((::RSA*)keypair);
        keypair = NULL;
    } 
}

secure::string RSA::pem(secure::strtype_t type)
{
    if(!keypair)
        return secure::string();

    BIO *bio = BIO_new(BIO_s_mem());
    
    switch(type) {
    case secure::PEM_PRIVATE:
        PEM_write_bio_RSAPrivateKey(bio, (::RSA*)keypair, NULL, NULL, 0, NULL, NULL);
        break;
    case secure::PEM_PUBLIC:
        PEM_write_bio_RSAPublicKey(bio, (::RSA*)keypair);
        break;
    default:
        BIO_free(bio);
        return secure::string();
    }

    size_t len = BIO_pending(bio);
    if(!len)
        return secure::string();

    temporary<char *>tmp(len + 1);
    BIO_read(bio, *tmp, len);
    (*tmp)[len] = 0;
    return secure::string(*tmp, type);        
}

} // namespace ucommon
