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

#ifndef _UCOMMON_GENERICS_H_
#include <ucommon/generics.h>
#endif

#ifndef _UCOMMON_OBJECT_H_
#include <ucommon/object.h>
#endif

namespace ucommon {

class TypeRelease;

/**
 * Smart pointer base class for auto-retained objects.  The underlying
 * container is heap allocated and page aligned.  A heap object is
 * automatically de-referenced by release during destruction.  The smart
 * pointer is a protected base class used to derive strongly typed
 * templates.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT TypeRef
{
protected:
	friend class ArrayRef;
	friend class SharedRef;
	friend class MapRef;
	friend class TypeRelease;

	class Release;

public:
	/**
	 * Heap base-class container for typeref objects.  This uses atomic
	 * reference counters for thread safety with maximal performance.  This
	 * is used as a protected base class used for strongly typed heap
	 * containers through templates.
	 * @author David Sugar <dyfet@gnutelephony.org>
	 */
	class __EXPORT Counted : public __PROTOCOL ObjectProtocol
	{
	private:
		__DELETE_COPY(Counted);

	protected:
		friend class TypeRef;
		friend class TypeRelease;

		union {
			TypeRelease *autorelease;			
			Counted *link;
		};

		mutable Atomic::counter count;
		unsigned offset;
		size_t size;

		/**
		 * Construction of aligned container.  This is used to inform the
		 * object of the underlying real address it exists on the heap 
		 * since malloc is not assured to be atomically aligned by default.
		 * @param address of actual allocation.
		 * @param size of object allocated.
		 * @param ar pool to use
		 */
		explicit Counted(void *address, size_t size, TypeRelease *ar = nullptr);

		/**
		 * Release memory and delete object when no longer referenced.
		 * This gets called with the atomic reference counter < 1, such
		 * as when the last smart pointer de-references.
		 */
		virtual void dealloc(void);

	public:
		/**
		 * Is this object not empty?
		 * @return true if not empty.
		 */
		inline bool is() const {
			return (count.get() > 0);
		}

		/**
		 * Number of retains (smart pointers) referencing us.
		 * @return number of copies of pointers referencing.
		 */
		inline unsigned copies() const {
			return ((unsigned)count.get());
		}

		inline TypeRelease *getRelease() const {
			return autorelease;
		}

		/**
		 * Override delete to de-allocate actual heap.  This
		 * is used because the object is atomically aligned, but
		 * the heap may not be.
		 * @param address of our object.
		 */
		void operator delete(void *address);

		/**
		 * Retain a copy of this object.  Usually a smart pointer
		 * referencing.
		 */
		void retain();

		/**
		 * Release a copy of this object.  Only when the reference
		 * count reaches 0 is it destroyed. 
		 */
		void release();
	};		

protected:
	Counted *ref;		// heap reference...

	/**
	 * Create a smart pointer referencing an existing heap object.
	 * @param object to reference.
	 */
	TypeRef(Counted *object);

	/**
	 * Create a smart pointer based on another pointer.  Both
	 * pointers then reference the same object.
	 * @param pointer instance to share reference with.
	 */
	TypeRef(const TypeRef& pointer);

	/**
	 * Create a smart pointer referencing nothing.
	 */
	TypeRef();

	/**
	 * Set our smart pointer to a specific heap container.  If
	 * we were pointing to something already we release that.
	 * @param object to reference.
	 */
	void set(Counted *object);

	/**
	 * Allocate memory from heap.  This may not be atomically aligned.
	 * The underlying alloc is larger to account for alignment.
	 * @param size of object to allocate.
	 * @return memory address allocated.
	 */
	static caddr_t alloc(size_t size);

	/**
	 * Adjust memory pointer to atomic boundry.
	 * @param address that was allocated.
	 * @return address for actual atomic aligned object.
	 */
	static caddr_t mem(caddr_t address);

public:
	/**
	 * Destroy pointer when falling out of scope.  This de-references
	 * the heap container.
	 */
	virtual ~TypeRef();

	/**
	 * Set our smart pointer based on another pointer instance.  If
	 * we are already referencing, we release the current container.
	 * @param pointer instance to share reference with.
	 */
	void set(const TypeRef& pointer);

	/**
	 * Manually release the current container.
	 */
	void clear(void);

	/**
	 * Get size of referenced heap object.
	 * @return size of container or 0 if none.
	 */
	size_t size(void) const;

	/**
	 * Get number of references to container.
	 * @return total number of pointers referencing container.
	 */	
	unsigned copies() const;
	
	/**
	 * Check if pointer currently has a heap container.
	 * @return true if we are referencing a container.
	 */
	inline operator bool() const {
		return ref != NULL;
	}

	/**
	 * Check if we are currently not pointing to anything.
	 * @return true if not referencing a container.
	 */
	inline bool operator!() const {
		return ref == NULL;
	}

	/**
	 * Special weak-public means to copy a container reference.
	 * This uses the base class container which is not public, so
	 * only derived type specific smart pointers can actually use 
	 * this method.  It is made public because making it protected
	 * actually makes it inaccessible to template derived classes.
	 * @param target smart pointer object to set.
	 * @param object to have it reference.
	 */
	inline static void put(TypeRef& target, Counted *object) {
		target.set(object);
	}

	TypeRelease *autorelease(TypeRelease *to);
};

class __EXPORT TypeRelease
{
public:
	inline TypeRelease() {}

protected:
	friend class TypeRef::Counted;

	virtual void dealloc(TypeRef::Counted *obj);

	inline size_t size(TypeRef::Counted *obj) {
		return obj->size;
	}

	inline void set(TypeRef::Counted *obj, TypeRef::Counted *ptr) {
		obj->link = ptr;
	}

	inline TypeRef::Counted *get(TypeRef::Counted *obj) {
		return obj->link;
	}

	inline void clear(TypeRef::Counted *obj) {
		obj->link = nullptr;
	}

private:
	__DELETE_COPY(TypeRelease);

};

class __EXPORT TypeSecure : private TypeRelease
{
private:
	virtual void dealloc(TypeRef::Counted *obj) __FINAL;

public:
	inline TypeSecure() {}
};

extern __EXPORT TypeRelease auto_release;
extern __EXPORT TypeSecure secure_release;

template<typename T, TypeRelease& R = auto_release>
class typeref : public TypeRef
{
private:
	class value : public Counted
	{
	private:
		__DELETE_COPY(value);

	public:
		T data;

		inline value(caddr_t mem, const T& object, TypeRelease *ar = &R) : 
		Counted(mem, sizeof(value), ar) {
			data = object;
		}
	};
 
public:
	inline typeref() :	TypeRef() {}

	inline typeref(const typeref& copy) : TypeRef(copy) {}

	inline typeref(const T& object, TypeRelease *ar = &R) : TypeRef() {
		caddr_t p = TypeRef::alloc(sizeof(value));
		TypeRef::set(new(mem(p)) value(p, object, ar)); 
	}

	inline explicit typeref(Counted *object) : TypeRef(object) {}

	inline const T* operator->() const {
		if(!ref)
			return NULL;
		value *v = polystatic_cast<value *>(ref);
		return &(v->data);
	}

	inline const T& operator*() const {
		value *v = polystatic_cast<value*>(ref);
		__THROW_DEREF(v);
		return *(&(v->data));
	}

	inline const T* operator()() const {
		value *v = polystatic_cast<value*>(ref);
		if(!v)
			return nullptr;

		return &(v->data);
	}

	inline operator const T&() const {
		value *v = polystatic_cast<value*>(ref);
		__THROW_DEREF(v);
		return *(&(v->data));
	}

	inline typeref& operator=(const typeref& ptr) {
		TypeRef::set(ptr);
		return *this;
	}

	inline bool operator==(const typeref& ptr) const {
		value *v1 = polystatic_cast<value*>(ref);
		value *v2 = polystatic_cast<value*>(ptr.ref);
		if(!v1 || !v2)
			return false;
		return v1->data == v2->data;
	}	

	inline bool operator==(const T& obj) const {
		value *v = polystatic_cast<value *>(ref);
		if(!v)
			return false;
		return v->data == obj;
	}

	inline bool operator!=(const typeref& ptr) const {
		return !(*this == ptr);
	}

	inline bool operator!=(const T& obj) const {
		return !(*this == obj);
	}

	inline void set(T& object, TypeRelease *pool = &R) {
		clear();
		caddr_t p = TypeRef::alloc(sizeof(value));
		TypeRef::set(new(mem(p)) value(p, object, pool));
	}

	inline typeref& operator=(T& object) {
		set(object);
		return *this;
	}
};

// The specializations are done as simple template specializations so that the
// hard parts can be hard-coded rather than inline members.  This means we do
// not pass the autorelease as a specialization here, but we can do a secondary
// template that does use releases with a lot less overhead.

template<>
class __EXPORT typeref<const char *> : public TypeRef
{
public:
	class value : public Counted
	{
	private:
		__DELETE_COPY(value);

	protected:
		friend class typeref;

		char mem[1];

		value(caddr_t addr, size_t size, const char *str, TypeRelease *ar = nullptr);

		void destroy(void);

	public:
		inline char *get() {
			return &mem[0];
		}

		inline size_t len() {
			return strlen(mem);
		}

		inline size_t max() {
			return size;
		}

		inline operator char *() {
			return &mem[0];
		}
	};

	typeref();
	
	typeref(const typeref& copy);

	typeref(const char *str, TypeRelease *ar = nullptr);

	typeref(size_t size, TypeRelease *ar = nullptr);

	inline explicit typeref(Counted *object) : TypeRef(object) {}

	inline explicit typeref(value *value) : TypeRef(value) {}

	const char *operator*() const;

	inline operator const char *() const {
		return operator*();
	}

	size_t len() const;

	bool operator==(const typeref& ptr) const;

	bool operator==(const char *obj) const;

	bool operator==(value *chars) const;

	inline bool operator!=(const typeref& ptr) const {
		return !(*this == ptr);
	}

	inline bool operator!=(value *chars) const {
		return !(*this == chars);
	}

	inline bool operator!=(const char *obj) const {
		return !(*this == obj);
	}

	bool operator<(const typeref& ptr) const;

	inline bool operator>(const typeref& ptr) const {
		return (ptr < *this);
	}

	inline bool operator<=(const typeref& ptr) const {
		return !(*this > ptr);
	}

	inline bool operator>=(const typeref& ptr) const {
		return !(*this < ptr);
	}

	typeref& operator=(const typeref& objref);

	typeref& operator=(const char *str);

	typeref& operator=(value *chars);

	const typeref operator+(const char *str) const;

	const typeref operator+(const typeref& ptr) const;

	const char *operator()(ssize_t offset) const;

	void set(const char *str, TypeRelease *ar = nullptr);

	void hex(const uint8_t *mem, size_t size, TypeRelease *ar = nullptr);

	void b64(const uint8_t *mem, size_t size, TypeRelease *ar = nullptr);

	void assign(value *chars);

	static void expand(value **handle, size_t size);

	static value *create(size_t size, TypeRelease *ar = nullptr);

	static void destroy(value *bytes);
};

template<>
class __EXPORT typeref<const uint8_t *> : public TypeRef
{
public:
	class value : public Counted
	{
	private:
		__DELETE_COPY(value);

	protected:
		friend class typeref;

		uint8_t mem[1];

		value(caddr_t addr, size_t size, const uint8_t *data = nullptr, TypeRelease *ar = nullptr);

		void destroy(void);

	public:
		inline size_t max() {
			return size;
		}

		inline uint8_t *get() {
			return &mem[0];
		}

		inline operator uint8_t*() {
			return &mem[0];
		}
	};

	typeref();
	
	typeref(const typeref& copy);

	typeref(uint8_t *str, size_t size, TypeRelease *ar = nullptr);

	typeref(bool mode, size_t bits, TypeRelease *ar = nullptr);

	inline explicit typeref(Counted *object) : TypeRef(object) {}

	const uint8_t *operator*() const;

	inline operator const uint8_t *() const {
		return operator*();
	}

	typeref& operator=(const typeref& objref);

	typeref& operator=(value *bytes);

	bool operator==(const typeref& ptr) const;

	bool operator==(value *bytes) const;

	inline bool operator!=(const typeref& ptr) const {
		return !(*this == ptr);
	}

	inline bool operator!=(value *bytes) const {
		return !(*this == bytes);
	}

	const typeref operator+(const typeref& ptr) const;

	void set(const uint8_t *str, size_t size, TypeRelease *ar = nullptr);

	size_t set(bool bit, size_t offset, size_t bits = 1);

	size_t hex(const char *str, bool ws = false, TypeRelease *ar = nullptr);

	size_t b64(const char *str, bool ws = false, TypeRelease *ar = nullptr);

	uint8_t *data(void);

	bool get(size_t offset);

	size_t count(size_t offset, size_t bits = 1);	

	void assign(value *bytes);

	typeref<const char *> hex();

	typeref<const char *> b64();

	static value *create(size_t size, TypeRelease *ar = nullptr);

	static void destroy(value *bytes);
};

// convenience classes that roll up autorelease behavior for strings and
// byte arrays into templates.

template<TypeRelease& R>
class stringref : public typeref<const char *>
{
public:
	inline stringref() : typeref<const char *>() {} 
	
	inline stringref(const stringref& copy) : typeref<const char *>(copy) {}

	inline stringref(const char *str) : typeref<const char *>(str, &R) {}

	inline stringref(size_t size) : typeref<const char *>(size, &R) {}

	inline explicit stringref(Counted *object) : typeref<const char *>(object) {}

	inline void set(const char *str) {
		typeref<const char *>::set(str, &R);
	}

	inline static value *create(size_t size) {
		return typeref<const char *>::create(size, &R);
	}
};

template<TypeRelease& R>
class byteref : public typeref<const uint8_t *>
{
public:
	inline byteref() : typeref<const uint8_t *>() {}

	inline byteref(uint8_t *str, size_t size) : typeref<const uint8_t *>(str, size, &R) {}

	inline byteref(bool mode, size_t bits) : typeref<const uint8_t *>(mode, bits, &R) {}

	inline explicit byteref(Counted *object) : typeref<const uint8_t *>(object) {}

	inline void set(const uint8_t *str, size_t size) {
		typeref<const uint8_t *>::set(str, size, &R);
	}

	inline size_t hex(const char *str, bool ws = false) {
		return typeref<const uint8_t *>::hex(str, ws, &R);
	}

	inline size_t b64(const char *str, bool ws = false) {
		return typeref<const uint8_t *>::b64(str, ws, &R);
	}

	inline stringref<R> hex() {
		typeref<const char *> str = typeref<const uint8_t *>::hex();
		stringref<R> result = *str;
		return result;
	}

	inline stringref<R> b64() {
		typeref<const char *> str = typeref<const uint8_t *>::b64();
		stringref<R> result = *str;
		return result;
	}

	inline static value *create(size_t size) {
		return typeref<const uint8_t *>::create(size, &R);
	}
};

// a namespace for aliasing things we may typically use as a typeref

namespace Type {
	typedef int32_t Integer;
	typedef double Real;
	typedef const char *Chars;
	typedef const uint8_t *Bytes;
	typedef const uint8_t *Bools;
}

typedef typeref<Type::Chars>::value *charvalues_t;
typedef	typeref<Type::Bytes>::value *bytevalues_t;
typedef	typeref<Type::Chars> stringref_t;
typedef typeref<Type::Bytes> byteref_t;
typedef typeref<Type::Bools> boolref_t;

template<typename T>
inline typeref<T> typeref_cast(T x) {
	return typeref<T>(x);
}

} // namespace

#endif
