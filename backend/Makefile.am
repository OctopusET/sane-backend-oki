##  Makefile.am -- an automake template for Makefile.in file
##  Copyright(c) 2012 Oki Data Corporation. All rights reserved.
##  Copyright (C) 2009 Chris Bagwell, Olaf Meeuwissen, and Sane Developers.
##
##  This file is part of the "Sane" build infra-structure.  See
##  included LICENSE file for license information.

INSTALL_LOCKPATH = @INSTALL_LOCKPATH@
LOCKPATH_GROUP = @LOCKPATH_GROUP@

BACKEND_LIBS_ENABLED=@BACKEND_LIBS_ENABLED@
BACKEND_CONFS_ENABLED=@BACKEND_CONFS_ENABLED@

DL_LIBS = @DL_LIBS@ 
LIBV4L_LIBS = @LIBV4L_LIBS@ 
MATH_LIB = @MATH_LIB@ 
IEEE1284_LIBS = @IEEE1284_LIBS@ 
TIFF_LIBS = @TIFF_LIBS@ 
JPEG_LIBS = @JPEG_LIBS@ 
GPHOTO2_LIBS = @GPHOTO2_LIBS@
GPHOTO2_LDFLAGS = @GPHOTO2_LDFLAGS@
SOCKET_LIBS = @SOCKET_LIBS@ 
AVAHI_LIBS = @AVAHI_LIBS@
USB_LIBS = @USB_LIBS@ 
SCSI_LIBS = @SCSI_LIBS@
PTHREAD_LIBS = @PTHREAD_LIBS@

AM_CPPFLAGS = -I. -I$(srcdir) -I$(top_builddir)/include -I$(top_srcdir)/include -DLIBDIR="$(libdir)/sane"

AM_LDFLAGS = @STRICT_LDFLAGS@
# The -rpath option is added because we are creating _LTLIBRARIES based
# on configure substitution.  This causes automake to not know the
# correct $libdir and must be added here.
DIST_SANELIBS_LDFLAGS = $(AM_LDFLAGS) -rpath '$(libdir)/sane' -version-number $(V_SANE_BACKEND_MAJOR):$(V_SANE_BACKEND_MINOR):$(V_SANE_BACKEND_REV) $(DYNAMIC_FLAG)
#DIST_LIBS_LDFLAGS = $(AM_LDFLAGS) -rpath '$(libdir)' -version-number $(V_MAJOR):$(V_MINOR):$(V_REV)

# LIBTOOL install is a little to noisy for my liking.
LIBTOOL = @LIBTOOL@ --silent
FIRMWARE_DIRS = 

EXTRA_DIST = logo.xpm
EXTRA_DIST += sane_strstatus.c

all: becfg

# FIXME: % is a GNU extension... This is only thing left requiring
# use to use GNU make.
%-s.c: $(srcdir)/stubs.c
	rm -f $@
	$(LN_S) $(srcdir)/stubs.c $@

EXTRA_DIST += stubs.c


dllsetup:
	@echo Generating $@
	@sed -e 's|@DATADIR@|$(datadir)|g' \
	     -e 's|@CONFIGDIR@|$(configdir)|g' \
	     -e 's|@DOCDIR@|$(docdir)|g' \
	     -e 's|@LIBDIR@|$(libdir)/sane|g' \
	     -e 's|@BINDIR@|$(bindir)|g' \
	     -e 's|@SBINDIR@|$(sbindir)|g' \
	     -e 's|@MAJOR@|$(V_SANE_BACKEND_MAJOR)|g' \
	     -e 's|@MINOR@|$(V_SANE_BACKEND_MINOR)|g' \
	     -e 's|@REV@|$(V_SANE_BACKEND_REV)|g' \
	     -e 's|@BACKENDNAME@|$(BACKEND_NAME)|g' dllsetup.in > $@
	@chmod +x $@

EXTRA_DIST += dllsetup.in


dll-preload.h:
	rm -f $@
	list="$(PRELOADABLE_BACKENDS)"; for be in $$list; do \
	  echo "PRELOAD_DECL($$be)" >> $@; \
	done
	echo "static struct backend preloaded_backends[] = {" >> $@
	sep=""; \
	list="$(PRELOADABLE_BACKENDS)"; \
	if test -z "$${list}"; then \
	  echo { 0, 0, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }} >> $@; \
	else \
	  for be in $$list; do \
	    echo "$${sep}PRELOAD_DEFN($$be)" >> $@; \
	    sep=","; \
	  done; \
	fi
	echo "};" >> $@

# TODO: This really belongs together with the saned sources and
# should be installed there as well.

# Backends are not required to have a config file. Any backend
# that wants to install a config file should list it here.
#BACKEND_CONFS=\
#	dll.conf
BACKEND_CONFS=

becfg: $(BACKEND_CONFS)


install-data-hook: install-becfg install-firmware-path $(INSTALL_LOCKPATH) install-logo

install-logo:
	test -d "$(DESTDIR)/usr/share/sane/xsane/" || $(MKDIR_P) "$(DESTDIR)/usr/share/sane/xsane/"
	$(INSTALL_DATA) logo.xpm $(DESTDIR)/usr/share/sane/xsane/sane-@BACKEND_NAME@-logo.xpm


# Custom install target to install config files.  Do not overwrite
# files that have been previously installed so that user modifications
# are not lost.
install-becfg:
	@# Libtool has a bug where it will sometimes symlink the last 
	@# installed library in $(sanelibdir) to $(sanelibdir)/libsane.*.  
	@# Having two libsane's can cause issues so get rid of it.
	@#-rm -f $(DESTDIR)$(sanelibdir)/libsane.*
	@#test -z "$(configdir)" || $(MKDIR_P) "$(DESTDIR)$(configdir)"
	@list="$(BACKEND_CONFS_ENABLED)"; for cfg in $$list; do \
	  if test ! -r $${cfg}; then continue; fi; \
	  if test -f $(DESTDIR)$(configdir)/$${cfg}; then \
	  echo NOT overwriting $${cfg} in $(configdir)...; \
	  else \
	  echo installing $${cfg} in $(configdir)/$${cfg}...; \
	  $(INSTALL_DATA) $${cfg} $(DESTDIR)$(configdir)/$${cfg} \
	  || exit 1; \
	  fi; \
	  done

install-firmware-path:
	for dir in $(FIRMWARE_DIRS) ; do \
	  $(mkinstalldirs) $(DESTDIR)$(datadir)/sane/$${dir} ; \
	  done

install-lockpath:
	$(mkinstalldirs) -m 775 -g $(LOCKPATH_GROUP) $(DESTDIR)$(locksanedir)

uninstall-hook:
	@#rm -rf $(DESTDIR)$(libdir)/sane $(DESTDIR)$(configdir) $(DESTDIR)$(locksanedir)
	@#rm -f $(DESTDIR)$(libdir)/libsane.*
	-for dir in $(FIRMWARE_DIRS) ; do \
	  rmdir $(DESTDIR)$(datadir)/sane/$${dir} ; \
	done
	rm -f $(DESTDIR)/usr/share/sane/xsane/sane-@BACKEND_NAME@-logo.xpm

CLEANFILES = $(BACKEND_CONFS) $(be_convenience_libs)
clean-local:
	find . -type l -name \*-s.c | xargs rm -f
	rm -v -f dllsetup

distclean-local:
	for f in `ls *.conf*`; do if test "$$f" != "skel.conf.in"; then rm -v -f $$f; fi; done
	rm -v -f *.la

be_convenience_libs =
be_convenience_libs += lib@BACKEND_NAME@.la
   
be_dlopen_libs =
be_dlopen_libs += libsane-@BACKEND_NAME@.la 

EXTRA_LTLIBRARIES =
#EXTRA_LTLIBRARIES += $(be_convenience_libs)
#EXTRA_LTLIBRARIES += $(be_dlopen_libs)

lib_LTLIBRARIES = 

noinst_LTLIBRARIES = $(be_convenience_libs) $(be_dlopen_libs)

sanelibdir = $(libdir)/sane
sanelib_LTLIBRARIES = 
sanelib_LTLIBRARIES += $(BACKEND_LIBS_ENABLED)

COMMON_LIBS = ../lib/liblib.la

ODC_CPPFLAGS = -DOEM=@OEM@ -DBACKEND_NAME=@BACKEND_NAME@ -fexceptions -I${PWD}/../driver/
ODC_LDFLAGS_ = -fexceptions -L${PWD}/../driver/.libs/

lib@BACKEND_NAME@_la_SOURCES = \
	odc.cpp \
	odc.h \
	odc_device.cpp \
	odc_device.h \
	odc_products.cpp \
	odc_products.h
lib@BACKEND_NAME@_la_CPPFLAGS = $(AM_CPPFLAGS) $(ODC_CPPFLAGS)
lib@BACKEND_NAME@_la_CPPFLAGS += -DPATH_SANE_CONFIG_DIR=\$(configdir)

nodist_libsane_@BACKEND_NAME@_la_SOURCES = odc-s.c
libsane_@BACKEND_NAME@_la_CPPFLAGS = $(AM_CPPFLAGS) $(ODC_CPPFLAGS)
libsane_@BACKEND_NAME@_la_LDFLAGS = $(DIST_SANELIBS_LDFLAGS) $(ODC_LDFLAGS_)
libsane_@BACKEND_NAME@_la_LIBADD = $(COMMON_LIBS) lib@BACKEND_NAME@.la
libsane_@BACKEND_NAME@_la_LIBADD += ../sanei/sanei_init_debug.lo
libsane_@BACKEND_NAME@_la_LIBADD += ../sanei/sanei_config.lo
libsane_@BACKEND_NAME@_la_LIBADD += ../sanei/sanei_constrain_value.lo
libsane_@BACKEND_NAME@_la_LIBADD += ../sanei/sanei_thread.lo
libsane_@BACKEND_NAME@_la_LIBADD += sane_strstatus.lo
libsane_@BACKEND_NAME@_la_LIBADD += $(USB_LIBS) $(PTHREAD_LIBS)
libsane_@BACKEND_NAME@_la_LIBADD += -lstdc++
libsane_@BACKEND_NAME@_la_LIBADD += -l@OEM@utl -l@OEM@scandif

