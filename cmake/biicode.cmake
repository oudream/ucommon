include("${CMAKE_HOME_DIRECTORY}/biicode.cmake")
init_biicode_block()
include(dyfet/ucommon/cmake/CapeConfig)
include(dyfet/ucommon/cmake/CapeMakeTargets)
include(dyfet/ucommon/cmake/ucommon)
include(dyfet/ucommon/cmake/config)

set(OPENSSL_FOUND TRUE)
set(HAVE_OPENSSL TRUE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ucommon-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/ucommon-config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_biicode_targets()
if(UNIX AND NOT APPLE)
	target_link_libraries(${BII_BLOCK_TARGET} INTERFACE dl)
endif()


