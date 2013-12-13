MAINTAINERCLEANFILES += \
src/bin/sds_horodatage/*.gc{no,da}

bin_PROGRAMS += \
src/bin/smman

src_bin_smman_SOURCES = \
src/bin/conf.c \
src/bin/utils.c \
src/bin/logfiles.c \
src/bin/logmessages.c \
src/bin/main.c \
src/bin/rules.c \
src/bin/send.c \
src/bin/spy.c
src_bin_smman_CPPFLAGS = @BIN_CFLAGS@ $(EXTRA_CPPFLAGS)
src_bin_smman_LDFLAGS = @BIN_LIBS@
src_bin_smman_LDADD = \
src/lib/libconf.la
