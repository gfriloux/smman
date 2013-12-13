MAINTAINERCLEANFILES += src/lib/*.gc{no,da}

noinst_LTLIBRARIES = \
src/lib/libconf.la

src_lib_libconf_la_SOURCES = \
src/lib/conf.c \
src/include/Conf.h
src_lib_libconf_la_CFLAGS = $(LIBS_CFLAGS) $(EXTRA_CPPFLAGS)
src_lib_libconf_la_LDFLAGS = $(LIBS_CFLAGS)
