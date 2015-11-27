# Copyright (C) 2010-2014 David Sugar, Tycho Softworks.
# Copyright (C) 2015 Cherokees of Idaho.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

MAINTAINERCLEANFILES = Makefile.in Makefile
RELEASE = -version-info $(LT_VERSION)
AM_CXXFLAGS = -I$(top_srcdir)/inc @UCOMMON_FLAGS@

noinst_HEADERS = local.h md5.h sha1.h sha2.h brg_types.h brg_endian.h
lib_LTLIBRARIES = libusecure.la

libusecure_la_LDFLAGS = ../corelib/libucommon.la @SECURE_LIBS@ @UCOMMON_LIBS@ $(RELEASE)
libusecure_la_SOURCES = secure.cpp digest.cpp random.cpp cipher.cpp hmac.cpp \
	sstream.cpp md5.cpp sha1.cpp sha2.cpp common.cpp 

