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

## Filters

### maskip

```
# net = 1111:2222:3333:4444:cafe::
net.ip.maskip(0:0:0:0:ffff::) = 0:0:0:0:cafe:: # true

function has_beef() {
  if net.ip.maskip(ffff::) = beef:: then return true;
  if net.ip.maskip(0:ffff::) = 0:beef:: then return true;
  if net.ip.maskip(0:0:ffff::) = 0:0:beef:: then return true;
  if net.ip.maskip(0:0:0:ffff::) = 0:0:0:beef:: then return true;
  if net.ip.maskip(0:0:0:0:ffff::) = 0:0:0:0:beef:: then return true;
  if net.ip.maskip(0:0:0:0:0:ffff::) = 0:0:0:0:0:beef:: then return true;
  if net.ip.maskip(0:0:0:0:0:0:ffff::) = 0:0:0:0:0:0:beef:: then return true;
  if net.ip.maskip(0:0:0:0:0:0:0:ffff) = 0:0:0:0:0:0:0:beef then return true;
  return false;
}
```