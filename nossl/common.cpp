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

Digest::Digest()
{
    hashtype = NULL;
    hashid = 0;
    context = NULL;
    bufsize = 0;
    textbuf[0] = 0;
}

Digest::Digest(const char *type)
{
    hashtype = NULL;
    hashid = 0;
    context = NULL;
    bufsize = 0;
    textbuf[0] = 0;

    set(type);
}

Digest::~Digest()
{
    release();
    memset(buffer, 0, sizeof(buffer));
}

secure::string Digest::str(void)
{
    if(!bufsize)
        get();

    if(!bufsize)
        return secure::string();

    return secure::string(textbuf);
}

secure::keybytes Digest::key(void)
{
    if(!bufsize)
        get();

    if(!bufsize)
        return secure::keybytes();

    return secure::keybytes(buffer, bufsize);
}

secure::string Digest::uuid(const char *name, const uint8_t *ns)
{
    unsigned mask = 0x50;
    const char *type = "sha1";
    if(!has("sha1")) {
        mask = 0x30;
        type = "md5";
    }

    Digest md(type);
    if(ns)
        md.put(ns, 16);
    md.puts(name);
    uint8_t *buf = (uint8_t *)md.get();

    buf[6] &= 0x0f;
    buf[6] |= mask;
    buf[8] &= 0x3f;
    buf[8] |= 0x80;

    char str[40];
    String::hexdump(buf, str, "4-2-2-2-6");
    return secure::string(str);
}

secure::keybytes Digest::md5(const uint8_t *mem, size_t size)
{
    if(!mem || !size || !has("md5"))
        return secure::keybytes();

    digest_t digest("md5");
    digest.put(mem, size);
    mem = digest.get();
    return secure::keybytes(mem, digest.size() * 8, secure::KEY_DIGEST);
}

secure::keybytes Digest::sha1(const uint8_t *mem, size_t size)
{
    if(!mem || !size || !has("sha1"))
        return secure::keybytes();

    digest_t digest("sha1");
    digest.put(mem, size);
    mem = digest.get();
    return secure::keybytes(mem, digest.size() * 8, secure::KEY_DIGEST);
}

secure::keybytes Digest::sha256(const uint8_t *mem, size_t size)
{
    if(!has("sha256") || !mem || !size)
        return secure::keybytes();

    digest_t digest("sha256");
    digest.put(mem, size);
    mem = digest.get();
    return secure::keybytes(mem, digest.size() * 8, secure::KEY_DIGEST);
}

secure::keybytes Digest::sha384(const uint8_t *mem, size_t size)
{
    if(!mem || !has("sha384") || !size)
        return secure::keybytes();

    digest_t digest("sha384");
    digest.put(mem, size);
    mem = digest.get();
    return secure::keybytes(mem, digest.size() * 8, secure::KEY_DIGEST);
}

secure::string Digest::md5(const char *text)
{
    if(!text || !has("md5"))
        return secure::string();

    digest_t digest("md5");
    digest.puts(text);
    return secure::string(*digest, secure::MD5_DIGEST);
}

secure::string Digest::sha1(const char *text)
{
    if(!text || !has("sha1"))
        return secure::string();

    digest_t digest("sha1");
    digest.puts(text);
    return secure::string(*digest, secure::SHA_DIGEST);
}

secure::string Digest::sha256(const char *text)
{
    if(!text || !has("sha256"))
        return secure::string();

    digest_t digest("sha256");
    digest.puts(text);
    return secure::string(*digest, secure::SHA_DIGEST);
}

secure::string Digest::sha384(const char *text)
{
    if(!text || !has("sha384"))
        return secure::string();

    digest_t digest("sha384");
    digest.puts(text);
    return secure::string(*digest, secure::SHA_DIGEST);
}

secure::keybytes HMAC::sha256(secure::keybytes key, const uint8_t *mem, size_t size)
{
    if(!mem || !has("sha256"))
        return secure::keybytes();

	hmac_t hmac("sha256", key);
    hmac.put(mem, size);
    mem = hmac.get();
    return secure::keybytes(mem, hmac.size() * 8, secure::KEY_DIGEST);
}

secure::keybytes HMAC::sha384(secure::keybytes key, const uint8_t *mem, size_t size)
{
    if(!mem || !has("sha384"))
        return secure::keybytes();

	hmac_t hmac("sha384", key);
    hmac.put(mem, size);
    mem = hmac.get();
    return secure::keybytes(mem, hmac.size() * 8, secure::KEY_DIGEST);
}

#if defined(_MSWINDOWS_)

static void cexport(HCERTSTORE ca, FILE *fp)
{
    PCCERT_CONTEXT cert = NULL;
    const uint8_t *cp;
    char buf[80];

    while ((cert = CertEnumCertificatesInStore(ca, cert)) != NULL) {
        fprintf(fp, "-----BEGIN CERTIFICATE-----\n");
        size_t total = cert->cbCertEncoded;
        size_t count;
        cp = (const uint8_t *)cert->pbCertEncoded;
        while(total) {
            count = String::b64encode(buf, cp, total, 64);
            if(count)
                fprintf(fp, "%s\n", buf);
            total -= count;
            cp += count;
        }
        fprintf(fp, "-----END CERTIFICATE-----\n");
    }
}

const char *secure::oscerts(void)
{
    const char *path = "c:/temp/ca-bundle.crt";
    if(!is_file(path)) {
        if(oscerts(path))
            return NULL;
    }
    return path;
}

int secure::oscerts(const char *pathname)
{
    bool caset = false;
    string_t target;

    if(pathname[1] == ':' || pathname[0] == '/' || pathname[0] == '\\')
        target = pathname;
    else
        target = shell::path(shell::USER_CONFIG) + "/" + pathname;

    FILE *fp = fopen(*target, "wt");

    if(!fp)
        return ENOSYS;

    HCERTSTORE ca = CertOpenSystemStoreA((HCRYPTPROV)NULL, "ROOT");
    if(ca) {
        caset = true;
        cexport(ca, fp);
        CertCloseStore(ca, 0);
    }

    ca = CertOpenSystemStoreA((HCRYPTPROV)NULL, "CA");
    if(ca) {
        caset = true;
        cexport(ca, fp);
        CertCloseStore(ca, 0);
    }

    fclose(fp);

    if(!caset) {
        fsys::erase(*target);
        return ENOSYS;
    }
    return 0;
}

#else
const char *secure::oscerts(void)
{
    if(is_file("/etc/ssl/certs/ca-certificates.crt"))
        return "/etc/ssl/certs/ca-certificates.crt";

    if(is_file("/etc/pki/tls/ca-bundle.crt"))
        return "/etc/pki/tls/ca-bundle.crt";

    if(is_file("/etc/ssl/ca-bundle.pem"))
        return "/etc/ssl/ca-bundle.pem";

    return NULL;
}

int secure::oscerts(const char *pathname)
{
    string_t source = oscerts();
    string_t target;

    if(pathname[0] == '/')
        target = pathname;
    else
        target = shell::path(shell::USER_CONFIG) + "/" + pathname;

    if(!source)
        return ENOSYS;

    return fsys::copy(*source, *target);
}
#endif

void secure::uuid(char *str)
{
    static uint8_t buf[16];
    static Timer::tick_t prior = 0l;
    static unsigned short seq;
    Timer::tick_t current = Timer::ticks();

    Mutex::protect(&buf);

    // get our (random) node identifier...
    if(!prior)
        Random::fill(buf + 10, 6);

    if(current == prior)
        ++seq;
    else
        Random::fill((uint8_t *)&seq, sizeof(seq));

    buf[8] = (uint8_t)((seq >> 8) & 0xff);
    buf[9] = (uint8_t)(seq & 0xff);
    buf[3] = (uint8_t)(current & 0xff);
    buf[2] = (uint8_t)((current >> 8) & 0xff);
    buf[1] = (uint8_t)((current >> 16) & 0xff);
    buf[0] = (uint8_t)((current >> 24) & 0xff);
    buf[5] = (uint8_t)((current >> 32) & 0xff);
    buf[4] = (uint8_t)((current >> 40) & 0xff);
    buf[7] = (uint8_t)((current >> 48) & 0xff);
    buf[6] = (uint8_t)((current >> 56) & 0xff);

    buf[6] &= 0x0f;
    buf[6] |= 0x10;
    buf[8] |= 0x80;
    String::hexdump(buf, str, "4-2-2-2-6");
    Mutex::release(&buf);
}

secure::string secure::uuid(void)
{
    char buf[38];
    uuid(buf);
    return secure::string(buf);
}

HMAC::HMAC()
{
    hmactype = NULL;
    hmacid = 0;
    context = NULL;
    bufsize = 0;
    textbuf[0] = 0;
}

HMAC::HMAC(const char *digest, const secure::keybytes& key)
{
    context = NULL;
    bufsize = 0;
    hmactype = NULL;
    hmacid = 0;
    textbuf[0] = 0;

    set(digest, key);
}

HMAC::~HMAC()
{
    release();
    memset(buffer, 0, sizeof(buffer));
}

secure::string HMAC::str(void)
{
    if(!bufsize)
        get();

    if(!bufsize)
        return secure::string();

    return secure::string(textbuf);
}

secure::keybytes HMAC::key(void)
{
    if(!bufsize)
        get();

    if(!bufsize)
        return secure::keybytes();

    return secure::keybytes(buffer, bufsize);
}

Cipher::Key::Key(const char *cipher)
{
    hashtype = algotype = NULL;
    hashid = algoid = 0;

    secure::init();

    set(cipher);
}

Cipher::Key::Key(const char *cipher, const uint8_t *iv, size_t ivsize)
{
    hashtype = algotype = NULL;
    hashid = algoid = 0;

    secure::init();

    set(cipher, iv, ivsize);
}

Cipher::Key::Key(const char *cipher, secure::keybytes& iv)
{
    hashtype = algotype = NULL;
    hashid = algoid = 0;

    secure::init();

    if(iv.type() == secure::IV_BUFFER)
        set(cipher, *iv, iv.size() / 8);
    else
        set(cipher);
}

Cipher::Key::Key(const char *cipher, const char *digest)
{
    hashtype = algotype = NULL;
    hashid = algoid = 0;

    secure::init();
    set(cipher, digest);
}

Cipher::Key::Key(const char *cipher, const char *digest, const char *text, size_t size, const uint8_t *salt, unsigned rounds)
{
    hashtype = algotype = NULL;
    hashid = algoid = 0;

    secure::init();

    set(cipher, digest);
    assign(text, size, salt, rounds);
}

Cipher::Key::Key()
{
    secure::init();
    clear();
}

Cipher::Key::~Key()
{
    clear();
}

bool Cipher::Key::operator==(const Key& other) const
{
    if(!keysize && !other.keysize)
        return true;

    if(keysize != other.keysize)
        return false;

    return !memcmp(keybuf, other.keybuf, keysize);
}

void Cipher::Key::b64(const char *key)
{
    clear();
    String::b64decode(keybuf, key, sizeof(keybuf));
}

void Cipher::Key::set(const uint8_t *key, size_t size)
{
    if(!size || size >= sizeof(keybuf))
        return;

    memcpy(keybuf, key, size);
}

void Cipher::Key::set(const char *cipher, const uint8_t *iv, size_t ivsize)
{
    set(cipher);
    if(blksize != ivsize)
        clear();

    if(!blksize)
        return;

    memcpy(ivbuf, iv, ivsize);
}

size_t Cipher::Key::get(uint8_t *keyout, uint8_t *ivout)
{
    size_t size = 0;
    if(keysize) {
        size += keysize;
        memcpy(keyout, keybuf, keysize);
        if(ivout) {
            memcpy(ivout, ivbuf, blksize);
            size += blksize;
        }
    }
    return size;
}

bool Cipher::Key::set(const char *cipher, const secure::keybytes& iv)
{
    const uint8_t *ivp = *iv;
    size_t size = iv.size() / 8;

    if(!ivp)
        return false;

    if(iv.type() != secure::IV_BUFFER)
        return false;

    if(size != blksize)
        return false;

    set(cipher, ivp, size);
    return true;
}

bool Cipher::Key::set(const secure::keybytes& key) 
{
    const uint8_t *kvp = *key;
    size_t size = key.size() / 8;

    if(!kvp)
        return false;

    if(key.type() != secure::UNPAIRED_KEYTYPE)
        return false;

    if(size != keysize)
        return false;

    set(kvp, size);
    return true;
}

secure::string Cipher::Key::b64(void)
{
    secure::string bytes;
    bytes.b64(keybuf, keysize);
    return bytes;
}

void Cipher::Key::clear(void)
{
    algotype = NULL;
    hashtype = NULL;
    algoid = hashid = 0;
    keysize = blksize = 0;

    zerofill(keybuf, sizeof(keybuf));
    zerofill(ivbuf, sizeof(ivbuf));
}

Cipher::Cipher(const key_t key, mode_t mode, uint8_t *address, size_t size)
{
    bufaddr = NULL;
    bufsize = bufpos = 0;
    context = NULL;
    set(key, mode, address, size);
}

Cipher::Cipher()
{
    bufaddr = NULL;
    bufsize = bufpos = 0;
    context = NULL;
}

Cipher::~Cipher()
{
    flush();
    release();
}

size_t Cipher::flush(void)
{
    size_t total = bufpos;

    if(bufpos && bufsize) {
        push(bufaddr, bufpos);
        bufpos = 0;
    }
    bufaddr = NULL;
    return total;
}

size_t Cipher::puts(const char *text)
{
    char padbuf[64];
    if(!text || !bufaddr)
        return 0;

    size_t len = strlen(text) + 1;
    size_t pad = len % keys.iosize();

    put((const uint8_t *)text, len - pad);
    if(pad) {
        memcpy(padbuf, text + len - pad, pad);
        memset(padbuf + pad, 0, keys.iosize() - pad);
        put((const uint8_t *)padbuf, keys.iosize());
        zerofill(padbuf, sizeof(padbuf));
    }
    return flush();
}

void Cipher::set(uint8_t *address, size_t size)
{
    flush();
    bufsize = size;
    bufaddr = address;
    bufpos = 0;
}

size_t Cipher::process(uint8_t *buf, size_t len, bool flag)
{
    set(buf);
    if(flag)
        return pad(buf, len);
    else
        return put(buf, len);
}

int Random::get(void)
{
    uint16_t v;;
    fill((uint8_t *)&v, sizeof(v));
    v /= 2;
    return (int)v;
}

int Random::get(int min, int max)
{
    unsigned rand;
    int range = max - min + 1;
    unsigned umax;

    if(max < min)
        return 0;

    memset(&umax, 0xff, sizeof(umax));

    do {
        fill((uint8_t *)&rand, sizeof(rand));
    } while(rand > umax - (umax % range));

    return min + (rand % range);
}

double Random::real(void)
{
    unsigned umax;
    unsigned rand;

    memset(&umax, 0xff, sizeof(umax));
    fill((uint8_t *)&rand, sizeof(rand));

    return ((double)rand) / ((double)umax);
}

double Random::real(double min, double max)
{
    return real() * (max - min) + min;
}

void Random::uuid(char *str)
{
    uint8_t buf[16];

    fill(buf, sizeof(buf));
    buf[6] &= 0x0f;
    buf[6] |= 0x40;
    buf[8] &= 0x3f;
    buf[8] |= 0x80;
    String::hexdump(buf, str, "4-2-2-2-6");
}

secure::string Random::uuid(void)
{
    char buf[38];
    uuid(buf);
    return secure::string(buf);
}

} // namespace ucommon
