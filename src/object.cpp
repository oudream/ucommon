// Copyright (C) 2006-2007 David Sugar, Tycho Softworks.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

#include <config.h>
#include <ucommon/object.h>
#include <stdlib.h>
#include <string.h>

using namespace UCOMMON_NAMESPACE;

CountedObject::CountedObject()
{
	count = 0;
}

CountedObject::CountedObject(const Object &source)
{
	count = 0;
}

Object::~Object()
{
}

void Object::release(void)
{
	delete this;
}

void Object::retain(void)
{
}

Object *Object::copy(void)
{
	retain();
	return this;
}

Temporary::~Temporary()
{
}

void CountedObject::dealloc(void)
{
	delete this;
}

void CountedObject::retain(void)
{
	++count;
}

void CountedObject::release(void)
{
	if(count > 1) {
		--count;
		return;
	}
	dealloc();
}

auto_delete::~auto_delete()
{
	if(object)
		delete object;

	object = 0;
}

auto_pointer::auto_pointer(Object *o)
{
	o->retain();
	object = o;
}

auto_pointer::auto_pointer()
{
	object = 0;
}

void auto_pointer::release(void)
{
	if(object)
		object->release();
	object = 0;
}

auto_pointer::~auto_pointer()
{
	release();
}

auto_pointer::auto_pointer(const auto_pointer &from)
{
	object = from.object;
	if(object)
		object->retain();
}

bool auto_pointer::operator!() const
{
	return (object == 0);
}

auto_pointer::operator bool() const
{
    return (object != 0);
}

bool auto_pointer::operator==(Object *o) const
{
	return object == o;
}

bool auto_pointer::operator!=(Object *o) const
{
	return object != o;
}

auto_pointer &auto_pointer::operator=(Object *o)
{
	if(object == o)
		return *this;

	if(o)
		o->retain();
	if(object)
		object->release();
	object = o;
	return *this;
}

sparse_array::sparse_array(unsigned m)
{
	max = m;
	vector = new Object*[m];
	memset(vector, 0, sizeof(Object *) * m);
}

sparse_array::~sparse_array()
{
	purge();
}

void sparse_array::purge(void)
{
	if(!vector)
		return;

	for(unsigned pos = 0; pos < max; ++ pos) {
		if(vector[pos])
			vector[pos]->release();
	}
	delete[] vector;
	vector = NULL; 
}

unsigned sparse_array::count(void)
{
	unsigned c = 0;
	for(unsigned pos = 0; pos < max; ++pos) {
		if(vector[pos])
			++c;
	}
	return c;
}
 
Object *sparse_array::get(unsigned pos)
{
	Object *obj;

	if(pos >= max)
		return NULL;

	if(!vector[pos]) {
		obj = create();
		if(!obj)
			return NULL;
		obj->retain();
		vector[pos] = obj;
	}
	return vector[pos];
}


