# Custom Network Proxy Server (C++)

## Overview

This project is a **Multithreaded HTTP/HTTPS proxy server** written in **C++**.

All architectural design decisions, execution flow, concurrency model, and flowcharts are documented in docs/design.md.

The proxy sits between clients (for example, `curl` or a browser) and destination servers. It forwards plain HTTP traffic, tunnels HTTPS traffic using the `CONNECT` method, applies domain‑based access control, and records detailed runtime information through logs and metrics.

---

## Implemented Features

* HTTP request forwarding for standard methods such as `GET` and `POST`
* HTTPS tunneling using the `CONNECT` method without TLS inspection
* Thread‑pool‑based concurrency with blocking socket I/O
* Robust handling of partial reads and partial writes on network sockets
* Configurable listening address and port
* Configurable thread pool size
* Configurable socket timeouts to prevent stalled or hung connections
* Domain‑based request blocking using a blocklist file
* Exact‑match and subdomain blocklist support
* Thread‑safe, append‑only request logging
* Explicit server lifecycle demarcation (START / STOP) in logs
* Graceful shutdown on termination signals
* Centralized configuration via `config/proxy.conf`
* Runtime metrics tracking (requests, bytes, hosts, rate)
* Metrics written to a dedicated `metrics.txt` file
* Modular codebase with clear separation of concerns (parsing, forwarding, logging, metrics, configuration)

---

## HTTP and Proxy Behavior

For **HTTP traffic**, the proxy parses incoming requests, validates the request line and headers, resolves the destination host, forwards the request to the upstream server, and relays the response back to the client. Requests are rewritten as necessary to ensure compatibility with upstream servers, and each connection is handled independently.

The proxy forwards requests using HTTP/1.0 semantics. This means each request is treated as a single, non-persistent transaction, and the connection is closed once the response is fully transmitted. As a result, there is no connection reuse (keep-alive), which simplifies connection lifecycle management, avoids long-lived sockets, and makes behavior more predictable.

For **HTTPS traffic**, the proxy supports the `CONNECT` method. Upon receiving a valid `CONNECT` request, the proxy establishes a TCP connection to the target server and transparently tunnels bytes between the client and server. Encrypted payloads are not inspected or modified, preserving end‑to‑end security.

Each client connection is processed by a worker thread from a fixed‑size thread pool. This allows multiple requests to be handled concurrently while keeping the control flow simple and deterministic.

---

## Configuration (`config/proxy.conf`)

All runtime behavior of the proxy is controlled through the `config/proxy.conf` file. This file is read at startup and allows the server to be reconfigured without recompilation.

Configurable parameters include:

* Listening address and port
* Thread pool size
* Socket buffer size
* Default HTTP port
* Socket timeout values
* Log file path and size limit
* Blocklist file path
* Metrics output file path

This configuration‑driven approach avoids hard‑coded values and makes the server easier to adapt to different environments and workloads.

---

## Build and Run Instructions

### System Requirements

To build and run this proxy server, you need:

* **Operating system**: Linux (tested on Ubuntu-based systems)
* **Compiler**: `g++` with **C++17** support
  Recommended version: g++ 7.0 or newer
* **Build tools**: `make`
* **Networking tools (for testing)**: `curl`

You can verify your compiler version with:

```bash
g++ --version
```

---

### Clone the Repository

Clone the repository to your local machine:

```bash
git clone https://github.com/Rishi-677/Custom_Network_Proxy_Server.git
cd proxy-server
```

The repository will be cloned locally, and the project directory is named `proxy-server`.

---

### Build

Before building, it is recommended to clean any existing build artifacts:

```bash
make clean
```

Then build the project:

```bash
make
```

On success, an executable named `proxy` will be generated in the project root.

---

### Run

Start the proxy server:

```bash
./proxy
```

Optionally override the listening port:

```bash
./proxy 9090
```

All other runtime settings can be adjusted in `config/proxy.conf`.
