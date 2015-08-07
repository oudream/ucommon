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
#include <ucommon/arrayref.h>
#include <ucommon/string.h>
#include <cstdlib>

namespace ucommon {

ArrayRef::Array::Array(void *addr, size_t used) :
Counted(addr, used)
{
    size_t index = 0;
    Counted **list = get();

    if(!used)
        return;

    while(index < used) {
        list[index++] = NULL;
    }
}

void ArrayRef::Array::dealloc()
{
    size_t index = 0;
    Counted **list = get();

    if(!size)
        return;

    while(index < size) {
        Counted *object = list[index];
        if(object) {
            object->release();
            list[index] = NULL;
        }
        ++index;
    }
    size = 0;
    Counted::dealloc();
}

TypeRef::Counted *ArrayRef::Array::get(size_t index)
{
    if(index >= size)
        return NULL;

    return (get())[index];
}

void ArrayRef::Array::assign(size_t index, Counted *object)
{
    if(index >= size)
        return;
    
    if(object)
        object->retain();

    Counted *replace = get(index);
    if(replace)
        replace->release();

    (get())[index] = object;
}   

ArrayRef::ArrayRef() :
TypeRef()
{
}

ArrayRef::ArrayRef(size_t size) :
TypeRef(create(size))
{
} 

ArrayRef::ArrayRef(const ArrayRef& copy) :
TypeRef(copy)
{
}

void ArrayRef::reset(Counted *object)
{
    size_t index = 0;
    Array *array = polystatic_cast<Array *>(ref);

    if(!array || !array->size || !object)
        return;

    while(index < array->size) {
        array->assign(index++, object);
    }
}

void ArrayRef::reset(TypeRef& var)
{
    reset(var.ref);
}

ArrayRef::Array *ArrayRef::create(size_t size)
{
    if(!size)
        return NULL;

    size_t s = sizeof(Array) + (size * sizeof(Counted *));
    caddr_t p = TypeRef::alloc(s);
    return new(mem(p)) Array(p, size);
}

void ArrayRef::assign(size_t index, TypeRef& t)
{
    Array *array = polystatic_cast<Array *>(ref);
    if(!array || index >= array->size)
        return;

    Counted *obj = t.ref;
    array->assign(index, obj);
}

void ArrayRef::resize(size_t size)
{
    Array *array = create(size);
    Array *current = polystatic_cast<Array *>(ref);
    size_t index = 0;

    if(array && current) {
        while(index < size && index < current->size) {
            array->assign(index, current->get(index));
            ++index;
        }
    }
    TypeRef::set(array);
}

TypeRef::Counted *ArrayRef::get(size_t index)
{
    Array *array = polystatic_cast<Array*>(ref);
    if(!array)
        return NULL;

    return array->get(index);
}

} // namespace
