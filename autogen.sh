#!/bin/sh

mkdir m4 2>/dev/null 1>&2

autoreconf -f -i
if [ $? -eq 0 ]; then
	./configure
fi
