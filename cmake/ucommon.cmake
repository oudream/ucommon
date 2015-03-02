# Copyright (C) 2009-2014 David Sugar, Tycho Softworks
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

if (NOT UCOMMON_CONFIGURED)
    set(UCOMMON_CONFIGURED TRUE)
    include(CheckCCompilerFlag)

    if(CMAKE_COMPILER_IS_GNUCXX)
        set(UCOMMON_VISIBILITY_FLAG "-fvisibility=hidden")
        if(MINGW OR MSYS OR CMAKE_SYSTEM MATCHES "Windows")
            set(CHECK_FLAGS -Wno-long-long -mthreads -fvisibility-inlines-hidden)
        else()
            if(CMAKE_SYSTEM MATCHES "SunOS.*")
                set(CHECK_FLAGS -Wno-long-long -mt -fvisibility-inlines-hidden)
            else()
                set(CHECK_FLAGS -Wno-long-long -pthread -mt -fvisibility-inlines-hidden)
            endif()
        endif()
    endif()

    if(BUILD_RUNTIME AND WIN32)
        set(UCOMMON_FLAGS ${UCOMMON_FLAGS} -DUCOMMON_RUNTIME)
    else()
        if(BUILD_STATIC)
            set(UCOMMON_FLAGS ${UCOMMON_FLAGS} -DUCOMMON_STATIC)
        endif()
    endif()

    # see if we are building with or without std c++ libraries...
    if (NOT BUILD_STDLIB)
        set(UCOMMON_FLAGS ${UCOMMON_FLAGS} -DUCOMMON_SYSRUNTIME)
        MESSAGE( STATUS "Configuring minimal C++ runtime")
        if(CMAKE_COMPILER_IS_GNUCXX)
            set(CHECK_FLAGS ${CHECK_FLAGS} -fno-exceptions -fno-rtti -fno-enforce-eh-specs)
            if(MINGW OR MSYS OR CMAKE_SYSTEM MATCHES "Windows")
                set(UCOMMON_LINKING -nodefaultlibs -nostdinc++)
            else()
                set(UCOMMON_LINKING -nodefaultlibs -nostdinc++)
            endif()
        endif()
    endif()

    # check final for compiler flags
    foreach(flag ${CHECK_FLAGS})
        check_c_compiler_flag(${flag} CHECK_${flag})
        if(CHECK_${flag})
            set(UCOMMON_FLAGS ${UCOMMON_FLAGS} ${flag})
        endif()
    endforeach()

    # visibility support for linking reduction (gcc >4.1 only so far...)

    if(UCOMMON_VISIBILITY_FLAG)
        check_c_compiler_flag(${UCOMMON_VISIBILITY_FLAG} CHECK_VISIBILITY)
    endif()

    find_package(Threads)
    if (CMAKE_HAVE_PTHREAD_H)
        set(HAVE_PTHREAD_H TRUE)
    endif()
    set (UCOMMON_LIBS ${UCOMMON_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${WITH_LDFLAGS})

    if (MINGW OR MSYS)
        set (UCOMMON_LIBS ${UCOMMON_LIBS} mingwex mingw32)
    else()
        if (WIN32 AND CMAKE_COMPILER_IS_GNUCXX)
            set (UCOMMON_LIBS ${UCOMMON_LIBS} mingwex mingw32)
        endif()
    endif()

    if (WIN32 OR MINGW OR MSYS OR CMAKE_SYSTEM MATCHES "Windows")
        set (UCOMMON_LIBS ${UCOMMON_LIBS} crypt32 advapi32 user32 ws2_32 wsock32 kernel32)
    endif()

    if(UNIX AND NOT BUILD_STDLIB)
        set(UCOMMON_LIBS ${UCOMMON_LIBS} c)
    endif()

    if(MSYS OR MINGW)
        set(UCOMMON_LIBS ${UCOMMON_LIBS} msvcrt)
    endif()

    if(CMAKE_COMPILER_IS_GNUCXX AND NOT BUILD_STDLIB)
        check_library_exists(gcc __modsi3 "" HAVE_GCC_LIB)
        if(HAVE_GCC_LIB)
                set(UCOMMON_LIBS ${UCOMMON_LIBS} gcc)
        endif()
    endif()

    if(UNIX OR MSYS OR MINGW OR CYGWIN)
        check_library_exists(dl dlopen "" HAVE_DL_LIB)
        if (HAVE_DL_LIB)
            set (UCOMMON_LIBS ${UCOMMON_LIBS} dl)
        else()
            check_library_exists(compat dlopen "" HAVE_COMPAT_LIB)
            if(HAVE_COMPAT_LIB)
                set (UCOMMON_LIBS ${UCOMMON_LIBS} compat)
            endif()
        endif()

        check_library_exists(dld shl_load "" HAVE DLD_LIB)
        if (HAVE_DLD_LIB)
            set (UCOMMON_LIBS ${UCOMMON_LIBS} dld)
        endif()

        check_library_exists(socket socket "" HAVE_SOCKET_LIB)
        if (HAVE_SOCKET_LIB)
            set (UCOMMON_LIBS ${UCOMMON_LIBS} socket)
        endif()

        check_library_exists(posix4 sem_wait "" HAVE_POSIX4_LIB)
        if (HAVE_POSIX4_LIB)
            set(UCOMMON_LIBS ${UCOMMON_LIBS} posix4)
        endif()

        check_library_exists(rt clock_gettime "" HAVE_RT_LIB)
        if (HAVE_RT_LIB)
            set(UCOMMON_LIBS ${UCOMMON_LIBS} rt)
        endif()
    endif()
endif()
