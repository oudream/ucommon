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

} // namespace