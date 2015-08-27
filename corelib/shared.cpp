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

#include <ucommon-config.h>
#include <ucommon/export.h>
#include <ucommon/typeref.h>
#include <ucommon/string.h>
#include <ucommon/thread.h>
#include <ucommon/shared.h>
#include <cstdlib>

namespace ucommon {

SharedRef::SharedRef() : TypeRef()
{
}

TypeRef SharedRef::get()
{
	lock.acquire();
	TypeRef ptr(ref);
	lock.release();
	return ptr;
}

void SharedRef::get(TypeRef& ptr)
{
	lock.acquire();
	Counted *old = ref;
	ref = ptr.ref;
	if(ref)
		ref->retain();
	lock.release();
	if(old)
		old->release();
}

void SharedRef::put(TypeRef& ptr)
{
	lock.acquire();
	ptr.ref = ref;
	lock.release();
}

MappedPointer::Index::Index(LinkedObject **origin) :
LinkedObject(origin)
{
	key = value = NULL;
}

MappedPointer::MappedPointer(size_t indexes, condlock_t *locking, size_t paging) : pager(paging)
{
	caddr_t p;
	if(!locking) {
		p = (caddr_t)pager.alloc(sizeof(condlock_t));
		locking = new(p) condlock_t;
	}
	lock = locking;

	list = (LinkedObject **)pager.alloc(sizeof(LinkedObject *) * indexes);
	free = NULL;
	paths = 0;
	while(paths < indexes)
		list[paths++] = NULL;
}

MappedPointer::~MappedPointer()
{
	pager.purge();
}	

LinkedObject *MappedPointer::access(size_t path)
{
	lock->access();
	return list[path % paths];
}

LinkedObject *MappedPointer::modify(size_t path)
{
	lock->modify();
	return list[path % paths];
}

void MappedPointer::release(void *object)
{
	if(object != nullptr)
		lock->release();
}

void MappedPointer::replace(Index *ind, void *object)
{
	ind->value = object;
	lock->commit();
}

void MappedPointer::remove(Index *ind, size_t path)
{
	LinkedObject **root = &list[path % paths];
	ind->delist(root);
	ind->enlist(&free);
	ind->key = ind->value = NULL;
	lock->commit();
}

void MappedPointer::insert(const void *key, void *value, size_t path)
{
	caddr_t p = (caddr_t)(free);
	if(free)
		free = free->getNext();
	else 
		p = (caddr_t)pager.alloc(sizeof(Index));

	Index *ind = new(p) Index(&list[path % paths]);
	ind->key = key;
	ind->value = value;
	lock->commit();
}	

size_t MappedPointer::keypath(const uint8_t *addr, size_t size)
{
	size_t value = size;
	while(size--) {
		value = (value << 3) ^ *(addr++);
	}
	return value;
}

} // namespace
