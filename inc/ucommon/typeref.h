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
 * A thread-safe atomic heap management system.  This is used to manage
 * immutable heap instances of object types that are reference counted
 * and automatically deleted when no longer used.  All references to the
 * object are through smart typeref pointers.  Both specific classes for
 * strings and byte arrays, and generic templates to support generic
 * types in the heap are offered.
 * @file ucommon/typeref.h
 */

#ifndef _UCOMMON_TYPEREF_H_
#define _UCOMMON_TYPEREF_H_

#ifndef _UCOMMON_CPR_H_
#include <ucommon/cpr.h>
#endif

#ifndef _UCOMMON_ATOMIC_H_
#include <ucommon/atomic.h>
#endif

#ifndef _UCOMMON_PROTOCOLS_H_
#include <ucommon/protocols.h>
#endif

#ifndef _UCOMMON_OBJECT_H_
#include <ucommon/object.h>
#endif

namespace ucommon {

/**
 * Heap base-class container for typeref objects.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT TypeCounted : public ObjectProtocol
{
protected:
	mutable atomic::counter count;
	size_t size;

	explicit TypeCounted(size_t size);

	TypeCounted(const TypeCounted &ref);

	virtual void dealloc();

public:
	inline bool is() const {
		return (count.get() > 0);
	}

	inline unsigned copies() const {
		return ((unsigned)count.get());
	}

	void retain();

	void release();
};

/**
 * Smart pointer base class for auto-retained objects.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT TypeRef
{
protected:
	TypeCounted *ref;		// heap reference...

	TypeRef(TypeCounted *object);
	TypeRef(const TypeRef& copy);
	TypeRef();

	void set(TypeCounted *object);

public:
	virtual ~TypeRef();

	void set(TypeRef ptr);
	void release(void);
	
	inline void operator=(TypeRef ptr) {
		set(ptr);
	}

	inline bool is() const {
		return ref != NULL;
	}

	inline bool operator!() const {
		return ref == NULL;
	}
};

} // namespace

#endif
