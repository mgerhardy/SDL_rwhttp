#include <SDL_rwhttp.h>
#include <stdlib.h>
#include <stdio.h>

int main (int argc, char *argv[])
{
	int ret = EXIT_SUCCESS;
	if (SDL_RWHttpInit() == -1) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_RWops* rwops = SDL_RWFromHttpSync("http://www.google.de");
	if (!rwops) {
		fprintf(stderr, "%s\n", SDL_GetError());
		ret = EXIT_FAILURE;
	} else {
		printf("success\n");
	}
	SDL_FreeRW(rwops);

	if (SDL_RWHttpShutdown() == -1) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	return ret;
}
