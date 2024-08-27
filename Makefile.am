##  Makefile.am -- an automake template for Makefile.in file
##  Copyright (c) 2012 Oki Data Corporation. All rights reserved.
##  Copyright (C) 2009 Chris Bagwell and Sane Developers.
##
##  This file is part of the "Sane" build infra-structure.  See
##  included LICENSE file for license information.

SUBDIRS = lib sanei backend po
DIST_SUBDIRS = include lib sanei backend po

#dist_doc_DATA = AUTHORS ChangeLog COPYING LICENSE NEWS PROBLEMS PROJECTS README
EXTRA_DIST = COPYING
EXTRA_DIST += $(PACKAGE).lsm
EXTRA_DIST += sane-backend.lsm.skel

#
# Keep the .cvsignore files sorted, and use this target to do it.
#
PERL=perl
sort-cvsignore:
	for f in `find . -name .cvsignore`; do \
	  $(PERL) -e 'print sort <>;' < $$f > $$f.tmptmp; \
	  mv $$f.tmptmp $$f; \
	done


#
# Check to make sure only sane_ and sanei_ symbols are exported from
# the libraries
#
libcheck:
	@echo "Libraries exporting 'illegal' symbols:"
	@for lib in backend/.libs/*.so backend/.libs/*.a; do \
	lines=`nm -g $$lib|grep '\( T \)\|\( D \)'|egrep -v ' _fini| _init'|egrep -v ' sane_| sanei_'`; \
	if test -n "$$lines" ; then \
	  echo -e "*** $$lib:\n$$lines"; \
	  found_errors="yes"; \
	fi \
	done; \
	echo; \
	echo "Libraries exporting 'non-standard sane_*' symbols:"; \
	for lib in backend/.libs/*.so backend/.libs/*.a; do \
	lines=`nm -g $$lib|grep '\( T \)\|\( D \)'|egrep ' sane_' | egrep -v 'sane_.*init|sane_.*exit|sane_.*get_devices|sane_.*open|sane_.*close|sane_.*get_option_descriptor|sane_.*control_option|sane_.*get_parameters|sane_.*start|sane_.*read|sane_.*cancel|sane_.*set_io_mode|sane_.*get_select_fd|sane_strstatus'` ; \
	if test -n "$$lines" ; then \
	  echo -e "*** $$lib:\n$$lines"; \
	  found_errors="yes"; \
	fi \
	done;
	if test ! -z $$found_errors ; then \
	  exit 1; \
	fi

# Verify that backends do not contain any unwanted public symbols
# anytime check or distcheck is ran.
# TODO: Currently this doesn't pass.  Uncomment once symbols have
# been cleaned up.
#check-am: libcheck

# Run testsuite any time installcheck or distcheck is ran.
# Must run testsuite after install or else shared library backends
# will not be found (including test backend).
installcheck-local:
	cd testsuite && $(MAKE) test.local

$(distdir).lsm: dist $(PACKAGE).lsm
	( cat $(PACKAGE).lsm | \
	  sed -e "s|_DATE_|`date +%d%b%y`|g" |\
	  sed -e "s|_VERSION_|$(VERSION)|g" | \
	  sed -e "s|_OEM_|$(OEM)|g" | \
	  sed -e "s|_BACKEND_NAME_|$(BACKEND_NAME)|g" | \
	  sed -e "s|_T_S_|`find $(distdir).tar.gz -printf "%4k"`|g" |\
	  sed -e "s|_L_S_|`find $(PACKAGE).lsm -printf "%4k"`|g" >\
	$(distdir).lsm \
	)
lsm: $(distdir).lsm

md5: dist
	md5sum $(distdir).tar.gz > $(distdir).tar.gz.md5

sane-backends: lsm md5

# Clean files created by custom targets in this file.
clean-local:
	rm -f sane-backends-*.lsm
	rm -f $(distdir).tar.gz.md5

# Clean files created by configure
distclean-local:
	rm -f include/_stdint.h
	rm -f include/byteorder.h
	rm -fv *.tar.gz

#[Private]
redistsrc: dist
	@mkdir redistsrc.tmp
	@cd backend/; \
	bksrc=`ls odc.* odc_*`; \
	echo "Backend sources: "$$bksrc; \
	cd ..; \
	cd redistsrc.tmp; \
	tar zxvf ../$(distdir).tar.gz; \
	cd $(distdir); \
	pwd; \
	rm -vf Makefile.am Makefile.in confirure.in AUTHORS ChangeLog INSTALL LICENSE NEWS PROJECTS PROBLEMS README; \
	rm -vf cmcut.pl recfin.pl remkam.pl; \
	rm -vf LINGUAS LANGUAGES LANGID DEFDOCLANG; \
	rm -vf sane-backend.lsm.skel; \
	rm -vf *.lsm; \
	rm -vf backend/odc*; \
	rm -vf po/*.pot; \
	rm -vfr po.driver; \
	rm -vfr po.utils; \
	rm -vfr doc; \
	rm -vfr doc.backend; \
	rm -vfr doc.driver; \
	rm -vfr doc.utils; \
	rm -vfr utils; \
	rm -vfr debian; \
	rm -vfr rpm; \
	rm -vfr licenses; \
	rm -vfr py-compile; \
	rm -vfr driver; \
	echo "Modify Makefile.am Makefile.in configure.in"; \
	../../remkam.pl ../../Makefile.am > Makefile.am; \
	../../recfin.pl ../../configure.in > configure.in; \
	mv po po; \
	touch AUTHORS ChangeLog NEWS README; \
	automake -a -Wno-all; \
	autoconf; \
	rm -rf autom4te.cache; \
	mkdir -p driver/devel; \
	sed -e "s|_BACKEND_NAME_|$(BACKEND_NAME)|g" ../../sane-backend.lsm.skel > sane-backend-$(BACKEND_NAME).lsm; \
	for f in $$bksrc; do \
		echo "Copy(cmcut) "$$f; cat ../../backend/$$f | ../../cmcut.pl > backend/$$f; \
	done; \
	for f in `ls ../../driver/devel/`; do \
		echo "Copy(cmcut) "$$f; cat ../../driver/devel/$$f | ../../cmcut.pl > driver/devel/$$f; \
	done; \
	langs=`grep -v "^\#" ../../licenses/LICENSE_LINGUAS`; \
	for l in $$langs; do \
		cp -v ../../licenses/License_$$l.txt driver/copying.oki.$$l; \
	done;
	@cd redistsrc.tmp/$(distdir); \
	tball=sane-backend-$(BACKEND_NAME)-$(V_SANE_BACKEND_MAJOR).$(V_SANE_BACKEND_MINOR).$(V_SANE_BACKEND_REV)-src.tar.gz; \
	echo "Creating $$tball"; \
	tar zcf ../../$$tball *;
	@rm -rf redistsrc.tmp


.PHONY: deb
deb:
	dpkg-buildpackage -us -uc -rfakeroot -b

.PHONY: rpm
rpm:
	@cd rpm; \
	make rpm

init:
	@cd rpm; make clean init;
	@cd debian; make clean init;
	@cd pardus; make clean init;
	@rm -fv changelog readme

initdoc:
	@cd doc.driver; make init
	@cd doc.utils; make init
	@cd doc.backend; make init




EXTRA_DIST += AUTHORS ChangeLog LICENSE NEWS PROBLEMS PROJECTS README
EXTRA_DIST += LINGUAS
EXTRA_DIST += LANGID
EXTRA_DIST += LANGUAGES
EXTRA_DIST += DEFDOCLANG
EXTRA_DIST += licenses
EXTRA_DIST += cmcut.pl
EXTRA_DIST += remkam.pl
EXTRA_DIST += recfin.pl
EXTRA_DIST += changelog.in
EXTRA_DIST += readme.in

changelog:
	@echo Generating $@ from $@.in
	@sed -e 's|@PACKAGENAME@|$(PACKAGE)|g' \
	     -e 's|@OEMNAME@|$(OEM)|g' \
	     -e 's|@BACKENDNAME@|$(BACKEND_NAME)|g' \
	     -e 's|@DATADIR@|$(datadir)|g' \
	     -e 's|@CONFIGDIR@|$(configdir)|g' \
	     -e 's|@SYSCONFIGDIR@|$(sysconfdir)|g' \
	     -e 's|@LIBEXECDIR@|$(libexecdir)|g' \
	     -e 's|@DOCDIR@|$(docdir)|g' \
	     -e 's|@LIBDIR@|$(libdir)|g' \
	     -e 's|@BINDIR@|$(bindir)|g' \
	     -e 's|@SBINDIR@|$(sbindir)|g' \
	     -e 's|@NUMBERVERSION@|$(NUMBER_VERSION)|g' \
	     -e 's|@DRIVERVERSION@|$(DRIVER_VERSION)|g' \
	     -e 's|@UTILSVERSION@|$(UTILS_VERSION)|g' \
	     -e 's|@SANEBACKENDVERSION@|$(SANE_BACKEND_VERSION)|g' $@.in > $@

readme: changelog
	@echo Generating $@ from $@.in
	@sed -e 's|@PACKAGENAME@|$(PACKAGE)|g' \
	     -e 's|@OEMNAME@|$(OEM)|g' \
	     -e 's|@BACKENDNAME@|$(BACKEND_NAME)|g' \
	     -e 's|@NUMBERVERSION@|$(NUMBER_VERSION)|g' \
	     -e 's|@DRIVERVERSION@|$(DRIVER_VERSION)|g' \
	     -e 's|@UTILSVERSION@|$(UTILS_VERSION)|g' \
	     -e 's|@SANEBACKENDVERSION@|$(SANE_BACKEND_VERSION)|g' $@.in > $@
	@cat changelog >> $@




LICENSE_DIR = $(top_srcdir)/licenses
LICENSE_LINGUAS=$(shell if test -r $(LICENSE_DIR)/LICENSE_LINGUAS; then grep -v "^\#" $(LICENSE_DIR)/LICENSE_LINGUAS; else echo ""; fi)
LICENSE_INSTALL_DIRS=usr/share/doc/$(OEM)$(PACKAGE)

install-license-doc: changelog readme
	test -z "$(DESTDIR)/usr/share/doc/$(OEM)$(PACKAGE)/" || $(MKDIR_P) "$(DESTDIR)/usr/share/doc/$(OEM)$(PACKAGE)/" ; \
	cp COPYING $(DESTDIR)/usr/share/doc/$(OEM)$(PACKAGE)/COPYING
	cp readme $(DESTDIR)/usr/share/doc/$(OEM)$(PACKAGE)/README
	for lang in $(LICENSE_LINGUAS) ; do \
		if test -r $(LICENSE_DIR)/*$${lang}*; then\
			f=`ls $(LICENSE_DIR)/*$${lang}* | head -1`;\
			cp $${f} $(DESTDIR)/usr/share/doc/$(OEM)$(PACKAGE)/copying.$(OEM).$${lang}; \
		fi; \
	done




RELEASEFILE=/etc/lsb-release
DISTRIB=$(shell if test -f $(RELEASEFILE); then . $(RELEASEFILE); echo $$DISTRIB_ID; else echo "UNKNOWN"; fi)

install-exec-hook:
	if test $(DISTRIB) = "Pardus"; then \
		make install-license-doc; \
	fi


