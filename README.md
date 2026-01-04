# Custom Network Proxy Server

This project is a forward HTTP proxy server implemented in C++.

It builds incrementally on a previously developed HTTP server,
reusing core concepts such as socket handling, concurrency,
and logging, while extending functionality to support request
forwarding and HTTPS tunneling.

## Scope
- HTTP forward proxy
- Minimal HTTP parsing
- Optional CONNECT tunneling
- No TLS inspection
- No full RFC compliance

## Build
make

## Run
./proxy
