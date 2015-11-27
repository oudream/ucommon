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
#include <cstdlib>

namespace ucommon {

TypeRef::Counted::Counted(void *addr, size_t objsize, TypeRelease *ar) : 
ObjectProtocol()
{
    this->offset = (unsigned)((char *)this - (char *)addr);
    this->size = objsize;
    this->autorelease = ar;
}

void TypeRef::Counted::dealloc()
{
    TypeRelease *rel = autorelease;
    if(rel) {
        autorelease = nullptr;
        rel->dealloc(this);
        return;
    }
        
    void *memory = (void *)((char *)this - offset);
    delete this;
    ::free(memory);
}

void TypeRef::Counted::operator delete(void *addr)
{
}

void TypeRef::Counted::retain(void)
{
    count.fetch_retain();
}

void TypeRef::Counted::release(void)
{
    if(count.fetch_release() < 2) {
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
    clear();
}

void TypeRef::clear(void)
{
    if(ref)
    	ref->release();
    ref = NULL;
}

TypeRelease *TypeRef::autorelease(TypeRelease *rel)
{
    if(!ref)
        return nullptr;

    TypeRelease *tmp = ref->autorelease;
    ref->autorelease = rel;
    return tmp;
}

void TypeRef::set(const TypeRef& ptr)
{
    if(ptr.ref)
    	ptr.ref->retain();
    clear();
    ref = ptr.ref;
}

void TypeRef::set(TypeRef::Counted *object)
{
    if(object)
        object->retain();
    clear();
    ref = object;
}

caddr_t TypeRef::mem(caddr_t addr)
{
    size_t mask = (Thread::cache() - 1);
    while(((uintptr_t)addr) & mask)
        ++addr;
    return addr;
}

typeref<const char *>::value::value(caddr_t addr, size_t objsize, const char *str, TypeRelease *ar) : 
TypeRef::Counted(addr, objsize, ar)
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

typeref<const char *>::typeref(const char *str, TypeRelease *ar) :
TypeRef()
{
    size_t size = 0;
    
    if(str)
        size = strlen(str);

    caddr_t p = ar->allocate(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, str, ar));
}

typeref<const char *>::typeref(size_t size, TypeRelease *ar) :
TypeRef()
{
    caddr_t p = ar->allocate(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, "", ar));
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
    value *v = polystatic_cast<value *>(ref);
    if(!v)
        return NULL;

    return &v->mem[0];
}

size_t typeref<const char *>::len() const
{
    value *v = polystatic_cast<value *>(ref);
    if(!v)
        return 0;

    return v->len();
}

const typeref<const char *> typeref<const char *>::operator+(const char *str2) const 
{
    value *v1 = polystatic_cast<value *>(ref);
    const char *str1 = "";
    TypeRelease *ar = nullptr;

    if(v1) {
        str1 = &v1->mem[0];
        ar = v1->autorelease;
    }

    if(!str2)
        str2 = "";

    size_t ss = strlen(str1);
    ss += strlen(str2);
    charvalues_t results = create(ss, ar);
    snprintf(results->get(), results->max() + 1, "%s%s", str1, str2);
    stringref_t result;
    result.assign(results);
    return result;
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

void typeref<const char *>::set(const char *str, TypeRelease *ar)
{
    clear();
    size_t size = 0;

    if(str)
        size = strlen(str);

    caddr_t p = ar->allocate(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, str, ar));
}

void typeref<const char *>::b64(const uint8_t *bytes, size_t bsize, TypeRelease *ar)
{
    clear();
    size_t len = String::b64size(bsize);

    caddr_t p = ar->allocate(sizeof(value) + len);
    value *s = new(mem(p)) value(p, len, "", ar);
    String::b64encode(&s->mem[0], bytes, bsize);
    TypeRef::set(s);
}

void typeref<const char *>::hex(const uint8_t *bytes, size_t bsize, TypeRelease *ar)
{
    clear();
    size_t len = bsize * 2;

    caddr_t p = ar->allocate(sizeof(value) + len);
    value *s = new(mem(p)) value(p, len, "", ar);

    for(size_t index = 0; index < bsize; ++index) {
        snprintf(&s->mem[index * 2], 3, "%2.2x", bytes[index]);
    }

    TypeRef::set(s);
}

void typeref<const char *>::assign(value *chars)
{
    clear();
    chars->size = strlen(chars->mem);
    TypeRef::set(chars);
}

typeref<const char *>& typeref<const char *>::operator=(value *chars)
{
    assign(chars);
    return *this;
}

typeref<const char *>::value *typeref<const char *>::create(size_t size, TypeRelease *ar)
{
    caddr_t p = ar->allocate(sizeof(value) + size);
    return new(mem(p)) value(p, size, NULL, ar);
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
    
typeref<const uint8_t *>::value::value(caddr_t addr, size_t objsize, const uint8_t *str, TypeRelease *ar) : 
TypeRef::Counted(addr, objsize, ar)
{
    if(objsize && str)
        memcpy(mem, str, objsize);
}

typeref<const uint8_t *>::typeref() :
TypeRef() {}

typeref<const uint8_t *>::typeref(const typeref<const uint8_t *>& copy) :
TypeRef(copy) {}

typeref<const uint8_t *>::typeref(uint8_t *str, size_t size, TypeRelease *ar) :
TypeRef()
{
    caddr_t p = ar->allocate(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, str, ar));
}

typeref<const uint8_t *>::typeref(bool mode, size_t bits, TypeRelease *ar) :
TypeRef()
{
    size_t size = (bits / 8);
    if(bits % 8)
        ++size;

    caddr_t p = ar->allocate(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, nullptr, ar));
    set(mode, 0, bits);
}

void typeref<const uint8_t *>::value::destroy(void) 
{
	count.clear();
	release();
}

typeref<const uint8_t *>& typeref<const uint8_t *>::operator=(const typeref<const uint8_t *>& objref)
{
    TypeRef::set(objref);
    return *this;
}

typeref<const uint8_t *>& typeref<const uint8_t *>::operator=(value *bytes)
{
    assign(bytes);
    return *this;
}

void typeref<const uint8_t *>::set(const uint8_t *str, size_t size, TypeRelease *ar)
{
    clear();
    caddr_t p = ar->allocate(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, str, ar));
}

size_t typeref<const uint8_t *>::hex(const char *str, bool ws, TypeRelease *ar)
{
    clear();
    size_t size = String::hexcount(str, ws);
    if(!size)
        return 0;

    caddr_t p = ar->allocate(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, nullptr, ar));
    String::hex2bin(str, data(), size, ws);
    return size;
}

size_t typeref<const uint8_t *>::b64(const char *str, bool ws, TypeRelease *ar)
{
    clear();
    size_t size = String::b64count(str, ws);
    if(!size)
        return 0;

    caddr_t p = ar->allocate(sizeof(value) + size);
    TypeRef::set(new(mem(p)) value(p, size, nullptr, ar));
    String::b64decode(data(), str, size, ws);
    return size;
}

const uint8_t *typeref<const uint8_t *>::operator*() const
{
    if(!ref)
	    return NULL;
    value *v = polystatic_cast<value *>(ref);
    return &v->mem[0];
}

uint8_t *typeref<const uint8_t *>::data()
{
    if(!ref)
	    return NULL;
    value *v = polystatic_cast<value *>(ref);
    return &v->mem[0];
}

void typeref<const uint8_t *>::assign(value *bytes)
{
    clear();
    TypeRef::set(bytes);
}

typeref<const uint8_t *>::value *typeref<const uint8_t *>::create(size_t size, TypeRelease *ar)
{
    caddr_t p = ar->allocate(sizeof(value) + size);
    return new(mem(p)) value(p, size, nullptr, ar);
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

typeref<const char *> typeref<const uint8_t *>::hex()
{
    typeref<const char *>str;
    if(ref) {
        value *v = polystatic_cast<value*>(ref);
        str.hex(&v->mem[0], v->size, ref->getRelease());
    }
    return str;
}

typeref<const char *> typeref<const uint8_t *>::b64()
{
    typeref<const char *>str;
    if(ref) {
        value *v = polystatic_cast<value*>(ref);
        str.b64(&v->mem[0], v->size, ref->getRelease());
    }
    return str;
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

const typeref<const uint8_t *> typeref<const uint8_t *>::operator+(const typeref<const uint8_t *>&add) const 
{
    value *v1 = polystatic_cast<value *>(ref);
    value *v2 = polystatic_cast<value *>(add.ref);
    const uint8_t *b1 = NULL, *b2 = NULL;
    uint8_t *out;
    size_t s1 = 0, s2 = 0, max;
    typeref<const uint8_t*> result;
    TypeRelease *ar = nullptr;

    if(v1) {
        s1 = v1->max();
        b1 = v1->get();
        ar = v1->autorelease;
    }

    if(v2) {
        s2 = v2->max();
        b2 = v2->get();
    }

    max = s1 + s2;
    if(!max)
        return result;

    bytevalues_t bytes = create(max, ar);
    out = const_cast<uint8_t *>(bytes->get());
    if(s1)
        memcpy(out, b1, s1);
    if(s2)
        memcpy(out + s1, b2, s2);
    result.assign(bytes);
    return result;
}

bool typeref<const uint8_t *>::get(size_t offset)
{
    uint8_t mask = 1;
    value *v = polystatic_cast<value *>(ref);
    if(!v || v->size < offset / 8)
        return false;

    mask = mask << offset % 8;
    return (((v->get())[offset / 8] & mask) != 0);
}

size_t typeref<const uint8_t *>::count(size_t offset, size_t bits)
{
    uint8_t mask = 1;
    size_t total = 0;

    value *v = polystatic_cast<value *>(ref);
    if(!v)
        return 0;

    uint8_t *data = v->get();
    while(bits--) {
        size_t pos = offset / 8;
        if(pos >= v->size)
            break;
        uint8_t bitmask = mask << (offset % 8);
        ++offset;
        if(data[pos] & bitmask)
            ++total;
    }
    return total;
}

size_t typeref<const uint8_t *>::set(bool mode, size_t offset, size_t bits)
{
    uint8_t mask = 1;
    size_t total = 0;

    value *v = polystatic_cast<value *>(ref);
    if(!v)
        return 0;

    uint8_t *data = v->get();
    while(bits--) {
        size_t pos = offset / 8;
        if(pos >= v->size)
            break;
        uint8_t bitmask = mask << (offset % 8);
        ++offset;
        bool current = ((data[pos] & bitmask) != 0);
        if(current != mode)
            ++total;
        else
            continue;

        if(mode)
            data[pos] |= bitmask;
        else
            data[pos] &= ~bitmask;
    }
    return total;
}

caddr_t TypeRelease::allocate(size_t size)
{
    return (caddr_t)::malloc(size + Thread::cache());
}

void TypeRelease::release(TypeRef::Counted *obj)
{
    obj->autorelease = nullptr;
    obj->dealloc();
}

void TypeRelease::purge()
{
    if(delegate)
        delegate->purge();
}

void TypeRelease::enlist(TypeRef::Counted **root, TypeRef::Counted *obj)
{
    obj->link = *root;
    *root = obj;
}

TypeRef::Counted *TypeRelease::delist(TypeRef::Counted **root)
{
    TypeRef::Counted *obj = *root;
    if(obj)
        *root = obj->link;
    else
        *root = nullptr;
    return obj;
}

void TypeRelease::dealloc(TypeRef::Counted *obj)
{
    if(delegate)
        delegate->release(obj);
    else
        release(obj);
} 

class __LOCAL TypeSecure __FINAL : public TypeRelease
{
private:
	void release(TypeRef::Counted *obj) __FINAL;

public:
	inline TypeSecure() {}
};

void TypeSecure::release(TypeRef::Counted *obj)
{
    char *addr = (char *)obj + sizeof(TypeRef::Counted);
    size_t size = TypeRelease::size(obj);
    memset(addr, 0, size);
    TypeRelease::release(obj);
}

static TypeSecure _secure_release;

TypeRelease auto_release;
TypeRelease secure_release(&_secure_release);

} // namespace
