MAINTAINERCLEANFILES = Makefile.in 

.PHONY: doc

PACKAGE_DOCNAME = $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc

doc-clean:
	rm -rf html/ latex/ $(PACKAGE_DOCNAME).tar*

doc: all
	doxygen
	cp -fr doxygen/* html/ 2>/dev/null

clean-local: doc-clean

EXTRA_DIST = \
Doxyfile \
$(wildcard img/*.*) \
style.css \
head.html \
foot.html \
smman.dox
