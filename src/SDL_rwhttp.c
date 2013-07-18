#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CURL
#include <curl/curl.h>
#endif
#include "SDL_rwhttp.h"

#define STRINGIFY(x) #x

int SDL_RWHttpShutdown (void)
{
	curl_global_cleanup();
	return 0;
}

int SDL_RWHttpInit (void)
{
	SDL_SetHint(SDL_RWHTTP_HINT_USER_AGENT, "sdl_rwhttp/1.0");
#ifdef HAVE_CURL
	const CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
	if (result == CURLE_OK)
		return 0;
	SDL_SetError(curl_easy_strerror(result));
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

	httpData->data = SDL_realloc(httpData->data, httpData->size + realsize + 1);
	if (httpData->data == NULL ) {
		SDL_SetError("not enough memory (realloc returned NULL)");
		return 0;
	}

	SDL_memcpy(&(httpData->data[httpData->size]), streamData, realsize);
	httpData->size += realsize;
	httpData->data[httpData->size] = 0;

	return realsize;
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

	httpData = SDL_malloc(sizeof(*httpData));
	SDL_zerop(httpData);

#ifdef HAVE_CURL
	curlHandle = curl_easy_init();
	curl_easy_setopt(curlHandle, CURLOPT_URL, uri);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, curlHttpWriteSync);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, (void * )httpData);
	curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, SDL_GetHint(SDL_RWHTTP_HINT_USER_AGENT));

	result = curl_easy_perform(curlHandle);
	if (result != CURLE_OK) {
		SDL_SetError(curl_easy_strerror(result));
		return NULL ;
	}
#endif

	rwops = SDL_RWFromConstMem(httpData->data, httpData->size);
	rwops->hidden.unknown.data1 = curlHandle;
	rwops->hidden.unknown.data2 = httpData;
	rwops->close = http_close;
	return rwops;
}

SDL_RWops* SDL_RWFromHttpAsync (const char *uri)
{
	SDL_SetError("Not yet supported");
	return NULL ;
}
