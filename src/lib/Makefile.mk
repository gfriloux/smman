MAINTAINERCLEANFILES += src/lib/*.gc{no,da}

noinst_LTLIBRARIES = \
src/lib/libconf.la \
src/lib/libspy.la \
src/lib/librules.la \
src/lib/libstore.la \
src/lib/libcjson.la

src_lib_libconf_la_SOURCES = \
src/lib/conf/conf_main.c \
src/lib/conf/conf_load.c \
src/lib/conf/conf_private.h \
src/include/Conf.h
src_lib_libconf_la_CFLAGS = $(LIBS_CFLAGS) $(EXTRA_CPPFLAGS)
src_lib_libconf_la_LDFLAGS = $(LIBS_LIBS)

src_lib_libspy_la_SOURCES = \
src/lib/spy/spy_main.c \
src/lib/spy/spy_file.c \
src/lib/spy/spy_line.c \
src/lib/spy/spy_private.h \
src/include/Spy.h
src_lib_libspy_la_CFLAGS = $(LIBS_CFLAGS) $(EXTRA_CPPFLAGS)
src_lib_libspy_la_LDFLAGS = $(LIBS_LIBS)

src_lib_librules_la_SOURCES = \
src/lib/rules/rules_main.c \
src/lib/rules/rules_load.c \
src/lib/rules/rules_private.h \
src/include/Rules.h
src_lib_librules_la_CFLAGS = $(LIBS_CFLAGS) $(EXTRA_CPPFLAGS)
src_lib_librules_la_LDFLAGS = $(LIBS_LIBS)
src_lib_librules_la_DEPENDENCIES = \
src/lib/libconf.la
src_lib_librules_la_LIBADD = \
src/lib/libconf.la

src_lib_libstore_la_SOURCES = \
src/lib/store/store_main.c \
src/lib/store/store_event.c \
src/lib/store/store_utils.c \
src/lib/store/store_private.h \
src/include/Store.h
src_lib_libstore_la_CFLAGS = $(LIBS_CFLAGS) $(EXTRA_CPPFLAGS)
src_lib_libstore_la_LDFLAGS = $(LIBS_LIBS)

src_lib_libcjson_la_SOURCES = \
src/lib/extras/cJSON.c
src_lib_libcjson_la_CFLAGS = $(LIBS_CFLAGS) $(EXTRA_CPPFLAGS)
src_lib_libcjson_la_LDFLAGS = $(LIBS_LIBS)
