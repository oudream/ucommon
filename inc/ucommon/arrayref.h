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
 * Arrays of thread-safe strongly typed heap objects.  This is used for
 * arrays of smart pointers to immutable heap instances of object types 
 * that are reference counted and automatically deleted when no longer used.
 * @file ucommon/arrayref.h
 */

#ifndef _UCOMMON_ARRAYREF_H_
#define _UCOMMON_ARRAYREF_H_

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

class __EXPORT ArrayRef : public TypeRef
{
protected:
	class Array : public Counted
	{
	protected:
		friend class ArrayRef;

		Mutex lock;

		explicit Array(void *addr, size_t size);

		void assign(size_t index, Counted *object);

		virtual void dealloc();

		inline Counted **get(void) {
			return reinterpret_cast<Counted **>(((caddr_t)(this)) + sizeof(Array));
		}

		Counted *get(size_t index);
	};

	ArrayRef(size_t size);
	ArrayRef(size_t size, TypeRef& object); 
	ArrayRef(const ArrayRef& copy);
	ArrayRef();

	void assign(size_t index, TypeRef& t);

	void reset(TypeRef& object);

	void reset(Counted *object);

	Counted *get(size_t index);

	bool is(size_t index);

	static Array *create(size_t size);

public:
	void resize(size_t size);

	void realloc(size_t size);

	void clear(void);
};

template<typename T>
class arrayref : public ArrayRef
{
public:
	inline arrayref() :	ArrayRef() {};

	inline arrayref(const arrayref& copy) : ArrayRef(copy) {};

	inline arrayref(size_t size) : ArrayRef(size) {};

	inline arrayref(size_t size, typeref<T>& t) : ArrayRef(size, t) {};

	inline arrayref(size_t size, T t) : ArrayRef(size) {
		typeref<T> v(t);
		reset(v);
	}

	inline size_t find(typeref<T> v, size_t start = 0) {
		for(size_t index = start; index < size(); ++index) {
			if(is(index) && at(index) == v) {
				return index;
			}
		}
		return (size_t)(-1);
	}

	inline size_t count(typeref<T> v) {
		size_t found = 0;
		for(size_t index = 0; index < size(); ++index) {
			if(is(index) && at(index) == v)
				++found;
		}
		return found;
	}

	inline arrayref& operator=(const arrayref& copy) {
		TypeRef::set(copy);
		return *this;
	}

	inline arrayref& operator=(typeref<T>& t) {
		reset(t);
		return *this;
	}

	inline arrayref& operator=(T t) {
		typeref<T> v(t);
		reset(v);
	}

	inline typeref<T> operator[](size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline typeref<T> operator()(size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline typeref<T> at(size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline typeref<T>  value(size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline void value(size_t index, typeref<T>& t) {
		ArrayRef::assign(index, t);
	}

	inline void put(typeref<T>& target, size_t index) {
		TypeRef::put(target, ArrayRef::get(index));
	}

	inline void operator()(size_t index, typeref<T>& t) {
		ArrayRef::assign(index, t);
	}

	inline void operator()(size_t index, T t) {
		typeref<T> v(t);
		ArrayRef::assign(index, v);
	}

	inline void release(void) {
		TypeRef::set(nullptr);
	}
};

typedef arrayref<const uint8_t *> bytearray_t;
typedef arrayref<const char *> stringarray_t;

} // namespace

#endif
