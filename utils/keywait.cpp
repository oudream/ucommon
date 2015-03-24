// Copyright (C) 2010-2014 David Sugar, Tycho Softworks.
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

#include <ucommon/secure.h>

#ifdef  _MSWINDOWS_
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

using namespace ucommon;

static shell::flagopt helpflag('h',"--help",    _TEXT("display this list"));
static shell::flagopt althelp('?', NULL, NULL);
static shell::numericopt timeout('t', "--timeout", _TEXT("optional keyboard input timeout"), "seconds", 0);

#ifdef _MSWINDOWS_

HANDLE input;
DWORD mode;

#else

static struct termios orig, current;

static void cleanup(void)
{
    tcsetattr(0, TCSANOW, &orig);
}
#endif

static void keyflush(void)
{
#ifdef  _MSWINDOWS_
    TCHAR ch;
    DWORD count;

    while(WaitForSingleObject(input, 20) == WAIT_OBJECT_0) {
        ReadConsole(input, &ch, 1, &count, NULL);
    }
#else
    fd_set inp;
    struct timeval tv;
    char ch;

    FD_ZERO(&inp);
    FD_SET(0, &inp);
    
    for(;;) {
        tv.tv_usec = 20000;
        tv.tv_sec = 0;
        if(select(1, &inp, NULL, NULL, &tv) <= 0)
            break;
        if(::read(0, &ch, 1) < 1)
            break;
    }
#endif
}

int main(int argc, char **argv)
{
    shell::bind("keywait");
    shell args(argc, argv);
    bool nl = false;

    unsigned count = 0;
    int excode = 2;

    if(is(helpflag) || is(althelp)) {
        printf("%s\n", _TEXT("Usage: keywait [options] [text]"));
        printf("%s\n\n", _TEXT("Wait for keyboard input"));
        printf("%s\n", _TEXT("Options:"));
        shell::help();
        printf("\n%s\n", _TEXT("Report bugs to dyfet@gnutelephony.org"));
        return 0;
    }

    while(count < args()) {
        shell::printf("%s ", args[count++]);
        nl = true;
    }

#ifdef  _MSWINDOWS_
    input = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(input, &mode);
    SetConsoleMode(input, 0);

    keyflush();

    DWORD timer = INFINITE;

    if(is(timeout))
        timer = *timeout;

    if(WaitForSingleObject(input, timer) == WAIT_OBJECT_0) {
        keyflush();
        excode = 0;
    }
    else
        excode = 1;

    SetConsoleMode(input, mode);

#else
    tcgetattr(0, &orig);
    shell::exiting(&cleanup);

    tcgetattr(0, &current);
    cfmakeraw(&current);
    tcsetattr(0, TCSANOW, &current);
    fd_set inp;
    struct timeval tv = {0, 0};
    struct timeval *tvp = NULL;

    FD_ZERO(&inp);
    FD_SET(0, &inp);
    

    if(*timeout) {
        tv.tv_sec = *timeout;
        tvp = &tv;
    }

    int rtn = select(1, &inp, NULL, NULL, tvp);
    if(rtn > 0) {
        keyflush();
        excode = 0;
    }
    else {
        if(rtn == 0)
            excode = 1;
    }
    cleanup();
#endif
    if(nl)
        shell::printf("\n");

    return excode;
}

