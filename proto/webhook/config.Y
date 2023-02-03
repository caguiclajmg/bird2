/*
 *	BIRD -- Webhook Protocol
 *
 *	(c) 2023 John Mark Gabriel Caguicla <jmg.caguicla@guarandoo.me>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#include "proto/webhook/webhook.h"

CF_DEFINES

#define WEBHOOK_CFG ((struct webhook_config *) this_proto)
#define WEBHOOK_CC ((struct channel_config *) this_channel)

static void
webhook_check_unused_url(void)
{
  if (WEBHOOK_CFG->url != NULL)
    cf_error("Only one url per protocol allowed");
}

CF_DECLS

CF_KEYWORDS(STATIC, ROUTE, VIA, DROP, REJECT, PROHIBIT, PREFERENCE, CHECK, LINK)
CF_KEYWORDS(ONLINK, WEIGHT, RECURSIVE, IGP, TABLE, BLACKHOLE, UNREACHABLE, BFD, MPLS)
CF_KEYWORDS(WEBHOOK, PEER, TABLE, URL)

CF_GRAMMAR

proto: webhook_proto '}' ;

webhook_proto_start: proto_start WEBHOOK
{
  this_proto = proto_config_new(&proto_webhook, $1);
};

webhook_proto:
   webhook_proto_start proto_name '{' 
 | webhook_proto proto_item ';'
 | webhook_proto proto_channel ';'
 | webhook_proto URL text ';' { WEBHOOK_CFG->url = $3; }
 ;

CF_CODE

CF_END
