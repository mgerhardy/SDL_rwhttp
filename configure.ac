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

SDL_LIBS=
PKG_CHECK_MODULES(SDL, sdl2, [AC_DEFINE([HAVE_SDL],[1],[found sdl 2.x])], [AC_MSG_ERROR("sdl 2.x not found")])
if test -z "$SDL_LIBS"; then
	SDL_LIBS=
	PKG_CHECK_MODULES(SDL, sdl, [AC_DEFINE([HAVE_SDL],[1],[found sdl 1.x])], [AC_MSG_WARN("sdl 1.x not found")])
fi

AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)

CURL_LIBS=
PKG_CHECK_MODULES(CURL, libcurl, [AC_DEFINE([HAVE_CURL],[1],[found libcurl])], [AC_MSG_ERROR("libcurl missing")])
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

AC_OUTPUT(
	Makefile
	src/Makefile
	libsdl_rwhttp.pc
)
