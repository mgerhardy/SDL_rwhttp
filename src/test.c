#include <SDL_rwhttp.h>
#include <stdlib.h>
#include <stdio.h>

// gcc src/test.c `pkg-config --libs --cflags SDL_rwhttp`

#if !SDL_VERSION_ATLEAST(2, 0, 0)
static int SDL_RWsize (SDL_RWops *rwops)
{
	int size;
	const int pos = SDL_RWseek(rwops, 0, RW_SEEK_CUR);
	if (pos < 0) {
		return -1;
	}
	size = SDL_RWseek(rwops, 0, RW_SEEK_END);
	SDL_RWseek(rwops, pos, RW_SEEK_SET);
	return size;
}
#endif

static void read (void **buffer, SDL_RWops *download)
{
	const long n = SDL_RWsize(download);
	size_t remaining = n;
	unsigned char *buf;

	*buffer = NULL;
	if (n <= 0)
		return;

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
		SDL_RWclose(rwops);
		fprintf(stderr, "Could not read download rwops\n");
		return;
	}

	SDL_RWseek(download, 0, RW_SEEK_SET);

	remaining = SDL_RWsize(download);
	buf = buffer;
	while (remaining) {
		const size_t written = SDL_RWwrite(rwops, buf, 1, remaining);
		if (written == 0) {
			SDL_RWclose(rwops);
			SDL_free(buffer);
			fprintf(stderr, "failed to write the file %s\n", file);
			return;
		}

		remaining -= written;
		buf += written;
	}

	SDL_RWclose(rwops);
	SDL_free(buffer);
}

static int download (const char *url)
{
	SDL_RWops* rwops = SDL_RWFromHttpSync(url);
	if (!rwops) {
		fprintf(stderr, "%s: %s\n", url, SDL_GetError());
		return EXIT_FAILURE;
	}
	printf("%s: success with length: %i\n", url, (int) SDL_RWsize(rwops));
	write(rwops);
	SDL_RWclose(rwops);
	return EXIT_SUCCESS;
}

int main (int argc, char *argv[])
{
	int ret = EXIT_SUCCESS;
	char **urls;
	int urlCount;
	char *token;
	char *state;
	char *parse;
	int i;

	if (SDL_RWHttpInit() == -1) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	if (argc >= 2) {
		urlCount = argc - 1;
		urls = (char **)malloc(sizeof(char**) * urlCount);
		for (i = 1; i < argc; ++i)
			urls[i - 1] = strdup(argv[i]);
	} else {
		urlCount = 1;
		urls = (char **)malloc(sizeof(char**) * urlCount);
		urls[0] = strdup("http://www.google.de");
	}
	for (i = 0; i < urlCount; ++i) {
		ret = download(urls[i]);
		if (ret != EXIT_SUCCESS)
			break;
	}

	for (i = 0; i < urlCount; ++i) {
		free(urls[i]);
	}
	free(urls);

	if (SDL_RWHttpShutdown() == -1) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	return ret;
}
