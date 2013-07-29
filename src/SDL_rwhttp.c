#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CURL
#include <curl/curl.h>
#endif
#if defined(HAVE_SDL_NET) || defined(HAVE_SDL2_NET)
#include <SDL_net.h>
#endif
#include "SDL_rwhttp.h"

#define STRINGIFY(x) #x

static const char *userAgent;
static int connectTimeout;
static int timeout;
static int fetchLimit;

int SDL_RWHttpShutdown (void)
{
#ifdef HAVE_CURL
	curl_global_cleanup();
#endif
	return 0;
}

int SDL_RWHttpInit (void)
{
	const char *hint;

	userAgent = SDL_GetHint(SDL_RWHTTP_HINT_USER_AGENT);
	if (!userAgent)
		userAgent = "SDL_rwhttp/" STRINGIFY(SDL_RWHTTP_MAJOR_VERSION) "." STRINGIFY(SDL_RWHTTP_MINOR_VERSION);

	hint = SDL_GetHint(SDL_RWHTTP_HINT_CONNECTTIMEOUT);
	if (hint)
		connectTimeout = atoi(hint);
	else
		connectTimeout = 3;

	hint = SDL_GetHint(SDL_RWHTTP_HINT_TIMEOUT);
	if (hint)
		timeout = atoi(hint);
	else
		timeout = 3;

	hint = SDL_GetHint(SDL_RWHTTP_HINT_FETCHLIMIT);
	if (hint)
		fetchLimit = atoi(hint);
	else
		fetchLimit = 1024 * 1024 * 5;

#ifdef HAVE_CURL
	{
		const CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
		if (result == CURLE_OK) {
			return 0;
		}
		SDL_SetError(curl_easy_strerror(result));
	}
#else
#if defined(HAVE_SDL_NET) || defined(HAVE_SDL2_NET)
	{
		if (SDLNet_Init() >= 0)
			/* The sdl error is already set */
			return 0;
		return -1;
	}
#endif
#endif

	SDL_SetError("Required library is missing");
	return -1;
}

typedef struct {
	char *uri;
	char *data;
	size_t size;
	size_t expectedSize;
	void *userData;
	Uint8 *base;
	Uint8 *here;
	Uint8 *stop;
} http_data_t;

static int SDLCALL http_close (SDL_RWops * context)
{
	int status = 0;
	if (!context) {
		return status;
	}

	if (context->hidden.unknown.data1) {
		http_data_t *data = (http_data_t*) context->hidden.unknown.data1;
#ifdef HAVE_CURL
		CURL *curl = (CURL*)data->userData;
		curl_easy_cleanup(curl);
#endif
		SDL_free(data->data);
		SDL_free(data);
	}
	SDL_FreeRW(context);
	return status;
}

static Sint64 SDLCALL http_size (SDL_RWops * context)
{
	const http_data_t *data = (const http_data_t*) context->hidden.unknown.data1;
	return (Sint64)(data->stop - data->base);
}

static Sint64 SDLCALL http_seek (SDL_RWops * context, Sint64 offset, int whence)
{
	http_data_t *data = (http_data_t*) context->hidden.unknown.data1;
	Uint8 *newpos;

	switch (whence) {
	case RW_SEEK_SET:
		newpos = data->base + offset;
		break;
	case RW_SEEK_CUR:
		newpos = data->here + offset;
		break;
	case RW_SEEK_END:
		newpos = data->stop + offset;
		break;
	default:
		return SDL_SetError("Unknown value for 'whence'");
	}
	if (newpos < data->base) {
		newpos = data->base;
	}
	if (newpos > data->stop) {
		newpos = data->stop;
	}
	data->here = newpos;
	return (Sint64)(data->here - data->base);
}

static size_t SDLCALL http_read (SDL_RWops * context, void *ptr, size_t size, size_t maxnum)
{
	http_data_t *data = (http_data_t*) context->hidden.unknown.data1;
	size_t total_bytes;
	size_t mem_available;

	total_bytes = (maxnum * size);
	if (maxnum <= 0 || size <= 0 || total_bytes / maxnum != (size_t) size) {
		return 0;
	}

	mem_available = data->stop - data->here;
	if (total_bytes > mem_available) {
		total_bytes = mem_available;
	}

	SDL_memcpy(ptr, data->here, total_bytes);
	data->here += total_bytes;

	return total_bytes / size;
}

static size_t SDLCALL http_writeconst (SDL_RWops * context, const void *ptr, size_t size, size_t num)
{
	SDL_SetError("Can't write to read-only memory");
	return 0;
}

#ifdef HAVE_CURL
static size_t curlHttpWriteSync (void *streamData, size_t size, size_t nmemb, void *userData)
{
	const size_t realsize = size * nmemb;
	http_data_t *httpData = (http_data_t *) userData;

	if (httpData->expectedSize == 0) {
		const int newSize = httpData->size + realsize;
		if (newSize > fetchLimit) {
			SDL_SetError("file exceeded the hardcoded limit of %i (%i)", fetchLimit, (int)newSize);
			return 0;
		}
		httpData->data = SDL_realloc(httpData->data, newSize);
		if (httpData->data == NULL) {
			SDL_SetError("not enough memory (realloc returned NULL)");
			return 0;
		}
	} else if (httpData->size + realsize > httpData->expectedSize) {
		SDL_SetError("illegal Content-Length - buffer overflow");
		return 0;
	}

	SDL_memcpy(&(httpData->data[httpData->size]), streamData, realsize);
	httpData->size += realsize;

	return realsize;
}

size_t curlHttpHeader (void *headerData, size_t size, size_t nmemb, void *userData)
{
	const char *header = (const char *)headerData;
	const size_t bytes = size * nmemb;
	const char *contentLength = "Content-Length: ";
	const size_t strLength = strlen(contentLength);

	if (bytes <= strLength) {
		return bytes;
	}

	if (!SDL_strncasecmp(header, contentLength, strLength)) {
		http_data_t *httpData = (http_data_t *) userData;
		httpData->expectedSize = SDL_strtoul(header + strLength, NULL, 10);
		if (httpData->expectedSize == 0) {
			SDL_SetError("invalid content length given: %i", (int)httpData->expectedSize);
			return 0;
		}
		if (httpData->expectedSize > fetchLimit) {
			SDL_SetError("content length exceeded the hardcoded limit of %i (%i)", fetchLimit, (int)httpData->expectedSize);
			return 0;
		}
		httpData->data = SDL_malloc(httpData->expectedSize);
		if (httpData->data == NULL) {
			SDL_SetError("not enough memory (malloc returned NULL)");
			return 0;
		}
	}

	return bytes;
}
#endif

static SDL_RWops* SDL_RWHttpCreate (http_data_t *httpData)
{
	SDL_RWops *rwops = SDL_AllocRW();
	if (!rwops)
		return NULL;

	rwops->size = http_size;
	rwops->seek = http_seek;
	rwops->read = http_read;
	rwops->write = http_writeconst;
	rwops->close = http_close;

	httpData->base = (Uint8*) httpData->data;
	httpData->here = httpData->base;
	httpData->stop = httpData->base + httpData->size;

	rwops->hidden.unknown.data1 = httpData;
	rwops->type = SDL_RWOPS_HTTP;
	return rwops;
}

SDL_RWops* SDL_RWFromHttpSync (const char *uri)
{
	SDL_RWops *rwops;
	http_data_t *httpData;
#ifdef HAVE_CURL
	CURL* curlHandle;
	CURLcode result;
#else
#if defined(HAVE_SDL_NET) || defined(HAVE_SDL2_NET)
	IPaddress ip;
	int port = 80;
	TCPsocket socket;
#endif
#endif

	if (!uri || uri[0] == '\0') {
		SDL_SetError("No uri given");
		return NULL;
	}

	httpData = SDL_malloc(sizeof(*httpData));
	SDL_zerop(httpData);

#ifdef HAVE_CURL
	curlHandle = curl_easy_init();
	curl_easy_setopt(curlHandle, CURLOPT_URL, uri);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, curlHttpWriteSync);
	curl_easy_setopt(curlHandle, CURLOPT_HEADERFUNCTION, curlHttpHeader);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, (void * )httpData);
	curl_easy_setopt(curlHandle, CURLOPT_HEADERDATA, (void * )httpData);
	curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, userAgent);
	curl_easy_setopt(curlHandle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curlHandle, CURLOPT_MAXREDIRS, 5);
	curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curlHandle, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(curlHandle, CURLOPT_CONNECTTIMEOUT, connectTimeout);
	curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, timeout);

	httpData->userData = curlHandle;

	result = curl_easy_perform(curlHandle);
	if (result != CURLE_OK) {
		SDL_SetError(curl_easy_strerror(result));
		return NULL;
	}
#else
#if defined(HAVE_SDL_NET) || defined(HAVE_SDL2_NET)
	if (SDL_strstr(uri, "://") != NULL)
		uri = SDL_strstr(uri, "://") + 3;
	if (SDL_strchr(uri, ':')) {
		port = atoi(SDL_strchr(uri, ':') + 1);
	}
	if (SDLNet_ResolveHost(&ip, uri, port) < 0) {
		/* sdl error is already set */
		return NULL;
	}

	if (!(socket = SDLNet_TCP_Open(&ip))) {
		return NULL;
	} else {
		const size_t bufSize = 512;
		Uint8 *buf;
		const int length = 80 + strlen(uri) + strlen(userAgent);
		char *request = SDL_malloc(length);
		if (request == NULL) {
			SDL_SetError("not enough memory (malloc returned NULL)");
			SDLNet_TCP_Close(socket);
			return NULL;
		}
		SDL_snprintf(request, length - 1, "GET / HTTP/1.0\r\nHost: %s\r\nConnection: close\r\nUser-Agent: %s\r\n\r\n", uri, userAgent);

		if (SDLNet_TCP_Send(socket, request, strlen(request)) < strlen(request)) {
			SDL_SetError("sending the request failed");
			SDLNet_TCP_Close(socket);
			return NULL;
		}

		buf = SDL_malloc(bufSize + 1);
		// TODO: recv
		SDL_free(buf);
	}
	SDLNet_TCP_Close(socket);
#endif
#endif

	rwops = SDL_RWHttpCreate(httpData);
	if (!rwops) {
		SDL_SetError("Could not fetch the data from %s", uri);
	}
	return rwops;
}

SDL_RWops* SDL_RWFromHttpAsync (const char *uri)
{
	SDL_SetError("Not yet supported");
	return NULL;
}
