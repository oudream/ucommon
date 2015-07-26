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
#include <cstdlib>

namespace ucommon {

CountedType::CountedType(size_t size) : 
ObjectProtocol()
{
	this->size = size;
}

CountedType::CountedType(const CountedType& source)
{
	this->size = source.size;
}

void CountedType::dealloc()
{
	delete this;
}

void CountedType::retain(void)
{
	count.fetch_add();
}

void CountedType::release(void)
{
	if(count.fetch_sub() == 1) {
		dealloc();
	}
}

} // namespace
