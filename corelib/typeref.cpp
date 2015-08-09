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

TypeRef::Counted::Counted(void *addr, size_t objsize) : 
ObjectProtocol()
{
    this->memory = addr;
    this->size = objsize;
}

void TypeRef::Counted::dealloc()
{
    delete this;
    ::free(memory);
}

void TypeRef::Counted::operator delete(void *addr)
{
}

void TypeRef::Counted::retain(void)
{
    count.fetch_add();
}

void TypeRef::Counted::release(void)
{
    if(count.fetch_sub() < 2) {
	    dealloc();
    }
}

TypeRef::TypeRef()
{
    ref = NULL;
}

TypeRef::TypeRef(TypeRef::Counted *object)
{
    ref = object;
    if(ref)
        ref->retain();
}

TypeRef::TypeRef(const TypeRef& copy)
{
    ref = copy.ref;
    if(ref)
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

void TypeRef::set(TypeRef::Counted *object)
{
    if(object)
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

typeref<const char *>::value::value(caddr_t addr, size_t objsize, const char *str) : 
TypeRef::Counted(addr, objsize)
{
    if(str)
    	String::set(mem, objsize + 1, str);
    else
	    mem[0] = 0;
}

void typeref<const char *>::value::destroy(void) 
{
	count.clear();
	release();
}

typeref<const char *>::typeref() :
TypeRef() {}

typeref<const char *>::typeref(const typeref<const char *>& copy) :
TypeRef(copy) {}

typeref<const char *>::typeref(const char *str) :
TypeRef()
{
    size_t size = 0;
    
    if(str)
        size = strlen(str);

    caddr_t p = TypeRef::alloc(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, str));
}

const char *typeref<const char *>::operator()(ssize_t offset) const
{
    value *v = polystatic_cast<value *>(ref);
    if(!v)
        return NULL;

    if(offset < 0 && offset < -((ssize_t)v->len()))
        return NULL;

    if(offset < 0)
        return &v->mem[v->len() + offset];

    if(offset > (ssize_t)v->len())
        return NULL;

    return &v->mem[v->len() + offset];
}

const char *typeref<const char *>::operator*() const 
{
    if(!ref)
	    return NULL;
    value *v = polystatic_cast<value *>(ref);
    return &v->mem[0];
}

typeref<const char *>& typeref<const char *>::operator=(const typeref<const char *>& objref)
{
    TypeRef::set(objref);
    return *this;
}

typeref<const char *>& typeref<const char *>::operator=(const char *str)
{
    set(str);
    return *this;
}

void typeref<const char *>::set(const char *str)
{
    release();
    size_t size = 0;

    if(str)
        size = strlen(str);

    caddr_t p = TypeRef::alloc(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, str));
}

void typeref<const char *>::assign(value *chars)
{
    release();
    chars->size = strlen(chars->mem);
    TypeRef::set(chars);
}

typeref<const char *>& typeref<const char *>::operator=(value *chars)
{
    assign(chars);
    return *this;
}

typeref<const char *>::value *typeref<const char *>::create(size_t size)
{
    caddr_t p = TypeRef::alloc(sizeof(value) + size);
    return new(mem(p)) value(p, size, NULL);
}

void typeref<const char *>::destroy(typeref<const char *>::value *chars)
{
    if(chars)
        chars->destroy();
}

void typeref<const char *>::expand(typeref<const char *>::value **handle, size_t size)
{
    if(!handle || !*handle)
        return;

    typeref<const char *>::value *change = create(size + (*handle)->max());
    if(change)
        String::set(change->get(), change->max() + 1, (*handle)->get());
    destroy(*handle);
    *handle = change;
}

bool typeref<const char *>::operator==(const typeref<const char *>& ptr) const
{
    value *v1 = polystatic_cast<value*>(ref);
    value *v2 = polystatic_cast<value*>(ptr.ref);
    if(!v1 || !v2)
        return false;
    return eq(&(v1->mem[0]), &(v2->mem[0]));
}

bool typeref<const char *>::operator==(const char *obj) const
{
    value *v = polystatic_cast<value *>(ref);
    if(!v)
        return false;
    return eq(&(v->mem[0]), obj);
}

bool typeref<const char *>::operator==(value *chars) const
{
    value *v = polystatic_cast<value *>(ref);
    if(!v || !chars)
        return false;
    return eq(&(v->mem[0]), &(chars->mem[0]));
}

bool typeref<const char *>::operator<(const typeref<const char *>& ptr) const
{
    value *v1 = polystatic_cast<value *>(ref);
    value *v2 = polystatic_cast<value *>(ptr.ref);

    if(!v1 && v2)
        return true;

    if(!v1 && !v2)
        return true;

    if(v1 && !v2)
        return false;

#ifdef  HAVE_STRCOLL
    return strcoll(&(v1->mem[0]), &(v2->mem[0])) < 0;
#else
    return strcmp(&(v1->mem[0]), &(v2->mem[0])) < 0:
#endif
}
    
typeref<const uint8_t *>::value::value(caddr_t addr, size_t objsize, const uint8_t *str) : 
TypeRef::Counted(addr, objsize)
{
    if(objsize)
        memcpy(mem, str, objsize);
}

typeref<const uint8_t *>::value::value(caddr_t addr, size_t size) : 
TypeRef::Counted(addr, size)
{
}

typeref<const uint8_t *>::typeref() :
TypeRef() {}

typeref<const uint8_t *>::typeref(const typeref<const uint8_t *>& copy) :
TypeRef(copy) {}

typeref<const uint8_t *>::typeref(uint8_t *str, size_t size) :
TypeRef()
{
    caddr_t p = TypeRef::alloc(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, str));
}

void typeref<const uint8_t *>::value::destroy(void) 
{
	count.clear();
	release();
}

const uint8_t *typeref<const uint8_t *>::operator*() const 
{
    if(!ref)
	    return NULL;
    value *v = polystatic_cast<value *>(ref);
    return &v->mem[0];
}

typeref<const uint8_t *>& typeref<const uint8_t *>::operator=(const byteref& objref)
{
    TypeRef::set(objref);
    return *this;
}

typeref<const uint8_t *>& typeref<const uint8_t *>::operator=(value *bytes)
{
    assign(bytes);
    return *this;
}

void typeref<const uint8_t *>::set(const uint8_t *str, size_t size)
{
    release();
    caddr_t p = TypeRef::alloc(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, str));
}

void typeref<const uint8_t *>::assign(value *bytes)
{
    release();
    TypeRef::set(bytes);
}

typeref<const uint8_t *>::value *typeref<const uint8_t *>::create(size_t size)
{
    caddr_t p = TypeRef::alloc(sizeof(value) + size);
    return new(mem(p)) value(p, size);
}

void typeref<const uint8_t *>::destroy(typeref<const uint8_t *>::value *bytes)
{
    if(bytes)
        bytes->destroy();
}

size_t TypeRef::size(void) const
{
    if(!ref)
        return 0;

    return ref->size;
}

unsigned TypeRef::copies() const 
{
	if(!ref)
		return 0;
	return ref->copies();
}

bool typeref<const uint8_t *>::operator==(const typeref<const uint8_t *>& ptr) const 
{
    value *v1 = polystatic_cast<value*>(ref);
    value *v2 = polystatic_cast<value*>(ptr.ref);
    if(!v1 || !v2 || v1->size != v2->size)
        return false;
    return !memcmp(&(v1->mem[0]), &(v2->mem[0]), v1->size);
}

bool typeref<const uint8_t *>::operator==(value *bytes) const 
{
    value *v = polystatic_cast<value *>(ref);
    if(!v || !bytes || v->size != bytes->size)
        return false;
    return !memcmp(&(v->mem[0]), &(bytes->mem[0]), v->size);
}

} // namespace
