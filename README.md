# BIRD2

## Protocols

### Webhook

```
protocol webhook {
  url "https://example.com/my-bird2-webhook";
  ipv4 { export all; };
  ipv6 { export all; };
}
```

```json
{
  "addr": "172.17.0.0/16",
  "routes": {
    "attrs": {},
    "flags": 1,
    "id": 2,
    "lastmod": 34347537210,
    "net": {},
    "next": {},
    "pflags": 0,
    "src": {
      "global_id": 7,
      "private_id": 8,
      "uc": 1
    }
  }
}
```