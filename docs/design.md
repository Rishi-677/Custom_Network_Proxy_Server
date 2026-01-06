# Design and Architecture of the Mutlithreaded HTTP/HTTPS Proxy Server

This document describes the architectural design, execution model, and operational behavior of a custom **Multithreaded HTTP/HTTPS proxy server**. The intent of this document is to explain **how the system is structured**, **how requests move through the system**, and **why specific design choices were made**.

---
High-Level Architecture Layers

The proxy server is organized into a small number of logical layers, each responsible for a distinct phase of request processing. This layered structure enforces separation of concerns and makes the system easier to reason about and maintain.

1. **Server Initialization & Control Layer**

This layer manages process startup and shutdown. It loads configuration, initializes logging and metrics subsystems, creates the listening socket, and coordinates graceful shutdown on termination signals.

2. **Connection Acceptance Layer**

This layer listens on the configured network address and port and accepts incoming client TCP connections. Accepted connections are converted into tasks and dispatched to the concurrency layer without inspecting request data.

3. **Concurrency Management Layer**

This layer implements a fixed-size thread pool and task queue. It bounds concurrency, reuses worker threads, and enables multiple client connections to be handled in parallel in a predictable manner.

4. **Request Handling Layer**

This layer executes within worker threads and manages the lifecycle of a single client connection. It reads data from the socket, handles partial reads and writes, parses HTTP requests, and determines request type and target host.

5. **Policy & Access Control Layer**

This layer applies domain-based access control after request parsing. It checks the extracted destination host against the configured blocklist and decides whether the request should be allowed or rejected.

6. **Forwarding & Tunneling Layer**

This layer handles communication with upstream servers. It forwards HTTP requests using HTTP/1.0 semantics or establishes transparent TCP tunnels for HTTPS CONNECT requests without inspecting encrypted data.

7. **Observability Layer**

This layer records runtime behavior through thread-safe logging and metrics collection. It captures request outcomes, bytes transferred, and server lifecycle events for visibility and analysis.

Source File Overview

main.cpp – Server startup, configuration loading, shutdown handling

config.cpp – Parses and exposes runtime configuration values

server.cpp – Listening socket setup and connection acceptance

thread_pool.cpp – Thread pool and task queue management

client_handler.cpp – Handles complete lifecycle of one client connection

http_parser.cpp – Parses and validates incoming HTTP requests

forwarder.cpp – HTTP forwarding and HTTPS CONNECT tunneling

blocklist.cpp – Domain-based access control logic

logger.cpp – Thread-safe append-only request logging

metrics.cpp – Runtime metrics tracking and persistence

