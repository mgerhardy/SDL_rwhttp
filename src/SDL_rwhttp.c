#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CURL
#include <curl/curl.h>
#endif
#include "SDL_rwhttp.h"

#define STRINGIFY(x) #x

static const char *userAgent;
static int connectTimeout;
static int timeout;

int SDL_RWHttpShutdown (void)
{
	curl_global_cleanup();
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

#ifdef HAVE_CURL
	{
		const CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
		if (result == CURLE_OK)
			return 0;
		SDL_SetError(curl_easy_strerror(result));
	}
#else
	SDL_SetError("Required library is missing");
#endif
	return -1;
}

#define http_get_curl(context) (CURL*) (context)->hidden.unknown.data1

typedef struct {
	char *uri;
	char *data;
	size_t size;
	size_t expectedSize;
} http_data_t;

static int SDLCALL http_close (SDL_RWops * context)
{
	int status = 0;
	if (!context)
		return status;

	if (context->hidden.unknown.data1) {
#ifdef HAVE_CURL
		CURL *curl = http_get_curl(context);
		curl_easy_cleanup(curl);
#endif
	}
	if (context->hidden.unknown.data2) {
		http_data_t *data = (http_data_t*) context->hidden.unknown.data2;
		SDL_free(data->data);
		SDL_free(data);
	}
	SDL_FreeRW(context);
	return status;
}

#ifdef HAVE_CURL
static size_t curlHttpWriteSync (void *streamData, size_t size, size_t nmemb, void *userData)
{
	const size_t realsize = size * nmemb;
	http_data_t *httpData = (http_data_t *) userData;

	if (httpData->expectedSize == 0) {
		httpData->data = SDL_realloc(httpData->data, httpData->size + realsize + 1);
		if (httpData->data == NULL) {
			SDL_SetError("not enough memory (realloc returned NULL)");
			return 0;
		}
	} else if (httpData->size + realsize >= httpData->expectedSize) {
		SDL_SetError("illegal Content-Length - buffer overflow");
		return 0;
	}

	SDL_memcpy(&(httpData->data[httpData->size]), streamData, realsize);
	httpData->size += realsize;
	httpData->data[httpData->size] = 0;

	return realsize;
}

size_t curlHttpHeader (void *headerData, size_t size, size_t nmemb, void *userData)
{
	const char *header = (const char *)headerData;
	const size_t bytes = size * nmemb;
	const char *contentLength = "Content-Length: ";
	const size_t strLength = strlen(contentLength);

	if (bytes <= strLength)
		return bytes;

	if (!SDL_strncasecmp(header, contentLength, strLength)) {
		http_data_t *httpData = (http_data_t *) userData;
		httpData->expectedSize = SDL_strtoul(header + strLength, NULL, 10);
		httpData->data = SDL_malloc(httpData->expectedSize);
		if (httpData->data == NULL) {
			SDL_SetError("not enough memory (malloc returned NULL)");
			return 0;
		}
	}

	return bytes;
}
#endif

SDL_RWops* SDL_RWFromHttpSync (const char *uri)
{
	SDL_RWops *rwops;
	http_data_t *httpData;
#ifdef HAVE_CURL
	CURL* curlHandle;
	CURLcode result;
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
	curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, userAgent);
	curl_easy_setopt(curlHandle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curlHandle, CURLOPT_MAXREDIRS, 5);
	curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curlHandle, CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(curlHandle, CURLOPT_CONNECTTIMEOUT, connectTimeout);
	curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, timeout);

	result = curl_easy_perform(curlHandle);
	if (result != CURLE_OK) {
		SDL_SetError(curl_easy_strerror(result));
		return NULL ;
	}
#endif

	rwops = SDL_RWFromConstMem(httpData->data, httpData->size);
	if (rwops) {
		rwops->hidden.unknown.data1 = curlHandle;
		rwops->hidden.unknown.data2 = httpData;
		rwops->close = http_close;
	} else {
		SDL_SetError("Could not fetch the data from %s", uri);
	}
	return rwops;
}

SDL_RWops* SDL_RWFromHttpAsync (const char *uri)
{
	SDL_SetError("Not yet supported");
	return NULL ;
}
