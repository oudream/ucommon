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

macro(add_make_dist_target _TARGET _VERSION)
    option(BUILD_DISTFILE "Set to ON to create tarball distfile" OFF)

    if(BUILD_DISTFILE OR CMAKE_GENERATOR MATCHES "Unix Makefiles")
        add_custom_target(cleandist
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMAND rm -f "${CMAKE_CURRENT_BINARY_DIR}/${_TARGET}[-_]*.gz"
            COMMAND rm -f "${CMAKE_CURRENT_BINARY_DIR}/${_TARGET}_*.dsc"
            COMMAND rm -f "${CMAKE_CURRENT_BINARY_DIR}/${_TARGET}-*.rpm"
            COMMAND rm -f "${CMAKE_CURRENT_BINARY_DIR}/${_TARGET}[-_]*.deb"
            COMMAND rm -f "${CMAKE_CURRENT_BINARY_DIR}/*${_TARGET}*[-_]*.deb"
            COMMAND rm -f "${CMAKE_CURRENT_BINARY_DIR}/${_TARGET}-*.zip"
            COMMAND rm -f "${CMAKE_CURRENT_BINARY_DIR}/*${_TARGET}*.changes"
        )

        add_custom_target(dist
            DEPENDS cleandist
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMAND git archive --format tar.gz --output="${CMAKE_CURRENT_BINARY_DIR}/${_TARGET}-${_VERSION}.tar.gz" --prefix="${_TARGET}-${_VERSION}/" HEAD
            COMMAND git archive --format zip --output="${CMAKE_CURRENT_BINARY_DIR}/${_TARGET}-${_VERSION}.zip" --prefix="${_TATGET}-${_VERSION}/" HEAD
        )
    endif()
endmacro(add_make_dist_target)

macro(add_make_srpm_target _TARGET)
    if(UNIX AND CMAKE_GENERATOR MATCHES "Unix Makefiles")
        add_custom_target(srpm
            DEPENDS dist
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMAND rm -f *.rpm
            COMMAND rpmbuild  -bs --nodeps --define "_sourcedir ." --define "_srcrpmdir ." --sign ${_TARGET}.spec
        )
    endif()
endmacro()

macro(add_make_uninstall_target)
    if(CMAKE_GENERATOR MATCHES "Unix Makefiles")
        add_custom_target(uninstall
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMAND xargs <install_manifest.txt
        )
    endif()
endmacro()


macro(add_make_deb_target _TARGET _VERSION)
    if(UNIX AND CMAKE_GENERATOR MATCHES "Unix Makefiles")
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/packaging/debian/")
            add_custom_target(deb
                DEPENDS dist
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                COMMAND rm -f *.deb *.debian.tar.gz *.dsc *.changes
                COMMAND cape-source --sign ${_TARGET}-${_VERSION}.tar.gz packaging
            )
        endif()
    endif()
endmacro()

macro(add_cape_make_targets _TARGET _VERSION)
    add_make_dist_target(${_TARGET} ${_VERSION})
    add_make_deb_target(${_TARGET} ${_VERSION})
    add_make_srpm_target(${_TARGET})
    add_make_uninstall_target()
endmacro()

macro(add_cape_docs_target _DOXYFILE)
    option(BUILD_DOCS "Set to ON to create doxygen docs" OFF)
    if(BUILD_DOCS OR CMAKE_GENERATOR MATCHES "Unix Makefiles")
        find_package(Doxygen)
    endif()
    if(DOXYGEN_FOUND)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${_DOXYFILE}.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/${_DOXYFILE} @ONLY )
        add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/${_DOXYFILE}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
        )
        if(WIN32)
            find_package(HTMLHelp)
            if(HTML_HELP_COMPILER AND EXISTS "doc/")
                set(TMP "${CMAKE_CURRENT_BINARY_DIR}\\doc\\html\\index.hhp")
                string(REGEX REPLACE "[/]" "\\\\" HHP_FILE ${TMP} )
                add_custom_target(winhelp ${HTML_HELP_COMPILER} ${HHP_FILE})
                add_dependencies(winhelp doc)
            endif()
        endif()
    endif()
endmacro()

