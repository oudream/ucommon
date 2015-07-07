#.rst:
# biicode
# ---------------

#=============================================================================
# Copyright (C) 2015 Cherokees of Idaho.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#=============================================================================

include("${CMAKE_HOME_DIRECTORY}/biicode.cmake")
init_biicode_block()
include(dyfet/ucommon/cmake/CapeConfig)
include(dyfet/ucommon/cmake/CapeMakeTargets)
include(dyfet/ucommon/cmake/ucommon)
include(dyfet/ucommon/cmake/config)

set(OPENSSL_FOUND TRUE)
set(HAVE_OPENSSL TRUE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ucommon-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/ucommon-config.h NEWLINE_STYLE UNIX)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_biicode_targets()
if(UNIX AND NOT APPLE)
	target_link_libraries(${BII_BLOCK_TARGET} INTERFACE dl)
endif()


