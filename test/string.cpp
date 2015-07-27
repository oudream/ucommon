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

#include <ucommon/ucommon.h>

#include <stdio.h>

using namespace ucommon;

static string_t testing("second test");

extern "C" int main()
{
    char buff[33];
    char *tokens = NULL;
    unsigned count = 0;
    const char *tp;
    const char *array[5];
    stringref_t sref = "ABC";
    string_t tref(sref);

    assert(eq(tref, "ABC"));
    assert(eq(sref, "ABC"));

    assert(max(3, 2) == 3);

    String::fill(buff, 32, ' ');
    stringbuf<128> mystr;
    mystr = (string_t)"hello" + (string_t)" this is a test";
    assert(eq_case("hello this is a test", *mystr));
    assert(eq_case("second test", *testing));
    assert(eq_case(" Is a test", mystr(-10)));
    mystr = "  abc 123 \n  ";
    assert(eq_case("abc 123", String::strip(mystr.c_mem(), " \n")));
    String::set(buff, sizeof(buff), "this is \"a test\"");
    while(NULL != (tp = String::token(buff, &tokens, " ", "\"\"")) && count < 4)
        array[count++] = tp;
    assert(count == 3);
    assert(eq_case(array[1], "is"));
    assert(eq_case(array[2], "a test"));

    uint8_t core[4] = {0x01, 0x10, 0x2f, 0x45};
    char hexbuf[12];

    assert(String::hexdump(core, hexbuf, "3-1") == 9);
    assert(eq(hexbuf, "01102f-45"));

    uint8_t hcore[4];

    String::hexpack(hcore, hexbuf, "3-1");
    assert(String::hexdump(hcore, hexbuf, "3-1") == 9);
    assert(eq(hexbuf, "01102f-45"));

    String numstr = "-33.5,25";
    Real num1;
    Unsigned num2;

    numstr % num1 % "," % num2;
    assert(num1 == -33.5);
    assert(num2 == 25);
    assert(numstr.len() == 0);

    char *test = strdup(str("hello") + " test" + str((short)13));
    assert(eq(test, "hello test13"));

    char *cdup = dup<char>(test[6]);
    assert(eq(cdup, "test13"));

    String paste_test = "foo";
    paste_test.paste(3, "bar", 3);
    String paste_test_empty;
    paste_test_empty.paste(3, "bar", 3);
    assert(eq(paste_test, "foobar"));
    assert(eq(paste_test_empty, "bar"));

    assert(String::check("xxx", 3));
    assert(!String::check("xxxx", 3));

    uint8_t hbuf[2];
    hbuf[0] = 0x23;
    hbuf[1] = 0xa9;
    string_t hex = String::hex(hbuf, 2);
    assert(eq(hex, "23a9"));

    strfree(test);
    strfree(cdup);

    stringref_t cvs;
    charvalues_t cv1 = stringref::create(64);
    charvalues_t cv2 = cv1;

    snprintf(cv1->get(), cv1->max(), "test %d\n", 1);
    stringref::expand(&cv1, 64);
    assert(cv1->max() == 128);
    assert(cv2 != cv1);
    assert(eq(cv1->get(), "test 1\n"));
    cvs.assign(cv1);
    assert(cvs.copies() == 1);
    assert(eq(cvs, "test 1\n"));
    assert(cvs.size() == 7);

    return 0;
}
