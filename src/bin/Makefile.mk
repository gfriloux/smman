MAINTAINERCLEANFILES += \
src/bin/*.gc{no,da}

bin_PROGRAMS += \
src/bin/smman

src_bin_smman_SOURCES = \
src/bin/main.c \
src/bin/config.c \
src/bin/smman.h
src_bin_smman_CPPFLAGS = @BIN_CFLAGS@ $(EXTRA_CPPFLAGS)
src_bin_smman_LDFLAGS = @BIN_LIBS@
src_bin_smman_LDADD = \
src/lib/libconf.la
