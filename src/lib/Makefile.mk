MAINTAINERCLEANFILES += src/lib/*.gc{no,da}

noinst_LTLIBRARIES = \
src/lib/libconf.la \
src/lib/libspy.la

src_lib_libconf_la_SOURCES = \
src/lib/conf/conf_main.c \
src/lib/conf/conf_load.c \
src/lib/conf/conf_private.h \
src/include/Conf.h
src_lib_libconf_la_CFLAGS = $(LIBS_CFLAGS) $(EXTRA_CPPFLAGS)
src_lib_libconf_la_LDFLAGS = $(LIBS_CFLAGS)

src_lib_libspy_la_SOURCES = \
src/lib/spy/spy_file.c \
src/lib/spy/spy_main.c \
src/lib/spy/spy_private.h \
src/include/Spy.h
src_lib_libspy_la_CFLAGS = $(LIBS_CFLAGS) $(EXTRA_CPPFLAGS)
src_lib_libspy_la_LDFLAGS = $(LIBS_CFLAGS)

