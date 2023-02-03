/*
 *	BIRD -- Webhook Protocol
 *
 *	(c) 2023 John Mark Gabriel Caguicla <jmg.caguicla@guarandoo.me>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_WEBHOOK_H_
#define _BIRD_WEBHOOK_H_

struct webhook_config {
  struct proto_config c;

  const char *url;
  struct channel_config *ip4_channel;
  struct channel_config *ip6_channel;
};

struct webhook_proto {
  struct proto p;

  struct channel *ip4_channel;
  struct channel *ip6_channel;
};

#endif
