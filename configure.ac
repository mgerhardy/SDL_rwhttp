AC_INIT(SDL_rwhttp, 0.1.0, mgerhardy@github)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src/SDL_rwhttp.c)
AC_CONFIG_HEADER(src/config.h)

AM_INIT_AUTOMAKE
AC_SUBST(VERSION)

AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_HEADER_STDC
AC_CONFIG_MACRO_DIR([m4])

LT_INIT

AC_ARG_WITH(curl,
	AS_HELP_STRING([--with-curl],
		[use libcurl]
	)
)

SDL2_LIBS=
PKG_CHECK_MODULES(SDL2, sdl2, [AC_DEFINE([HAVE_SDL2],[1],[found sdl 2.x])], [AC_MSG_WARN("sdl 2.x not found")])
if test -z "$SDL2_LIBS"; then
	SDL_LIBS=
	PKG_CHECK_MODULES(SDL, sdl, [AC_DEFINE([HAVE_SDL],[1],[found sdl 1.x])], [AC_MSG_ERROR("sdl 1.x not found")])
fi
AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)
AC_SUBST(SDL2_CFLAGS)
AC_SUBST(SDL2_LIBS)

if test "x${with_curl}" != xno; then
	CURL_LIBS=
	PKG_CHECK_MODULES(CURL, libcurl, [AC_DEFINE([HAVE_CURL],[1],[found libcurl])], [AC_MSG_WARN("libcurl missing")])
else
	AC_MSG_NOTICE(skipping curl)
fi

AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

if test -z "$SDL2_LIBS"; then
	SDL_NET_LIBS=
	PKG_CHECK_MODULES(SDL_NET, SDL_net, [AC_DEFINE([HAVE_SDL_NET],[1],[found SDL_net])], [AC_MSG_WARN("SDL_net missing")])
else
	SDL2_NET_LIBS=
	PKG_CHECK_MODULES(SDL2_NET, SDL2_net, [AC_DEFINE([HAVE_SDL2_NET],[1],[found SDL2_net])], [AC_MSG_WARN("SDL2_net missing")])
fi
AC_SUBST(SDL_NET_CFLAGS)
AC_SUBST(SDL_NET_LIBS)
AC_SUBST(SDL2_NET_CFLAGS)
AC_SUBST(SDL2_NET_LIBS)

AC_OUTPUT(
	Makefile
	src/Makefile
	SDL_rwhttp.pc
)
