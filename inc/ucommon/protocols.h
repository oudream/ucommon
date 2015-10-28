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
// along with GNU uCommon C++.  If not, see <http://www.gnu.org/licenses/>.

/**
 * Abstract interfaces and support.  This is a set of "protocols", a concept
 * borrowed from other object oriented languages, to define interfaces for
 * low level services.  By using a protocol base class which offers both
 * virtuals and support methods only, one can easily stack and share these
 * as common base classes without having to consider when the final derived
 * object implements them.  Core protocol methods always are tagged with a
 * _ prefix to make it easier to track their derivation.
 * @file ucommon/protocols.h
 * @author David Sugar <dyfet@gnutelephony.org>
 */

#ifndef _UCOMMON_PROTOCOLS_H_
#define _UCOMMON_PROTOCOLS_H_

#ifndef _UCOMMON_CPR_H_
#include <ucommon/cpr.h>
#endif

namespace ucommon {

class String;
class StringPager;

class __EXPORT MemoryProtocol
{
protected:
    friend class MemoryRedirect;

    /**
     * Protocol to allocate memory from the pager heap.  The size of the
     * request must be less than the size of the memory page used.  The
     * actual method is in a derived or stacked object.
     * @param size of memory request.
     * @return allocated memory or NULL if not possible.
     */
    virtual void *_alloc(size_t size) = 0;

public:
    virtual ~MemoryProtocol();

    /**
     * Convenience function.
     * @param size of memory request.
     * @return alocated memory or NULL if not possible.
     */
    inline void *alloc(size_t size) {
        return _alloc(size);
    }

    /**
     * Allocate memory from the pager heap.  The size of the request must be
     * less than the size of the memory page used.  The memory is initialized
     * to zero.  This uses alloc.
     * @param size of memory request.
     * @return allocated memory or NULL if not possible.
     */
    void *zalloc(size_t size);

    /**
     * Duplicate NULL terminated string into allocated memory.  This uses
     * alloc.
     * @param string to copy into memory.
     * @return allocated memory with copy of string or NULL if cannot allocate.
     */
    char *dup(const char *string);

    /**
     * Duplicate existing memory block into allocated memory.  This uses alloc.
     * @param memory to data copy from.
     * @param size of memory to allocate.
     * @return allocated memory with copy or NULL if cannot allocate.
     */
    void *dup(void *memory, size_t size);
};

/**
 * A redirection base class for the memory protocol.  This is used because
 * sometimes we choose a common memory pool to manage different objects.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT MemoryRedirect : public __VIRTUAL MemoryProtocol
{
private:
    MemoryProtocol *target;

public:
    MemoryRedirect(MemoryProtocol *protocol);

    virtual void *_alloc(size_t size) __OVERRIDE;
};

/**
 * Common locking protocol.  This is used for objects that may internally
 * have sync'd functions, directly or in a derived class, that lock the
 * current object.  The default handlers do nothing but offer the virtuals
 * as a stub.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT LockingProtocol
{
protected:
    virtual void _lock(void);
    virtual void _unlock(void);

public:
    virtual ~LockingProtocol();
};

/**
 * Used for forming stream output.  We would create a derived class who's
 * constructor creates an internal string object, and a single method to
 * extract that string.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT PrintProtocol
{
public:
    virtual ~PrintProtocol();

    /**
     * Extract formatted string for object.
     */
    virtual const char *_print(void) const = 0;
};

/**
 * Used for processing input.  We create a derived class that processes a
 * single character of input, and returns a status value.  EOF means it
 * accepts no more input and any value other than 0 is a character to also
 * unget.  Otherwise 0 is good to accept more input.  The constructor is
 * used to reference a final destination object in the derived class.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT InputProtocol
{
public:
    virtual ~InputProtocol();

    /**
     * Extract formatted string for object.
     * @param character code we are pushing.
     * @return 0 to keep processing, EOF if done, or char to unget.
     */
    virtual int _input(int code) = 0;
};

/**
 * A common base class for all managed objects.  This is used to manage
 * objects that might be linked or reference counted.  The base class defines
 * only core virtuals some common public methods that should be used by
 * all inherited object types.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT ObjectProtocol
{
public:
    /**
     * Method to retain (or increase retention) of an object.
     */
    virtual void retain(void) = 0;

    /**
     * Method to release (or decrease retention) of an object.
     */
    virtual void release(void) = 0;

    /**
     * Required virtual destructor.
     */
    virtual ~ObjectProtocol();

    /**
     * Retain (increase retention of) object when copying.
     */
    ObjectProtocol *copy(void);

    /**
     * Increase retention operator.
     */
    inline void operator++(void) {
        retain();
    }

    /**
     * Decrease retention operator.
     */
    inline void operator--(void) {
        release();
    }
};

/**
 * Key data protocol used for things like maps and ordered lists.
 */
class __EXPORT KeyProtocol
{
protected:
    virtual int keytype(void) const = 0;

    /**
     * Size of key data.
     */
    virtual size_t keysize(void) const = 0;

    /**
     * Buffer of key value.
     */
    virtual const void *keydata(void) const = 0;

    virtual bool equal(const KeyProtocol& compare) const;

    inline bool operator!=(const KeyProtocol& compare) const {
        return !equal(compare);
    }

    virtual ~KeyProtocol();
};

} // namespace ucommon

#endif
