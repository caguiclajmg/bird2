#include <curl/curl.h>

#include "nest/bird.h"
#include "nest/http.h"
#include "conf/conf.h"
#include "lib/resource.h"
#include "lib/string.h"

/**
 * http_init - initialize http
 *
 * This function initializes HTTP related resources (libcurl)
 */
void
http_init()
{
#ifdef CONFIG_WEBHOOK
  curl_global_init(CURL_GLOBAL_ALL);
#endif
}
