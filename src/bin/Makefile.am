MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = smman

smman_SOURCES  = main.c rules.c spy.c logfiles.c send.c date.c conf.c logmessages.c libconf.c

smman_LDFLAGS = @eina_LIBS@ @ecore_LIBS@ @libcurl_LIBS@
smman_CPPFLAGS = -W -Wall -O2 -g @eina_CFLAGS@ @ecore_CFLAGS@ @libcurl_CFLAGS@ -DREVISION=\"@revision_number@\" -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS
