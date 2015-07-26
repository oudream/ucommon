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
 * Smart pointer base class for auto-retained objects.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT TypeRef
{
protected:
    /**
	 * Heap base-class container for typeref objects.
	 * @author David Sugar <dyfet@gnutelephony.org>
	 */
	class __EXPORT Counted : public ObjectProtocol
	{
	protected:
		friend class TypeRef;

		mutable atomic::counter count;
		size_t size;
		void *memory;

		explicit Counted(void *addr, size_t size);

		virtual void dealloc();

	public:
		inline bool is() const {
			return (count.get() > 0);
		}

		inline unsigned copies() const {
			return ((unsigned)count.get());
		}

		void operator delete(void *addr);

		void retain();

		void release();
	};



	Counted *ref;		// heap reference...

	TypeRef(Counted *object);
	TypeRef(const TypeRef& copy);
	TypeRef();

	void set(Counted *object);

	static caddr_t alloc(size_t size);

	static caddr_t mem(caddr_t addr);

public:
	virtual ~TypeRef();

	void set(const TypeRef& ptr);
	void release(void);
	
	inline bool is() const {
		return ref != NULL;
	}

	inline bool operator!() const {
		return ref == NULL;
	}

	inline unsigned copies() const {
		if(!ref)
			return 0;
		return ref->copies();
	}

	inline size_t size() const {
		if(!ref)
			return 0;
		return ref->size;
	}
};

template<typename T>
class typeref : public TypeRef
{
private:
	class value : public Counted
	{
	public:
		inline value(caddr_t mem) : 
		Counted(mem, sizeof(value)) {};

		inline value(caddr_t mem, const T& object) : 
		Counted(mem, sizeof(value)) {
			data = object;
		}

		T data;
	};
 
public:
	inline typeref() :	TypeRef() {};

	inline typeref(const typeref& copy) : TypeRef(copy) {};

	inline typeref(const T& object) : TypeRef() {
		caddr_t p = TypeRef::alloc(sizeof(value));
		TypeRef::set(new(mem(p)) value(p, object)); 
	}

	inline T* operator->() {
		if(!ref)
			return NULL;
		value *v = polystatic_cast<value *>(ref);
		return &(v->data);
	}

	inline const T *operator*() const {
		if(!ref)
			return NULL;
		value *v = polystatic_cast<value*>(ref);
		return &(v->data);
	}

	inline operator const T&() const {
		value *v = polystatic_cast<value*>(ref);
		return *(&(v->data));
	}

	inline typeref& operator=(const typeref& ptr) {
		TypeRef::set(ptr);
		return *this;
	}

	inline void set(const T& object) {
		release();
		caddr_t p = TypeRef::alloc(sizeof(value));
		TypeRef::set(new(mem(p)) value(p, object));
	}

	inline typeref& operator=(const T& object) {
		set(object);
		return *this;
	}
};

class __EXPORT stringref : public TypeRef
{
private:
	class value : public Counted
	{
	public:
		value(caddr_t addr, size_t size, const char *str);

		char mem[1];
	};

public:
	stringref();
	
	stringref(const stringref& copy);

	stringref(const char *str);

	const char *operator*() const;

	inline operator const char *() const {
		return operator*();
	}

	stringref& operator=(const stringref& objref);

	stringref& operator=(const char *str);

	void set(const char *str);
};

class __EXPORT byteref : public TypeRef
{
public:
	class value : public Counted
	{
	protected:
		friend class byteref;

		uint8_t mem[1];

		value(caddr_t addr, size_t size, const uint8_t *data);

		value(caddr_t addr, size_t size);

		inline void destroy(void) {
			count.clear();
			release();
		}

	public:
		inline size_t max() {
			return size;
		}

		inline uint8_t *get() {
			return &mem[0];
		}
	};

	byteref();
	
	byteref(const byteref& copy);

	byteref(const uint8_t *str, size_t size);

	const uint8_t *operator*() const;

	inline operator const uint8_t *() const {
		return operator*();
	}

	byteref& operator=(const byteref& objref);

	byteref& operator=(value *bytes);

	void set(const uint8_t *str, size_t size);

	void set(value *bytes);

	static byteref::value *create(size_t size);

	static void destroy(value *bytes);
};

typedef	byteref::value	*bytevalues_t;
typedef	stringref	stringref_t;
typedef byteref		byteref_t;

} // namespace

#endif
