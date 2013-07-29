lib_LTLIBRARIES = libSDL_rwhttp.la

libSDL_rwhttp_la_SOURCES = \
	SDL_rwhttp.c

include_HEADERS = \
	SDL_rwhttp.h

EXTRA_DIST =

libSDL_rwhttp_la_CFLAGS = -Wall @CURL_CFLAGS@ @SDL_NET_CFLAGS@ @SDL2_NET_CFLAGS@ @SDL_CFLAGS@ @SDL2_CFLAGS@

libSDL_rwhttp_la_LDFLAGS = -no-undefined -release @VERSION@ -lm

libSDL_rwhttp_la_LIBADD = @CURL_LIBS@ @SDL_NET_LIBS@ @SDL2_NET_LIBS@ @SDL_LIBS@ @SDL2_LIBS@
