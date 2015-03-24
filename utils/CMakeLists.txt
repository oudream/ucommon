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

file(GLOB ucommon_man *.1)

add_executable(ucommon-args args.cpp)
add_dependencies(ucommon-args ucommon)
set_target_properties(ucommon-args PROPERTIES OUTPUT_NAME args)
target_link_libraries(ucommon-args ucommon ${UCOMMON_LIBS} ${WITH_LIBS})

add_executable(usecure-car car.cpp)
add_dependencies(usecure-car usecure ucommon)
set_target_properties(usecure-car PROPERTIES OUTPUT_NAME car)
target_link_libraries(usecure-car usecure ucommon ${SECURE_LIBS} ${UCOMMON_LIBS} ${WITH_LIBS})

add_executable(ucommon-keywait keywait.cpp)
add_dependencies(ucommon-keywait ucommon)
set_target_properties(ucommon-keywait PROPERTIES OUTPUT_NAME keywait)
target_link_libraries(ucommon-keywait ucommon ${UCOMMON_LIBS} ${WITH_LIBS})

add_executable(ucommon-pdetach pdetach.cpp)
add_dependencies(ucommon-pdetach ucommon)
set_target_properties(ucommon-pdetach PROPERTIES OUTPUT_NAME pdetach)
target_link_libraries(ucommon-pdetach ucommon ${UCOMMON_LIBS} ${WITH_LIBS})

add_executable(ucommon-sockaddr sockaddr.cpp)
add_dependencies(ucommon-sockaddr ucommon)
set_target_properties(ucommon-sockaddr PROPERTIES OUTPUT_NAME sockaddr)
target_link_libraries(ucommon-sockaddr ucommon ${UCOMMON_LIBS} ${WITH_LIBS})

add_executable(usecure-scrub scrub.cpp)
add_dependencies(usecure-scrub usecure ucommon)
set_target_properties(usecure-scrub PROPERTIES OUTPUT_NAME scrub-files)
target_link_libraries(usecure-scrub usecure ucommon ${SECURE_LIBS} ${UCOMMON_LIBS} ${WITH_LIBS})

add_executable(usecure-mdsum mdsum.cpp)
add_dependencies(usecure-mdsum usecure ucommon)
set_target_properties(usecure-mdsum PROPERTIES OUTPUT_NAME mdsum)
target_link_libraries(usecure-mdsum usecure ucommon ${SECURE_LIBS} ${UCOMMON_LIBS} ${WITH_LIBS})

add_executable(usecure-zerofill zerofill.cpp)
add_dependencies(usecure-zerofill usecure ucommon)
set_target_properties(usecure-zerofill PROPERTIES OUTPUT_NAME zerofill)
target_link_libraries(usecure-zerofill usecure ucommon ${SECURE_LIBS} ${UCOMMON_LIBS} ${WITH_LIBS})

install(TARGETS ucommon-args ucommon-pdetach ucommon-keywait usecure-car usecure-scrub usecure-mdsum ucommon-sockaddr usecure-zerofill DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${ucommon_man} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

