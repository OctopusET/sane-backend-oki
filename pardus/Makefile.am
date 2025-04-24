##  Makefile.am -- an automake template for Makefile.in file
##  Copyright (c) 2012 Oki Data Corporation. All rights reserved.

AM_CPPFLAGS = 

bin_PROGRAMS =
noinst_PROGRAMS = 

bin_SCRIPTS = 
noinst_SCRIPTS = 
BUILT_SOURCES = 
CLEANFILES = $(bin_SCRIPTS) $(dist_noinst_SCRIPTS)

VERSION=@NUMBER_VERSION@

CURDIR=$(shell pwd)

EXTRA_DIST =

LINGUAS=$(shell \
	if test -r $(srcdir)/LINGUAS; then \
		grep -v "^\#" $(srcdir)/LINGUAS; \
	else \
		if test -r $(top_srcdir)/LINGUAS; then \
			grep -v "^\#" $(top_srcdir)/LINGUAS; \
		else \
			echo ""; \
		fi\
	fi)

LIBDIR=$(shell if test "`arch`" = "x86_64" ; then echo "/usr/lib64"; else echo "/usr/lib"; fi)


.py.in.py:
	@echo Generating $@ from $^
	@sed -e 's|@OEMNAME@|$(OEM)|g' \
	     -e 's|@BACKENDNAME@|$(BACKEND_NAME)|g' \
	     -e 's|@DATADIR@|$(datadir)|g' \
	     -e 's|@CONFIGDIR@|$(configdir)|g' \
	     -e 's|@DOCDIR@|$(docdir)|g' \
	     -e 's|@LIBDIR@|$(libdir)/sane|g' \
	     -e 's|@BINDIR@|$(bindir)|g' \
	     -e 's|@SBINDIR@|$(sbindir)|g' \
	     -e 's|@LIBEXECDIR@|$(libexecdir)|g' \
	     -e 's|@MAJOR@|$(V_MAJOR)|g' \
	     -e 's|@MINOR@|$(V_MINOR)|g' \
	     -e 's|@REV@|$(V_REV)|g' \
	     -e 's|@NUMBERVERSION@|$(NUMBER_VERSION)|g' $? > $@
	@chmod +x $@


pspec.xml:
	@echo Generating $@ from $@.in
	@DIST="../mfpsdrv-$(NUMBER_VERSION).tar.gz"; \
	test -f $$DIST || (cd ..; make dist); \
	SHA1SUM="`sha1sum $$DIST | awk '{print $$1}'`"; \
	sed  -e 's|@OEMNAME@|$(OEM)|g' \
	     -e "s|@SHA1SUM@|$$SHA1SUM|g" \
	     -e "s|@DIST@|$$DIST|g" \
	     -e 's|@DATADIR@|$(datadir)|g' \
	     -e 's|@CONFIGDIR@|$(configdir)|g' \
	     -e 's|@DOCDIR@|$(docdir)|g' \
	     -e 's|@LIBDIR@|$(libdir)/sane|g' \
	     -e 's|@BINDIR@|$(bindir)|g' \
	     -e 's|@SBINDIR@|$(sbindir)|g' \
	     -e 's|@LIBEXECDIR@|$(libexecdir)|g' \
	     -e 's|@MAJOR@|$(V_MAJOR)|g' \
	     -e 's|@MINOR@|$(V_MINOR)|g' \
	     -e 's|@REV@|$(V_REV)|g' \
	     -e 's|@NUMBERVERSION@|$(NUMBER_VERSION)|g' $@.in > $@




EXTRA_DIST += package.py.in
EXTRA_DIST += service.py.in
EXTRA_DIST += actions.py.in
EXTRA_DIST += pspec.xml.in




all-am: build-files

# remove sub-directories
clean-local:
	@for d in `ls`; do if test -d $$d; then rm -r -f -v $$d; fi; done;
	@rm -rfv buildroot
	@rm -rfv RPMS
	@rm -fv README

distclean-local:


build-files: package.py service.py actions.py pspec.xml
	@test -z "files" || $(MKDIR_P) "files"
	@test -z "comar" || $(MKDIR_P) "comar"
	@cp package.py comar/
	@cp service.py comar/


pisi:
	pisi bi


init:
	@rm -fv package.py service.py
	@rm -fv actions.py pspec.xml
	@rm -rfv comar
	@rm -rfv files
	