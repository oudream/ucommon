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

#ifndef DEBUG
#define DEBUG
#endif

#include <ucommon-config.h>
#include <ucommon/secure.h>

#include <stdio.h>

using namespace ucommon;

int main(int argc, char **argv)
{
    digest_t md5("md5");

    md5.puts("this is some text");
    assert(eq("684d9d89b9de8178dcd80b7b4d018103", *md5));

    md5 = "sha";
    md5.puts("something else");
    assert(!eq("684d9d89b9de8178dcd80b7b4d018103", *md5));

    md5 = "md5";
    md5.puts("this is some text");
    assert(eq("684d9d89b9de8178dcd80b7b4d018103", *md5));
    secure::keybytes key = md5.key();
    assert(eq("684d9d89b9de8178dcd80b7b4d018103", key.hex()));

    secure::string dig = Digest::md5("this is some text");
    assert(eq("684d9d89b9de8178dcd80b7b4d018103", *dig));

    return 0;
}

