#include <SDL_rwhttp.h>
#include <stdlib.h>
#include <stdio.h>

// gcc src/test.c `pkg-config --libs libsdl_rwhttp` `pkg-config --cflags libsdl_rwhttp`

int main (int argc, char *argv[])
{
	int ret = EXIT_SUCCESS;
	if (SDL_RWHttpInit() == -1) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	const char *url;
	if (argc == 2) {
		url = argv[1];
	} else {
		url = "http://www.google.de";
	}

	SDL_RWops* rwops = SDL_RWFromHttpSync(url);
	if (!rwops) {
		fprintf(stderr, "%s\n", SDL_GetError());
		ret = EXIT_FAILURE;
	} else {
		printf("success with length: %i\n", (int)SDL_RWsize(rwops));
	}
	SDL_FreeRW(rwops);

	if (SDL_RWHttpShutdown() == -1) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	return ret;
}
