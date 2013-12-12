MAINTAINERCLEANFILES += \
src/bin/sds_horodatage/*.gc{no,da}

bin_PROGRAMS += \
src/bin/smman

src_bin_smman_SOURCES = \
src/bin/conf.c \
src/bin/date.c \
src/bin/libconf.c \
src/bin/logfiles.c \
src/bin/logmessages.c \
src/bin/main.c \
src/bin/rules.c \
src/bin/send.c \
src/bin/spy.c
src_bin_smman_CPPFLAGS = @BIN_CFLAGS@
src_bin_smman_LDFLAGS = @BIN_LIBS@
