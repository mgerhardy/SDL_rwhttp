#ifndef _SDL_RWHTTP_
#define _SDL_RWHTTP_

#include <SDL.h>

#define SDL_RWHTTP_MAJOR_VERSION   0
#define SDL_RWHTTP_MINOR_VERSION   1
#define SDL_RWHTTP_PATCHLEVEL      0

#define SDL_RWHTTP_VERSION(x) \
{ \
    (x)->major = SDL_RWHTTP_MAJOR_VERSION; \
    (x)->minor = SDL_RWHTTP_MINOR_VERSION; \
    (x)->patch = SDL_RWHTTP_PATCHLEVEL;    \
}

#define SDL_RWOPS_HTTP 404

/**
 *  \brief A variable that lets you specify the user agent the requests are using.
 */
#define SDL_RWHTTP_HINT_USER_AGENT "SDL_RWHTTP_USERAGENT"

/**
 *  \brief Initializes the library. Should only be called once per application.
 *
 *  \return -1 if any error occurred, 0 otherwise.
 */
int SDL_RWHttpInit (void);

/*
 * \brief Cleanup the library.
 *
 *  \return -1 if any error occurred, 0 otherwise.
 */
int SDL_RWHttpShutdown (void);

/**
 *  \name RWFrom functions
 *
 *  Functions to create SDL_RWops structures from http streams.
 */
SDL_RWops* SDL_RWFromHttpAsync (const char *uri);
SDL_RWops* SDL_RWFromHttpSync (const char *uri);

#endif
