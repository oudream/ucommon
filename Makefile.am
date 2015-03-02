# Copyright (C) 2006-2014 David Sugar, Tycho Softworks.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = autogen.sh README* *.pc.in *.spec.in *.spec *-config.in \
    *-config.1 Doxyfile.in BUILDS SUPPORT COPYING* COPYRIGHT CMakeLists.txt \
    cmake-abi.sh ucommon-config.h.cmake directive.in cmake/*.cmake

DIST_SUBDIRS = corelib commoncpp openssl gnutls nossl utils inc test
SUBDIRS = corelib @SECURE@ @COMPAT@ inc test
if BUILD_UTILS
SUBDIRS += utils
endif
EXTRA_SCRIPTS = commoncpp-config

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ucommon.pc @COMPAT_PC@

scriptdir = $(bindir)
script_SCRIPTS = ucommon-config @COMPAT_CONFIG@

cmakedir = $(datadir)/ucommon/cmake
cmake_DATA = cmake/Cape*.cmake

man_MANS = ucommon-config.1 commoncpp-config.1

.phony:	doxy todo

todo:
	@grep --color=tty --exclude="Makefile*" --exclude-dir=.git -nrEI "/[*] TODO|/[*] FIXME|// TODO|// FIXME" .

doxy:
	-rm -rf doc
	$(DOXYGEN) Doxyfile

srpm:	dist
	rm -f *.rpm
	rpmbuild  -bs --nodeps --define "_sourcedir ." --define "_srcrpmdir ." --sign ucommon.spec

deb:	dist
	rm -f *.deb *.debian.tar.gz *.dsc *.changes
	cape-source --sign ucommon-${VERSION}.tar.gz packaging

clean-local:
	-rm -rf doc

dist-hook:  cleandist

cleandist:
	@rm -f ${PACKAGE}-[0-9]*.tar.gz
	@rm -f ${PACKAGE}-[0-9]*.tar.xz
	@rm -f ${PACKAGE}-[0-9]*.tar.bz2
	@rm -f ${PACKAGE}-[0-9]*.shar.gz
	@rm -f ${PACKAGE}-[0-9]*.zip
	@rm -f *${PACKAGE}*.rpm
	@rm -f *${PACKAGE}*.deb
	@rm -f ${PACKAGE}_*.xz
	@rm -f ${PACKAGE}_*.gz
	@rm -f ${PACKAGE}_*.dsc
	@rm -f *.changes

