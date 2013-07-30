#include <SDL_rwhttp.h>
#include <stdlib.h>
#include <stdio.h>

// gcc src/test.c `pkg-config --libs SDL_rwhttp` `pkg-config --cflags SDL_rwhttp`

static void read (void **buffer, SDL_RWops *download)
{
	const long n = SDL_RWsize(download);
	size_t remaining = n;
	unsigned char *buf;

	*buffer = NULL;
	if (n <= 0)
		return;
	printf("read %i bytes into a buffer\n", (int)n);
	*buffer = SDL_malloc(n);
	buf = (unsigned char *) *buffer;

	while (remaining) {
		const int readAmount = SDL_RWread(download, buf, 1, remaining);

		if (readAmount == 0) {
			return;
		} else if (readAmount == -1) {
			fprintf(stderr, "failed to read from file\n");
			return;
		}

		remaining -= readAmount;
		buf += readAmount;
	}
}

static void write (SDL_RWops *download)
{
	const char *file = "download";
	void *buffer;
	SDL_RWops *rwops;
	int remaining;
	void *buf;

	rwops = SDL_RWFromFile(file, "wb");
	if (!rwops) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return;
	}

	SDL_RWseek(download, 0, RW_SEEK_SET);

	read(&buffer, download);
	if (!buffer) {
		SDL_FreeRW(rwops);
		fprintf(stderr, "Could not read download rwops\n");
		return;
	}

	SDL_RWseek(download, 0, RW_SEEK_SET);

	remaining = SDL_RWsize(download);
	buf = buffer;
	while (remaining) {
		const size_t written = SDL_RWwrite(rwops, buf, 1, remaining);
		if (written == 0) {
			SDL_FreeRW(rwops);
			SDL_free(buffer);
			fprintf(stderr, "failed to write the file %s\n", file);
			return;
		}

		remaining -= written;
		buf += written;
	}

	SDL_FreeRW(rwops);
	SDL_free(buffer);
	printf("wrote to file %s\n", file);
}

int main (int argc, char *argv[])
{
	int ret = EXIT_SUCCESS;
	const char *url;
	SDL_RWops* rwops;

	if (SDL_RWHttpInit() == -1) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	if (argc == 2) {
		url = argv[1];
	} else {
		url = "http://www.google.de";
	}

	rwops = SDL_RWFromHttpSync(url);
	if (!rwops) {
		fprintf(stderr, "%s\n", SDL_GetError());
		ret = EXIT_FAILURE;
	} else {
		printf("success with length: %i\n", (int) SDL_RWsize(rwops));
	}

	write(rwops);

	SDL_FreeRW(rwops);

	if (SDL_RWHttpShutdown() == -1) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	return ret;
}
