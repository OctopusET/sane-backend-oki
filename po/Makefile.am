##  Makefile.am -- an automake template for Makefile.in file
##  Copyright (c) 2012 Oki Data Corporation. All rights reserved.
##  Copyright (C) 2009 Chris Bagwell and Sane Developers.
##
##  This file is part of the "Sane" build infra-structure.  See
##  included LICENSE file for license information.

# See po/README for documentation.

MSGFMT = @MSGFMT@
XGETTEXT = @XGETTEXT@
MSGMERGE = @MSGMERGE@ -w75
MSGINIT = @MSGINIT@

#GETTEXT_PACKAGE = @PACKAGE@
GETTEXT_PACKAGE = sane-backend-@BACKEND_NAME@

TMP_FILE_DIR = .tmp

PO_LINGUAS=$(shell \
	if test -r $(srcdir)/LINGUAS; then \
		grep -v "^\#" $(srcdir)/LINGUAS; \
	else \
		if test -r $(top_srcdir)/LINGUAS; then \
			grep -v "^\#" $(top_srcdir)/LINGUAS; \
		else \
			echo ""; \
		fi\
	fi)
SRC_FILES=$(shell if test -r $(srcdir)/POTFILES; then grep -v "^\#" $(srcdir)/POTFILES; else echo ""; fi)

PO_FILES=$(shell LINGUAS="$(PO_LINGUAS)"; for lang in $$LINGUAS; do printf "$$lang.po "; done)
MO_FILES=$(shell LINGUAS="$(PO_LINGUAS)"; for lang in $$LINGUAS; do printf "$$lang.mo "; done)

EXTRA_DIST = README POTFILES
EXTRA_DIST += $(PO_FILES)


if ENABLE_TRANSLATIONS
EXTRA_DIST += $(GETTEXT_PACKAGE).pot
INSTALL_TRANSLATIONS = install-translations
UNINSTALL_TRANSLATIONS = uninstall-translations
ALL = all-mos
else
INSTALL_TRANSLATIONS = 
UNINSTALL_TRANSLATIONS = 
ALL = 
endif

all: $(ALL)

all-mos: $(MO_FILES)

.po.mo:
	@echo "generating $@ from $^"
	@$(MSGFMT) -o $@ $^

install-translations:
	@for lang in $(PO_LINGUAS) ; do \
	  dir=$(localedir)/$${lang}/LC_MESSAGES ; \
	  echo "installing sane-backends $${lang}.mo to $${dir}/$(GETTEXT_PACKAGE).mo..." ; \
	 $(mkinstalldirs) $(DESTDIR)$${dir} && \
	 $(INSTALL_DATA) $${lang}.mo $(DESTDIR)$${dir}/$(GETTEXT_PACKAGE).mo ; \
	 done

install-data-local: $(INSTALL_TRANSLATIONS)

uninstall-translations:
	@for lang in $(PO_LINGUAS) ; do \
	   dir=$(localedir)/$${lang}/LC_MESSAGES ; \
	   echo removing $${dir}/$(GETTEXT_PACKAGE).mo ; \
	   rm -f $(DESTDIR)$${dir}/$(GETTEXT_PACKAGE).mo ; \
	 done

uninstall-local: $(UNINSTALL_TRANSLATIONS)

clean-local:
	rm -f *.mo
	rm -f *.old
	rm -f *.pot

$(GETTEXT_PACKAGE).pot: $(SRC_FILES)
	@echo "creating $@ from $^"
	@$(XGETTEXT) -d$(GETTEXT_PACKAGE) -kBACKEND_I18N --flag=BACKEND_I18N:1:no-c-format -f POTFILES --from-code=UTF-8
	@mv $(GETTEXT_PACKAGE).po $(GETTEXT_PACKAGE).pot

update: $(GETTEXT_PACKAGE).pot
	@$(MSGINIT) --no-translator --input=$(GETTEXT_PACKAGE).pot -o template.po
	@for lang in $(PO_LINGUAS) ; do \
	   pofile=$${lang}.po ; \
	   if test ! -f $${pofile} ; then \
	     echo "creating $${pofile}" ; \
	     cp template.po $${pofile} ; \
	   fi ; \
	   echo "updating $${pofile}" ; \
	   cp $${pofile} $${pofile}.old  ; \
	   $(MSGMERGE) $${pofile}.old $< -o $${pofile} ; \
	   rm -f $${pofile}.old ; \
	 done ; \
	 rm -f template.po

