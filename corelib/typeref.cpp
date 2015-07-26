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

#include <ucommon-config.h>
#include <ucommon/export.h>
#include <ucommon/typeref.h>
#include <ucommon/string.h>
#include <cstdlib>

namespace ucommon {

TypeCounted::TypeCounted(void *addr, size_t size) : 
ObjectProtocol()
{
    this->memory = addr;
    this->size = size;
}

void TypeCounted::dealloc()
{
    delete this;
    ::free(memory);
}

void TypeCounted::operator delete(void *addr)
{
}

void TypeCounted::retain(void)
{
    count.fetch_add();
}

void TypeCounted::release(void)
{
    if(count.fetch_sub() == 1) {
	dealloc();
    }
}

TypeRef::TypeRef()
{
    ref = NULL;
}

TypeRef::TypeRef(TypeCounted *object)
{
    ref = object;
    object->retain();
}

TypeRef::TypeRef(const TypeRef& copy)
{
    ref = copy.ref;
    ref->retain();
}

TypeRef::~TypeRef()
{
    release();
}

void TypeRef::release(void)
{
    if(ref)
    	ref->release();
    ref = NULL;
}

void TypeRef::set(const TypeRef& ptr)
{
    if(ptr.ref)
    	ptr.ref->retain();
    release();
    ref = ptr.ref;
}

void TypeRef::set(TypeCounted *object)
{
    object->retain();
    release();
    ref = object;
}

caddr_t TypeRef::alloc(size_t size)
{
    return (caddr_t)::malloc(size + 16);
}

caddr_t TypeRef::mem(caddr_t addr)
{
    while(((uintptr_t)addr) & 0xf)
        ++addr;
    return addr;
}

stringref::value::value(caddr_t addr, size_t size, const char *str) : 
TypeCounted(addr, size)
{
    if(str)
    	String::set(mem, size + 1, str);
    else
	mem[0] = 0;
}

stringref::stringref() :
TypeRef() {}

stringref::stringref(const stringref& copy) :
TypeRef(copy) {}

stringref::stringref(const char *str) :
TypeRef()
{
    size_t size = 0;
    
    if(str)
        size = strlen(str);

    caddr_t p = TypeRef::alloc(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, str));
}

const char *stringref::operator*() const 
{
    if(!ref)
	return NULL;
    value *v = polystatic_cast<value *>(ref);
    return &v->mem[0];
}

stringref& stringref::operator=(const stringref& objref)
{
    TypeRef::set(objref);
    return *this;
}

stringref& stringref::operator=(const char *str)
{
    set(str);
    return *this;
}

void stringref::set(const char *str)
{
    release();
    size_t size = 0;

    if(str)
        size = strlen(str);

    caddr_t p = TypeRef::alloc(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, str));
}


} // namespace
