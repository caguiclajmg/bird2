/*
 *	BIRD -- Webhook Protocol
 *
 *	(c) 2023 John Mark Gabriel Caguicla <jmg.caguicla@guarandoo.me>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Webhook
 *
 * Fires an HTTP request whenever the attached table is updated
 */

#undef LOCAL_DEBUG

#include <curl/curl.h>
#include <json-c/json_tokener.h>

#include "nest/bird.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "nest/route.h"
#include "nest/cli.h"
#include "conf/conf.h"
#include "filter/filter.h"
#include "lib/string.h"

#include "webhook.h"

static void
webhook_rt_notify(struct proto *P, struct channel *src_ch, net *n, rte *new, rte *old)
{
  struct webhook_proto *p = (void *) P;
  struct webhook_config *cf = (void *) p->p.cf;

  CURL *curl = curl_easy_init();
  if (curl) {
    struct json_object *json = json_object_new_object();
    char addrBuf[100];
    bsnprintf(addrBuf, sizeof(addrBuf), "%N", n->n.addr);
    json_object_object_add(json, "addr", json_object_new_string(addrBuf));
    struct json_object *json_routes = json_object_new_object();
    struct json_object *json_routes_attrs = json_object_new_object();
    json_object_object_add(json_routes, "attrs", json_routes_attrs);
    json_object_object_add(json_routes, "flags", json_object_new_int(n->routes->flags));
    json_object_object_add(json_routes, "id", json_object_new_int(n->routes->id));
    json_object_object_add(json_routes, "lastmod", json_object_new_int64(n->routes->lastmod));
    struct json_object *json_routes_net = json_object_new_object();
    json_object_object_add(json_routes, "net", json_routes_net);
    struct json_object *json_routes_next = json_object_new_object();
    json_object_object_add(json_routes, "next", json_routes_next);
    json_object_object_add(json_routes, "pflags", json_object_new_int(n->routes->pflags));
    struct json_object *json_routes_src = json_object_new_object();
    json_object_object_add(json_routes_src, "global_id", json_object_new_int(n->routes->src->global_id));
    json_object_object_add(json_routes_src, "private_id", json_object_new_int(n->routes->src->private_id));
    json_object_object_add(json_routes_src, "uc", json_object_new_int(n->routes->src->uc));
    json_object_object_add(json_routes, "src", json_routes_src);
    json_object_object_add(json, "routes", json_routes);
    const char* body = json_object_to_json_string(json);

    curl_easy_setopt(curl, CURLOPT_URL, cf->url);
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) log(L_ERR "webhook request failed: %s\n", curl_easy_strerror(res));
    curl_easy_cleanup(curl);
  }

  return;
}

static void
webhook_reload_routes(struct channel *C)
{
  struct webhook_proto *p = (void *) C->proto;

  log(L_INFO "bruh");
  /* Route reload on one channel is just refeed on the other */
  //channel_request_feeding((C == p->pri) ? p->sec : p->pri);
}

static void
webhook_postconfig(struct proto_config *CF)
{
  struct webhook_config *cf = (void *) CF;
  struct channel_config *ip4, *ip6, *ip6_sadr;

  ip4 = proto_cf_find_channel(CF, NET_IP4);
  ip6 = proto_cf_find_channel(CF, NET_IP6);
  ip6_sadr = proto_cf_find_channel(CF, NET_IP6_SADR);

  cf->ip4_channel = ip4;
  cf->ip6_channel = ip6 ?: ip6_sadr;
}

static int
webhook_configure_channels(struct webhook_proto *p, struct webhook_config *cf)
{
  struct channel_config *ip4, *ip6;

  ip4 = cf->ip4_channel;
  ip4->ra_mode = RA_ANY;

  ip6 = cf->ip6_channel;
  ip6->ra_mode = RA_ANY;

  return
    proto_configure_channel(&p->p, &p->ip4_channel, ip4) &&
    proto_configure_channel(&p->p, &p->ip6_channel, ip6);
}

static struct proto *
webhook_init(struct proto_config *CF)
{
  struct proto *P = proto_new(CF);
  struct webhook_proto *p = (void *) P;
  struct webhook_config *cf = (void *) CF;

  P->rt_notify = webhook_rt_notify;
  P->reload_routes = webhook_reload_routes;

  webhook_configure_channels(p, cf);

  return P;
}

static int
webhook_reconfigure(struct proto *P, struct proto_config *CF)
{
  struct webhook_proto *p = (void *) P;
  struct webhook_config *cf = (void *) CF;

  return webhook_configure_channels(p, cf);
}

static void
webhook_copy_config(struct proto_config *dest UNUSED, struct proto_config *src UNUSED)
{
  /* Just a shallow copy, not many items here */
}

static void
webhook_get_status(struct proto *P, byte *buf)
{
  struct webhook_proto *p = (void *) P;
  struct webhook_config *cf = (void *) p->p.cf;

  bsprintf(buf, "Publishing to %s", cf->url);
}

static void
webhook_show_proto_info(struct proto *P)
{
  struct webhook_proto *p = (void *) P;
  struct webhook_config *cf = (void *) p->p.cf;

  cli_msg(-1006, "  Channel %s", "main");
  cli_msg(-1006, "    Url:            %s", cf->url);
}

void
webhook_update_debug(struct proto *P)
{
  struct webhook_proto *p = (void *) P;

  if(p->ip4_channel) p->ip4_channel->debug = p->p.debug;
  if(p->ip6_channel) p->ip6_channel->debug = p->p.debug;
}

struct protocol proto_webhook = {
  .name =		"Webhook",
  .template =		"webhook%d",
  .class =		PROTOCOL_WEBHOOK,
  .channel_mask = NB_IP | NB_IP6,
  .proto_size =		sizeof(struct webhook_proto),
  .config_size =	sizeof(struct webhook_config),
  .postconfig =		webhook_postconfig,
  .init =		webhook_init,
  .reconfigure =	webhook_reconfigure,
  .copy_config = 	webhook_copy_config,
  .get_status = 	webhook_get_status,
  .show_proto_info = 	webhook_show_proto_info
};

void
webhook_build(void)
{
  proto_build(&proto_webhook);
}
