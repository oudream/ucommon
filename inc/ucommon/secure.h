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

/**
 * This library holds basic cryptographic functions and secure socket support
 * for use with GNU uCommon C++.  This library might be used in conjunction
 * with openssl, gnutls, etc.  If no secure socket library is available, then
 * a stub library may be used with very basic cryptographic support.
 * @file ucommon/secure.h
 */

/**
 * Example of SSL socket code.
 * @example ssl.cpp
 */

/**
 * Example of cryptographic digest code.
 * @example digest.cpp
 */

/**
 * Example of cipher code.
 * @example cipher.cpp
 */

#ifndef _UCOMMON_SECURE_H_
#define _UCOMMON_SECURE_H_

#ifndef _UCOMMON_CONFIG_H_
#include <ucommon/platform.h>
#endif

#ifndef _UCOMMON_UCOMMON_H_
#include <ucommon/ucommon.h>
#endif

#define MAX_CIPHER_KEYSIZE  512
#define MAX_DIGEST_HASHSIZE 512

namespace ucommon {

namespace Type {

    class SecChars 
    {
    public:
        typedef enum {GENERIC_STRING, B64_STRING, HEX_STRING, MD5_DIGEST, SHA_DIGEST, PEM_PUBLIC, PEM_PRIVATE} strtype_t;  
    };

    class KeyBytes
    {
    public:
        typedef enum {UNDEFINED_KEYTYPE, IV_BUFFER, UNPAIRED_KEYTYPE, RSA_KEYTYPE, KEY_DIGEST} keytype_t;
    };

}

class __SHARED AutoClear
{
private:
    __DELETE_DEFAULTS(AutoClear);

protected:
    size_t size;
    void *pointer;

    AutoClear(size_t alloc);

public:
    virtual ~AutoClear();
};    

template<typename T>
class autoclear : public AutoClear
{
private:
    __DELETE_COPY(autoclear);

public:
    autoclear() : AutoClear(sizeof(T)) {};

    inline operator T() {
        return *(static_cast<T*>(pointer));
    }

    inline T& operator*() {
        return *(static_cast<T*>(pointer));
    }

    inline T* operator->() {
        return static_cast<T*>(pointer);
    }
};

template <>
class autoclear<char *> : public AutoClear
{
private:
    __DELETE_COPY(autoclear);

public:
    autoclear(size_t len) : AutoClear(len) {};

    inline char *operator*() {
        return (char *)pointer;
    }
};

template <>
class autoclear<uint8_t *> : public AutoClear
{
private:
    __DELETE_COPY(autoclear);

public:
    autoclear(size_t len) : AutoClear(len) {};

    inline char *operator*() {
        return (char *)pointer;
    }
};

template <>
class __SHARED typeref<Type::SecChars> : protected TypeRef, public Type::SecChars
{
public:
    class storage : public Counted
    {
    private:
        friend class typeref;

        strtype_t type;
        char mem[1];

        __DELETE_COPY(storage);

        storage(caddr_t addr, size_t size, const char *str, strtype_t strtype = GENERIC_STRING);

        virtual void dealloc() __FINAL;

        inline const char *get() {
            return &mem[0];
        }

        inline size_t len() {
            return strlen(mem);
        }

        inline size_t max() {
            return size;
        }

        inline operator const char *() {
            return &mem[0];
        }
    };

    typeref();
    
    typeref(const typeref& copy);

    typeref(const char *str, strtype_t strtype = GENERIC_STRING);

    const char *operator*() const;

    inline operator const char *() const {
        return operator*();
    }

    bool operator==(const typeref& ptr) const;

    bool operator==(const char *obj) const;

    inline bool operator!=(const typeref& ptr) const {
        return !(*this == ptr);
    }

    inline bool operator!=(const char *obj) const {
        return !(*this == obj);
    }

    typeref& operator=(const typeref& objref);

    typeref& operator=(const char *str);

    void set(const char *str, strtype_t strtype = GENERIC_STRING);

    void b64(const uint8_t *bytes, size_t bsize);

    void hex(const uint8_t *bytes, size_t bsize);

    strtype_t type(void) const;

    size_t size(void) const;

    size_t len(void) const;
};

template <>
class __SHARED typeref<Type::KeyBytes> : protected TypeRef, public Type::KeyBytes
{
public:
    class storage : public Counted
    {
    private:
        friend class typeref;

        __DELETE_COPY(storage);

        keytype_t type;
        uint8_t mem[1];

        storage(caddr_t addr, size_t size, const uint8_t *key = NULL, keytype_t keytype = UNPAIRED_KEYTYPE);

        virtual void dealloc() __FINAL;

        inline const uint8_t *get() {
            return &mem[0];
        }

        inline operator const uint8_t *() {
            return &mem[0];
        }
    };

    typeref();
    
    typeref(const typeref& copy);

    typeref(size_t keysize, keytype_t keytype = UNPAIRED_KEYTYPE);

    typeref(const uint8_t *key, size_t keysize, keytype_t keytype = UNPAIRED_KEYTYPE);

    const uint8_t *operator*() const;

    inline operator const uint8_t *() const {
        return operator*();
    }

    bool operator==(const typeref& ptr) const;

    inline bool operator!=(const typeref& ptr) const {
        return !(*this == ptr);
    }

    typeref& operator=(const typeref& objref);

    void set(const uint8_t *str, size_t keysize, keytype_t keytype = UNPAIRED_KEYTYPE);

    void generate(size_t keysize, keytype_t keytype = UNPAIRED_KEYTYPE);

    keytype_t type(void) const;

    size_t size(void) const;

    size_t bytes(void) const;
};

template<>
inline size_t mapkeypath<Type::SecChars>(typeref<Type::SecChars>& object)
{
	size_t path = 1;
	return MapRef::index(path, (const uint8_t *)(*object), object.len());
}

template<>
inline size_t mapkeypath<Type::KeyBytes>(typeref<Type::KeyBytes>& object)
{
	size_t path = object.size();
	return MapRef::index(path, *object, object.size() / 8);
}

/**
 * Common secure socket support.  This offers common routines needed for
 * secure/ssl socket support code.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __SHARED secure : public Type::SecChars, public Type::KeyBytes
{
public:
    /**
     * Different error states of the security context.
     */
    typedef enum {OK=0, INVALID, MISSING_CERTIFICATE, MISSING_PRIVATEKEY, INVALID_CERTIFICATE, INVALID_AUTHORITY, INVALID_PEERNAME, INVALID_CIPHER} error_t;

    typedef enum {NONE, SIGNED, VERIFIED} verify_t;

    typedef typeref<Type::SecChars> string;

    typedef arrayref<Type::SecChars> strarray;

    typedef queueref<Type::SecChars> strqueue;

    typedef typeref<Type::KeyBytes> keybytes;
    
    typedef typeref<Type::KeyBytes> keyarray;

    typedef typeref<Type::KeyBytes> keyqueue;

private:
    __DELETE_COPY(secure);

protected:
    /**
     * Last error flagged for this context.
     */
    error_t error;

    inline secure() {error = OK;}

public:
    /**
     * This is derived in different back-end libraries, and will be used to
     * clear certificate credentials.
     */
    virtual ~secure();

    /**
     * Convenience type to represent a security context.
     */
    typedef secure *client_t;

    typedef secure *server_t;

    /**
     * Convenience type to represent a secure socket session.
     */
    typedef void *session_t;

    /**
     * Convenience type to represent a ssl certificate object.
     */
    typedef void *cert_t;

    /**
     * Convenience type to represent a secure socket buf i/o stream.
     */
    typedef void *bufio_t;

    /**
     * Initialize secure stack for first use, and report if SSL support is
     * compiled in.
     * @return true if ssl support is available, false if not.
     */
    static bool init(void);

    /**
     * Initialize secure stack with fips support.  If fips support is not
     * successfully enabled, the secure stack is also not initialized.  Hence
     * init() can be used for non-fips certified operation if fips fails.
     * @return true if fips support enabled and stack initialized.
     */
    static bool fips(void);

    /**
     * Copy system certificates to a local path.
     * @param path to copy to.
     * @return 0 or error number on failure.
     */
    static int oscerts(const char *path);

    /**
     * Get path to system certificates.
     * @return path to system certificates.
     */
    static const char *oscerts(void);

    /**
     * Verify a certificate chain through your certificate authority.
     * This uses the ca loaded as an optional argument for client and
     * server.  Optionally the hostname of the connection can also be
     * verified by pulling the peer certificate.
     * @param session that is connected.
     * @param peername that we expect.
     * @return secure error level or secure::OK if none.
     */
    static error_t verify(session_t session, const char *peername = NULL);

    /**
     * Create a sever context.  The certificate file used will be based on
     * the init() method name.  This may often be /etc/ssl/certs/initname.pem.
     * Similarly, a matching private key certificate will also be loaded.  An
     * optional certificate authority document can be used when we are
     * establishing a service which ssl clients have their own certificates.
     * @param authority path to use or NULL if none.
     * @return a security context that is cast from derived library.
     */
    static server_t server(const char *keyfile = NULL, const char *authority = NULL);

    /**
     * Create an anonymous client context with an optional authority to
     * validate.
     * @param authority path to use or NULL if none.
     * @param paths of certificates to use.
     * @return a basic client security context.
     */
    static client_t client(const char *authority = NULL, const char *paths = NULL);

    /**
     * Create a peer user client context.  This assumes a user certificate
     * in ~/.ssl/certs and the user private key in ~/.ssl/private.  The
     * path to an authority is also sent.
     * @param authority path to use.
     */
    static client_t user(const char *authority);

    /**
     * Assign a non-default cipher to the context.
     * @param context to set cipher for.
     * @param ciphers to set.
     */
    static void cipher(secure *context, const char *ciphers);

    /**
     * Determine if the current security context is valid.
     * @return true if valid, -1 if not.
     */
    inline bool is_valid(void) const {
        return error == OK;
    };

    /**
     * Get last error code associated with the security context.
     * @return last error code or 0/OK if none.
     */
    inline error_t err(void) const {
        return error;
    };

    /**
     * Create 36 character traditional version 1 uuid.
     * @param string to write uuid into, must be 37 bytes or more.
     */
    static void uuid(char *string);

    static secure::string pass(const char *prompt, size_t size);

    static secure::string uuid(void);

    inline operator bool() const {
        return is_valid();
    }

    inline bool operator!() const {
        return !is_valid();
    }
};

/**
 * A generic data ciphering class.  This is used to construct cryptographic
 * ciphers to encode and decode data as needed.  The cipher type is specified
 * by the key object.  This class can be used to send output streaming to
 * memory or in a fixed size buffer.  If the latter is used, a push() method
 * is called through a virtual when the buffer is full.  Since block ciphers
 * are used, buffers should be aligned to the block size.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __SHARED Cipher
{
public:
    typedef enum {ENCRYPT = 1, DECRYPT = 0} mode_t;

    /**
     * Cipher key formed by hash algorithm.  This can generate both a
     * key and iv table based on the algorithms used and required.  Normally
     * it is used from a pass-phrase, though any block of data may be
     * supplied.
     * @author David Sugar <dyfet@gnutelephony.org>
     */
    class __SHARED Key
    {
    protected:
        friend class Cipher;

        union {
            const void *algotype;
            int algoid;
        };

        union {
            const void *hashtype;
            int hashid;
        };

        int modeid;

        // assume 512 bit cipher keys possible...
        uint8_t keybuf[MAX_CIPHER_KEYSIZE / 8], ivbuf[MAX_CIPHER_KEYSIZE / 8];

        // generated keysize
        size_t keysize, blksize;

        Key(const char *ciper);

        void set(const char *cipher);

    public:
        Key();

        Key(const char *cipher, const char *digest, const char *text, size_t size = 0, const uint8_t *salt = NULL, unsigned rounds = 1);

        Key(const char *cipher, const uint8_t *iv, size_t ivsize);

        Key(const char *cipher, secure::keybytes& iv);

        Key(const char *cipher, const char *digest);

        ~Key();

        void set(const uint8_t *key, size_t size);

        inline secure::keybytes key() {
            return secure::keybytes(keybuf, keysize);
        }

        inline secure::keybytes iv() {
            return secure::keybytes(ivbuf, blksize, secure::IV_BUFFER);
        }

        bool set(const secure::keybytes& key);

        void set(const char *cipher, const char *digest);

        void set(const char *cipher, const uint8_t *iv, size_t ivsize);

        void assign(const char *key, size_t size, const uint8_t *salt, unsigned rounds);

        bool set(const char *cipher, const secure::keybytes& iv);

        void assign(const char *key, size_t size = 0);

        void clear(void);

        secure::string b64(void);

        void b64(const char *string);

        size_t get(uint8_t *key, uint8_t *ivout = NULL);

        inline size_t size(void) const {
            return keysize;
        }

        inline size_t iosize(void) const {
            return blksize;
        }

        inline operator bool() const {
            return keysize > 0;
        }

        inline bool operator!() const {
            return keysize == 0;
        }

        inline Key& operator=(const char *pass) {
            assign(pass); 
            return *this;
        }

        bool operator==(const Key& other) const;

        inline bool operator!=(const Key& other) const {
            return !operator==(other);
        }

        static void options(const uint8_t *salt = NULL, unsigned rounds = 1);
    };

    typedef Key *key_t;

private:
    Key keys;
    size_t bufsize, bufpos;
    mode_t bufmode;
    uint8_t *bufaddr;
    void *context;

    __DELETE_COPY(Cipher);

protected:
    virtual void push(uint8_t *address, size_t size);

    void release(void);

public:
    Cipher();

    Cipher(const key_t key, mode_t mode, uint8_t *address = NULL, size_t size = 0);

    virtual ~Cipher();

    void set(uint8_t *address, size_t size = 0);

    void set(const key_t key, mode_t mode, uint8_t *address, size_t size = 0);

    inline secure::keybytes iv() {
        return keys.iv();
    }

    inline secure::keybytes key() {
        return keys.key();
    }

    /**
     * Push a final cipher block.  This is used to push the final buffer into
     * the push method for any remaining data.
     */
    size_t flush(void);

    /**
     * Process cipher data.  This requires the size to be a multiple of the
     * cipher block size.  If an unaligned sized block of data is used, it
     * will be ignored and the size returned will be 0.
     * @param data to process.
     * @param size of data to process.
     * @return size of processed output, should be same as size or 0 if error.
     */
    size_t put(const uint8_t *data, size_t size);

    /**
     * This essentially encrypts a single string and pads with NULL bytes
     * as needed.
     * @param string to encrypt.
     * @return total encrypted size.
     */
    size_t puts(const char *string);

    /**
     * This is used to process any data unaligned to the blocksize at the end
     * of a cipher session.  On an encryption, it will add padding or an
     * entire padding block with the number of bytes to strip.  On decryption
     * it will remove padding at the end.  The pkcs5 method of padding with
     * removal count is used.  This also sets the address buffer to NULL
     * to prevent further puts until reset.
     * @param address of data to add before final pad.
     * @param size of data to add before final pad.
     * @return actual bytes encrypted or decrypted.
     */
    size_t pad(const uint8_t *address, size_t size);

    /**
     * Process encrypted data in-place.  This assumes no need to set the
     * address buffer.
     * @param address of data to process.
     * @param size of data to process.
     * @param flag if to pad data.
     * @return bytes processed and written back to buffer.
     */
    size_t process(uint8_t *address, size_t size, bool flag = false);

    inline size_t size(void) const {
        return bufsize;
    }

    inline size_t pos(void) const {
        return bufpos;
    }

    inline size_t align(void) const {
        return keys.iosize();
    }

    /**
     * Check if a specific cipher is supported.
     * @param name of cipher to check.
     * @return true if supported, false if not.
     */
    static bool has(const char *name);
};

/**
 * A cryptographic digest class.  This class can support md5 digests, sha1,
 * sha256, etc, depending on what the underlying library supports.  The
 * hash class accumulates the hash in the object.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __SHARED Digest
{
private:
    void *context;

    union {
        const void *hashtype;
        int hashid;
    };

    unsigned bufsize;
    uint8_t buffer[MAX_DIGEST_HASHSIZE / 8];
    char textbuf[MAX_DIGEST_HASHSIZE / 8 + 1];

    __DELETE_COPY(Digest);

protected:
    void release(void);

    const uint8_t *get(void);

public:
    Digest(const char *type);

    Digest();

    ~Digest();

    inline bool puts(const char *str) {
        return put(str, strlen(str));
    }

    inline Digest &operator<<(const char *str) {
        puts(str); 
        return *this;
    }

    inline Digest &operator<<(int16_t value) {
        int16_t v = htons(value); 
        put(&v, 2); 
        return *this;
    }

    inline Digest &operator<<(int32_t value) {
        int32_t v = htonl(value); 
        put(&v, 4); 
        return *this;
    }

    inline Digest &operator<<(const PrintProtocol& p) {
        const char *cp = p._print(); 
        if(cp) 
            puts(cp); 
        return *this;
    }

    bool put(const void *memory, size_t size);

    inline unsigned size() const {
        return bufsize;
    }

    secure::keybytes key(void);

    secure::string str(void);

    inline operator secure::string() {
        return str();
    }

    void set(const char *id);

    inline Digest& operator=(const char *id) {
        set(id);
        return *this;
    };

    inline bool operator *=(const char *text) {
        return puts(text);
    }

    inline bool operator +=(const char *text) {
        return puts(text);
    }

    inline secure::string operator*() {
        return str();
    }

    inline bool operator!() const {
        return !bufsize && context == NULL;
    }

    inline operator bool() const {
        return bufsize > 0 || context != NULL;
    }

    /**
     * Finalize and recycle current digest to start a new
     * digest.
     * @param binary digest used rather than text if true.
     */
    void recycle(bool binary = false);

    /**
     * Reset and restart digest object.
     */
    void reset(void);

    /**
     * Test to see if a specific digest type is supported.
     * @param name of digest we want to check.
     * @return true if supported, false if not.
     */
    static bool has(const char *name);

    static secure::string uuid(const char *name, const uint8_t *ns = NULL);

    /**
     * Shortcut for short md5 digests if supported...
     * @param text to create a digest for.
     * @return digest string.
     */
    static secure::string md5(const char *text);

    static secure::string sha1(const char *text);

    static secure::string sha256(const char *text);

    static secure::string sha384(const char *text);

    static secure::keybytes md5(const uint8_t *mem, size_t size);

    static secure::keybytes sha1(const uint8_t *mem, size_t size);

    static secure::keybytes sha256(const uint8_t *mem, size_t size);

    static secure::keybytes sha384(const uint8_t *mem, size_t size);

};

/**
 * A cryptographic message authentication code class.  This class can support 
 * md5 digests, sha1, sha256, etc, depending on what the underlying library 
 * supports.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __SHARED HMAC
{
private:
    void *context;

    union {
        const void *hmactype;
        int hmacid;
    };

    unsigned bufsize;
    uint8_t buffer[MAX_DIGEST_HASHSIZE / 8];
    char textbuf[MAX_DIGEST_HASHSIZE / 8 + 1];

    __DELETE_COPY(HMAC);

protected:
    void release(void);

    const uint8_t *get(void);

public:
    HMAC(const char *digest, const secure::keybytes& key);

    HMAC();

    ~HMAC();

    inline bool puts(const char *str) {
        return put(str, strlen(str));
    }

    inline HMAC &operator<<(const char *str) {
        puts(str); 
        return *this;
    }

    inline HMAC &operator<<(int16_t value) {
        int16_t v = htons(value); 
        put(&v, 2); 
        return *this;
    }

    inline HMAC &operator<<(int32_t value) {
        int32_t v = htonl(value); 
        put(&v, 4); 
        return *this;
    }

    inline HMAC &operator<<(const PrintProtocol& p) {
        const char *cp = p._print(); 
        if(cp) 
            puts(cp); 
        return *this;
    }

    bool put(const void *memory, size_t size);

    inline unsigned size() const {
        return bufsize;
    }

    secure::string str(void);

    secure::keybytes key(void);

    inline operator secure::string() {
        return str();
    }

    inline bool operator *=(const char *text) {
        return puts(text);
    }

    void set(const char *digest, const secure::keybytes& key);

    inline bool operator +=(const char *text) {
        return puts(text);
    }

    inline secure::string operator*() {
        return str();
    }

    inline bool operator!() const {
        return !bufsize && context == NULL;
    }

    inline operator bool() const {
        return bufsize > 0 || context != NULL;
    }

    /**
     * Test to see if a specific digest type is supported.
     * @param name of digest we want to check.
     * @return true if supported, false if not.
     */
    static bool has(const char *name);

    static secure::keybytes sha256(secure::keybytes key, const uint8_t *mem, size_t size);

    static secure::keybytes sha384(secure::keybytes key, const uint8_t *mem, size_t soze);
};

/**
 * Cryptographically relevant random numbers.  This is used both to gather
 * entropy pools and pseudo-random values.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __SHARED Random
{
private:
    __DELETE_DEFAULTS(Random);

public:
    /**
     * Push entropic seed.
     * @param buffer of random data to push.
     * @param size of buffer.
     * @return true if successful.
     */
    static bool seed(const uint8_t *buffer, size_t size);

    /**
     * Re-seed pseudo-random generation and entropy pools.
     */
    static void seed(void);

    /**
     * Get high-entropy random data.  This is often used to
     * initialize keys.  This operation may block if there is
     * insufficient entropy immediately available.
     * @param memory buffer to fill.
     * @param size of buffer.
     * @return number of bytes filled.
     */
    static size_t key(uint8_t *memory, size_t size);

    /**
     * Fill memory with pseudo-random values.  This is used
     * as the basis for all get and real operations and does
     * not depend on seed entropy.
     * @param memory buffer to fill.
     * @param size of buffer to fill.
     * @return number of bytes set.
     */
    static size_t fill(uint8_t *memory, size_t size);

    /**
     * Get a pseudo-random integer, range 0 - 32767.
     * @return random integer.
     */
    static int get(void);

    /**
     * Get a pseudo-random integer in a preset range.
     * @param min value of random integer.
     * @param max value of random integer.
     * @return random value from min to max.
     */
    static int get(int min, int max);

    /**
     * Get a pseudo-random floating point value.
     * @return psudo-random value 0 to 1.
     */
    static double real(void);

    /**
     * Get a pseudo-random floating point value in a preset range.
     * @param min value of random floating point number.
     * @param max value of random floating point number.
     * @return random value from min to max.
     */
    static double real(double min, double max);

    /**
     * Determine if we have sufficient entropy to return random
     * values.
     * @return true if sufficient entropy.
     */
    static bool status(void);

    /**
     * Create 36 character random uuid string.
     * @param string to write uuid into, must be 37 bytes or more.
     */
    static void uuid(char *string);

    static secure::string uuid(void);

    template <class T>
    inline static T value(void) {
        T tmp;
        Random::key(reinterpret_cast<uint8_t *>(&tmp), sizeof(tmp));
        return tmp;
    }

    template <class T>
    inline static T value(T max) {
        T slice;
        T value;

        value = 0xffffffff;
        slice = 0xffffffff / max;
        while(value >= max) {
            value = Random::value<T>() / slice;
        }
        return value;
    }

    template <class T>
    inline static T value(T min, T max)
    {
        return min + Random::value<T>(max - min);
    }    
};


/**
 * Convenience type for generic digests.
 */
typedef Digest digest_t;

/**
 * Convenience type for generic digests.
 */
typedef HMAC hmac_t;

/**
 * Convenience type for generic ciphers.
 */
typedef Cipher cipher_t;

/**
 * Convenience type for generic cipher key.
 */
typedef Cipher::Key skey_t;

inline void zerofill(void *addr, size_t size)
{
    ::memset(addr, 0, size);
}

#ifndef UCOMMON_SYSRUNTIME

/**
 * Secure socket using std::iostream.  Being based on tcpstream, it also
 * inherits the character protocol.  If no context is given or the handshake 
 * fails, then the stream defaults to insecure TCP connection behavior.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __SHARED sstream : public tcpstream
{
private:
    __DELETE_COPY(sstream);

protected:
    secure::session_t ssl;
    secure::bufio_t bio;
    secure::cert_t cert;
    secure::verify_t verified;
    bool server;

public:
    sstream(secure::client_t context);
    sstream(const TCPServer *server, secure::server_t context, size_t size = 536);
    ~sstream();

    void open(const char *host, const char *service, size_t size = 536);

    void close(void);

    int sync();

    void release(void);

    virtual bool _verify(void *cert);

    ssize_t _write(const char *address, size_t size) __OVERRIDE;

    ssize_t _read(char *address, size_t size) __OVERRIDE;

    bool _wait(void) __OVERRIDE;

    inline void flush(void) {
        sync();
    }

    inline secure::cert_t certificate(void) const {
        return cert;
    }

    inline bool is_secure(void) const {
        return bio != NULL;
    }

    inline bool is_certificate(void) const {
        return cert != NULL;
    }

    inline bool is_verified(void) const {
        return verified == secure::VERIFIED;
    }

    inline bool is_signed(void) const {
        return verified != secure::NONE;
    }
};

#endif

// can be specialized...
template<typename T>
void clearmem(T &var)
{
    memset(&var, 0, sizeof(var));
}

typedef secure::string keystring_t;

} // namespace ucommon

#endif
