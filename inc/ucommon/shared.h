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

#ifndef _UCOMMON_SOCKET_H_
#include <ucommon/socket.h>
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

class __EXPORT MappedPointer
{
private:
	inline MappedPointer(const MappedPointer& copy) {};

protected:
	class __EXPORT Index : public LinkedObject
	{
	public:
		explicit Index(LinkedObject **origin);

		const void *key;
		void *value;
	};

	condlock_t *lock;

	LinkedObject *free, **list;

	memalloc pager;

	size_t paths;

	MappedPointer(size_t indexes, condlock_t *locking = NULL, size_t paging = 0);
	~MappedPointer();

	LinkedObject *access(size_t path);

	LinkedObject *modify(size_t path);

	void release(void *obj);

	void insert(const void *key, void *value, size_t path);

	void replace(Index *ind, void *value);

	void remove(Index *ind, size_t path);

public:
	static size_t keypath(const uint8_t *addr, size_t size);
};

template<typename T>
inline size_t mapped_keypath(const T *addr)
{
	if(!addr)
		return 0;

	return MappedPointer::keypath((const uint8_t *)addr, sizeof(T)); 
}

template<typename T>
inline bool mapped_keyequal(const T* key1, const T* key2)
{
	if(!key1 || !key2)
		return false;
	return !memcmp(key1, key2, sizeof(T));
}

template<>
inline size_t mapped_keypath<char>(const char *addr)
{
	if(!addr)
		return 0;

	return MappedPointer::keypath((const uint8_t *)addr, strlen(addr));
}

template<>
inline bool mapped_keyequal<char>(const char *k1, const char *k2)
{
	if(!k1 || !k2)
		return false;

	return eq(k1, k2);
}

template<>
inline size_t mapped_keypath<struct sockaddr>(const struct sockaddr *addr)
{
	if(!addr)
		return 0;

	return MappedPointer::keypath((const uint8_t *)addr, Socket::len(addr));
}

template<>
inline bool mapped_keyequal<struct sockaddr>(const struct sockaddr *s1, const struct sockaddr *s2)
{
	if(!s1 || !s2)
		return false;
	return Socket::equal(s1, s2);
}

template<typename K, typename V>
class mapped_pointer : public MappedPointer
{
public:
	inline mapped_pointer(size_t indexes = 37, condlock_t *locking = NULL, size_t paging = 0) : MappedPointer(indexes, locking, paging) {}

	inline void release(V* object) {
		MappedPointer::release(object);
	}

	void remove(const K* key) {
		size_t path = mapped_keypath<K>(key);
		linked_pointer<Index> ip = modify(path);
		while(is(ip)) {
			if(mapped_keyequal<K>((const K*)(ip->key), key)) {
				MappedPointer::remove(*ip, path);
				return;
			}
			ip.next();
		}
		lock->commit();
	}

	V* get(const K* key) {
		linked_pointer<Index> ip = access(mapped_keypath<K>(key));
		while(is(ip)) {
			if(mapped_keyequal<K>((const K*)(ip->key), key)) {
				return static_cast<V*>(ip->value);
			}
			ip.next();
		}
		lock->release();
		return nullptr;
	}

	void set(const K* key, V* ptr) {
		size_t path = mapped_keypath<K>(key);
		linked_pointer<Index> ip = modify(path);
		while(is(ip)) {
			if(mapped_keyequal<K>((const K*)(ip->key), key)) {
				replace(*ip, ptr);
				return;
			}
		}
		insert((const void *)key, (void *)ptr, path);
	}
};

} // namespace

#endif
