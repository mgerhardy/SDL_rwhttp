lib_LTLIBRARIES = libsdl_rwhttp.la

libsdl_rwhttp_la_SOURCES = \
	SDL_rwhttp.c

include_HEADERS = \
	SDL_rwhttp.h

EXTRA_DIST =

libsdl_rwhttp_la_CFLAGS = -Wall @CURL_CFLAGS@ @SDL_CFLAGS@

libsdl_rwhttp_la_LDFLAGS = -no-undefined -release @VERSION@ -lm

libsdl_rwhttp_la_LIBADD = @CURL_LIBS@ @SDL_LIBS@
