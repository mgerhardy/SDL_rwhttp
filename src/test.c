#include <SDL_rwhttp.h>
#include <stdlib.h>
#include <stdio.h>

int main (int argc, char *argv[])
{
	int ret = EXIT_SUCCESS;
	SDL_RWHttpInit();

	SDL_RWops* rwops = SDL_RWFromHttpSync("http://www.google.de");
	if (!rwops) {
		fprintf(stderr, "%s\n", SDL_GetError());
		ret = EXIT_FAILURE;
	} else {
		printf("success\n");
	}
	SDL_FreeRW(rwops);

	SDL_RWHttpShutdown();
	return ret;
}
