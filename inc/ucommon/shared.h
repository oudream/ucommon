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
 * Common thread shared data types we will use.  This is for specialized
 * data types that include locking to be thread-safe.
 * @file ucommon/shared.h
 */

#ifndef _UCOMMON_SHARED_H_
#define _UCOMMON_SHARED_H_

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

#ifndef	_UCOMMON_TYPEREF_H_
#include <ucommon/typeref.h>
#endif

#ifndef _UCOMMON_THREAD_H_
#include <ucommon/thread.h>
#endif

namespace ucommon {

class __EXPORT SharedRef : protected TypeRef
{
protected:
	Mutex lock;

	SharedRef();

	TypeRef get();

	void get(TypeRef& object);

	void put(TypeRef& object);
};

template<typename T>
class sharedref : private SharedRef
{
private:
	inline sharedref(const sharedref& copy) {};

public:
	inline sharedref() : SharedRef() {};

	inline operator typeref<T>() {
		lock.acquire();
		typeref<T> ptr(ref);
		lock.release();
		return ptr;
	}

	inline typeref<T> operator*() {
		lock.acquire();
		typeref<T> ptr(ref);
		lock.release();
		return ptr;
	}

	inline void put(typeref<T>& ptr) {
		SharedRef::put(ptr);
	}

	inline sharedref& operator=(typeref<T> ptr) {
		SharedRef::get(ptr);
		return *this;
	}

	inline sharedref& operator=(T obj) {
		typeref<T> ptr(obj);
		SharedRef::get(ptr);
		return *this;
	}
};

} // namespace

#endif
