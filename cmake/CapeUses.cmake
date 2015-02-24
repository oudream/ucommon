#.rst:
# CapeMakeTargets
# ---------------

#=============================================================================
# Copyright (C) 2015 David Sugar, Tycho Softworks.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#=============================================================================

include(CapeConfig)

macro(uses_ucommon _VERSION)
    if (USES_UCOMMON_INCLUDE_DIRS)
        message(STATUS "  Using local ucommon dependency")
    else()
        find_package(PkgConfig)
        pkg_check_modules(USES_UCOMMON REQUIRED ucommon>=${_VERSION})
    endif()

    set(USES_UCOMMON_REQUIRES ${_VERSION})
    include_directories(${USES_UCOMMON_INCLUDE_DIRS})
    link_directories(${USES_UCOMMON_LIBRARY_DIRS})
    add_definitions(${USES_UCOMMON_CFLAGS})
endmacro()

macro(uses_sipwitch _VERSION)
    if (USES_SIPWITCH_INCLUDE_DIRS)
        message(STATUS "  Using local sipwitch dependency")
    else()
        find_package(PkgConfig)
        pkg_check_modules(USES_SIPWITCH REQUIRED libsipwitch>=${_VERSION})
    endif()

    set(USES_SIPWITCH_REQUIRES ${_VERSION})
    include_directories(${USES_SIPWITCH_INCLUDE_DIRS})
    link_directories(${USES_SIPWITCH_LIBRARY_DIRS})
    add_definitions(${USES_SIPWITCH_CFLAGS})
endmacro()

macro(uses_ccscript _VERSION)
    if (USES_CCSCRIPT_INCLUDE_DIRS)
        message(STATUS "  Using local ccscript dependency")
    else()
        find_package(PkgConfig)
        pkg_check_modules(USES_CCSCRIPT REQUIRED ccscript>=${_VERSION})
    endif()

    set(USES_CCSCRIPT_REQUIRES ${_VERSION})
    include_directories(${USES_CCSCRIPT_INCLUDE_DIRS})
    link_directories(${USES_CCSCRIPT_LIBRARY_DIRS})
    add_definitions(${USES_CCSCRIPT_CFLAGS})
endmacro()

macro(uses_ccaudio2 _VERSION)
    if (USES_CCAUDIO2_INCLUDE_DIRS)
        message(STATUS "  Using local ccaudio2 dependency")
    else()
        find_package(PkgConfig)
        pkg_check_modules(USES_CCAUDIO2 REQUIRED ccaudio2>=${_VERSION})
    endif()

    set(USES_CCAUDIO2_REQUIRES ${_VERSION})
    include_directories(${USES_CCAUDIO2_INCLUDE_DIRS})
    link_directories(${USES_CCAUDIO2_LIBRARY_DIRS})
    add_definitions(${USES_CCAUDIO2_CFLAGS})
endmacro()
