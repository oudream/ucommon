# Copyright (C) 2009-2014 David Sugar, Tycho Softworks
# Copyright (C) 2015 Cherokees of Idaho.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# This is a simplified build system for GNU uCommon.  In particular, it
# offers limited cmodel linkage support, at least for mingw32 support (and
# probably visual c 6 also).  If one really needs to do such things outside
# these use cases, it is suggested to continue using the existing autotools
# configure script driven build environment instead.  Mostly the cmake one is
# meant for generating project files for those working with IDE's to do
# otherwise generic builds of the library and supporting applications.

cmake_minimum_required(VERSION 2.6)
PROJECT(ucommon)
set (VERSION 6.5.0)

if(BIICODE)
    include(dyfet/ucommon/cmake/biicode)
    return()
endif()

# when we override default install prefix, assume full path is used...

if(WIN32)
    set(MODULE_FLAGS "-module -shared -no-undefined")
else()
    set(MODULE_FLAGS "-module -shared -avoid-version")
endif()

# project options

option(BUILD_STDLIB "Set to OFF to disable C++ stdlib" ON)
if(WIN32)
    option(BUILD_RUNTIME "Set to OFF to build static runtime" ON)
    if(BUILD_RUNTIME)
        set(BUILD_RUNTIME_TYPE SHARED)
    endif()
    option(BUILD_STATIC "Set to OFF to build shared libraries" ON)
    set(POSIX_TIMERS OFF CACHE BOOL "does not use posix timers" FORCE)
else()
    option(BUILD_STATIC "Set to ON to build static libraries" OFF)
    option(POSIX_TIMERS "Set to ON to enable" OFF)
endif()

option(BUILD_EXTRAS  "Set to OFF to disable" ON)
option(BUILD_ATOMICS "Set to OFF to disable" ON)
option(BUILD_TESTING "Set to ON to build test programs" OFF)
option(CRYPTO_STATIC "Set to ON to build static crypto" OFF)
option(CRYPTO_OPENSSL "Set to OFF to disable openssl" ON)

MARK_AS_ADVANCED(POSIX_TIMERS BUILD_ATOMICS BUILD_EXTRAS)

if(BUILD_ATOMICS)
    set(HAVE_ATOMICS true)
else()
    set(HAVE_ATOMICS false)
endif()

MESSAGE( STATUS "Configuring GNU ${PROJECT_NAME} ${VERSION}...")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(GNUInstallDirs)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CTest)
include(CapeConfig)
include(CapeMakeTargets)
include(ucommon)
include(config)

# we are making this optional in automake, not default...

find_package(PkgConfig)

if(HAVE_REGEX_H)
    check_library_exists(regex regfree "" HAVE_REGEX_LIB)
    if(HAVE_REGEX_LIB OR MINGW OR MSYS)
        set(UCOMMON_LIBS ${UCOMMON_LIBS} "regex")
    endif()
endif()

set(UCOMMON_LIBS ${UCOMMON_LIBS} ${UCOMMON_LINKING})

# for some reason, normal library searches always fail on broken windows
if (WIN32 AND NOT UNIX AND NOT MINGW AND NOT MSYS)
    set(HAVE_GETADDRINFO True)
    set(HAVE_INET_NTOP True)
endif()

# alternate we use generic cmake openssl search...
if(CRYPTO_OPENSSL AND PKGCONFIG_FOUND)
    pkg_check_modules(OPENSSL openssl>=1.0.0)
endif()
if(CRYPTO_OPENSSL AND NOT OPENSSL_FOUND)
    FIND_PACKAGE(OpenSSL)
    if(OPENSSL_FOUND)
        set(OPENSSL_INCLUDE_DIRS "${OPENSSL_INCLUDE_DIR}")
        set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${OPENSSL_INCLUDE_DIR}")
    endif()
endif()
if(OPENSSL_FOUND)
    set(HAVE_OPENSSL TRUE)
    check_include_files(openssl/fips.h HAVE_OPENSSL_FIPS_H)
elseif(PKGCONFIG_FOUND)
    pkg_check_modules(GNUTLS gnutls>=3.0.0)
endif()

# common build options can be passed to cmake using WITH_CFLAGS, WITH_LIBS,
# and WITH_INCLUDES.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc ${WITH_INCLUDES})
add_definitions(${UCOMMON_FLAGS} ${WITH_CFLAGS})
link_libraries(${WITH_LIBS})

# by default we build static libs for windows, shared libs for unix.
# we may also set this from a top level cmake or -DWITH_XX_LIBS

if(BUILD_STATIC)
    set(BUILD_LIBRARY_TYPE STATIC)
    set(BUILD_CRYPTO_TYPE STATIC)
else()
    set(BUILD_LIBRARY_TYPE SHARED)
    if(CRYPTO_STATIC)
        set(BUILD_CRYPTO_TYPE STATIC)
    else()
        set(BUILD_CRYPTO_TYPE SHARED)
    endif()
endif()

if(NOT BUILD_RUNTIME_TYPE)
    set(BUILD_RUNTIME_TYPE ${BUILD_LIBRARY_TYPE})
endif()

file(GLOB common_src corelib/*.cpp)
file(GLOB ucommon_inc inc/ucommon/*.h)
file(GLOB commoncpp_src commoncpp/*.cpp)
file(GLOB commoncpp_inc inc/commoncpp/*.h)
file(GLOB scripts_man *.1)
file(GLOB cmakes_inc cmake/Cape*.cmake)
list(REMOVE_ITEM ucommon_inc inc/ucommon/secure.h)
set(secure_inc inc/ucommon/secure.h)

if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    link_directories(${OPENSSL_LIBRARY_DIRS})
    file(GLOB secure_src openssl/*.cpp openssl/*.h)
    list(APPEND secure_src nossl/common.cpp)
    if(MINGW)
        set(SECURE_LIBS ${OPENSSL_LIBRARIES} gdi32 z)
    elseif(WIN32 AND NOT CYGWIN)
        set(SECURE_LIBS ${OPENSSL_LIBRARIES} gdi32)
    else()
        set(SECURE_LIBS ${OPENSSL_LIBRARIES})
    endif()
elseif(GNUTLS_FOUND)
    include_directories(${GNUTLS_INCLUDE_DIRS})
    link_directories(${GNUTLS_LIBRARY_DIRS})
    file(GLOB secure_src gnutls/*.cpp gnutls/*.h)
    list(APPEND secure_src nossl/common.cpp)
    set(SECURE_LIBS ${GNUTLS_LIBRARIES})
else()
    file(GLOB secure_src nossl/*.cpp nossl/*.h)
endif()

pc_flags(PKG_UCOMMON_FLAGS ${UCOMMON_FLAGS} ${UCOMMON_VISIBILITY_FLAG})
pc_libs(PKG_UCOMMON_LIBS ${UCOMMON_LIBS})
pc_libs(PKG_SECURE_LIBS ${SECURE_LIBS})

create_specfile()
create_headers(ucommon-config.h)
create_scripts(scripts_cfg ucommon-config commoncpp-config)
create_scripts(localscripts PKGBUILD)
create_pcfiles(pc_files ucommon commoncpp)

add_library(ucommon ${BUILD_RUNTIME_TYPE} ${common_src} ${ucommon_inc})
set_library_version(ucommon)
target_link_libraries(ucommon ${UCOMMON_LIBS} ${WITH_LIBS})

add_library(usecure ${BUILD_CRYPTO_TYPE} ${secure_src} ${secure_inc})
set_library_version(usecure)
target_link_libraries(usecure ucommon ${WITH_LIBS} ${SECURE_LIBS} ${UCOMMON_LIBS})
add_dependencies(usecure ucommon)

if(BUILD_STDLIB)
    add_library(commoncpp ${BUILD_LIBRARY_TYPE} ${commoncpp_src} ${commoncpp_inc})
    set_library_version(commoncpp)
    target_link_libraries(commoncpp ucommon ${UCOMMON_LIBS} ${WITH_LIBS} ${SECURE_LIBS})
    add_dependencies(commoncpp ucommon)
endif()

if (BUILD_EXTRAS)
    add_subdirectory(utils)
endif()

if (BUILD_TESTING)
    ENABLE_TESTING()
    add_subdirectory(test)
endif()

install(FILES ${pc_files} DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(FILES ${scripts_cfg} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${scripts_man} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install(FILES ${cmakes_inc} DESTINATION ${CMAKE_INSTALL_DATADIR}/ucommon/cmake)
install(FILES ${ucommon_inc} ${secure_inc} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ucommon)
install(FILES ${commoncpp_inc} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/commoncpp)

if(BUILD_STDLIB)
    install(TARGETS commoncpp DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

install(TARGETS ucommon usecure DESTINATION ${CMAKE_INSTALL_LIBDIR})

# cmake std targets not used; dist made on automake...
#add_cape_make_targets(ucommon ${VERSION})
add_make_lint_target()
add_cape_docs_target(Doxyfile)
add_make_uninstall_target()
