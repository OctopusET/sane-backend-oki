##  Makefile.am -- an automake template for Makefile.in file
##  Copyright (c) 2012 Oki Data Corporation. All rights reserved.

RELEASEFILE=/etc/lsb-release
DISTRIB=$(shell if test -f $(RELEASEFILE); then . $(RELEASEFILE); echo $$DISTRIB_ID; else echo "UNKNOWN"; fi)

#AM_CPPFLAGS = -I. -I$(top_builddir)/include -I$(top_srcdir)/include
AM_CPPFLAGS = -I. 

bin_PROGRAMS =
sbin_PROGRAMS =
noinst_PROGRAMS = 

noinst_LTLIBRARIES = 




bin_SCRIPTS = 
sbin_SCRIPTS =
noinst_SCRIPTS = 

BUILT_SOURCES = 
CLEANFILES = $(bin_SCRIPTS) $(dist_noinst_SCRIPTS)


V_MAJOR = @V_DRIVER_MAJOR@
V_MINOR = @V_DRIVER_MINOR@
V_REV = @V_DRIVER_REV@
V_OEM = @OEM@

COMMON_CPPFLAGS=-DOEM=@OEM@

AM_LDFLAGS = @STRICT_LDFLAGS@
DIST_LIBS_LDFLAGS = $(AM_LDFLAGS) -rpath '$(libdir)' -version-number $(V_MAJOR):$(V_MINOR):$(V_REV)

EXTRA_DIST = 


lib_LTLIBRARIES = lib@OEM@utl.la lib@OEM@scandif.la


lib@OEM@utl_la_SOURCES = \
	odc_tracer.c \
	odc_tracer.h \
	odc_misc.cpp \
	odc_misc.h \
	odc_boundary.cpp \
	odc_boundary.h \
	odc_pnm.cpp \
	odc_pnm.h \
	odc_define.h
lib@OEM@utl_la_CPPFLAGS = $(AM_CPPFLAGS) -DOEM=@OEM@ -DLIBNAME=lib@OEM@utl -fexceptions
lib@OEM@utl_la_LDFLAGS = $(DIST_LIBS_LDFLAGS) -lpthread
lib@OEM@utl_la_LIBADD =

lib@OEM@scandif_la_SOURCES = \
	odc_scandif.cpp \
	odc_scandif.h \
	scandifc.cpp \
	scandifc.h \
	odc_define.h
lib@OEM@scandif_la_CPPFLAGS = $(AM_CPPFLAGS) -DOEM=@OEM@ -DLIBNAME=lib@OEM@scandif -fexceptions
lib@OEM@scandif_la_LDFLAGS = $(DIST_LIBS_LDFLAGS) -l@OEM@utl
lib@OEM@scandif_la_LIBADD =



