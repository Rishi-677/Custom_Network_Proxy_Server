# Design and Architecture of the Multithreaded HTTP/HTTPS Proxy Server

This document describes the architectural design, execution model, and operational behavior of a custom **Multithreaded HTTP/HTTPS Proxy Server**. The intent of this document is to explain **how the system is structured**, **how requests move through the system**, and **why specific design choices were made**.

---

## High-Level Architecture Layers

The proxy server is organized into a small number of logical layers, each responsible for a distinct phase of request processing. This layered structure enforces separation of concerns and makes the system easier to reason about and maintain.

### 1. Server Initialization & Control Layer

This layer manages process startup and shutdown. It loads configuration, initializes logging and metrics subsystems, creates the listening socket, and coordinates graceful shutdown on termination signals.

### 2. Connection Acceptance Layer

This layer listens on the configured network address and port and accepts incoming client TCP connections. Accepted connections are converted into tasks and dispatched to the concurrency layer without inspecting request data.

### 3. Concurrency Management Layer

This layer implements a fixed-size thread pool and task queue. It bounds concurrency, reuses worker threads, and enables multiple client connections to be handled in parallel in a predictable manner.

### 4. Request Handling Layer

This layer executes within worker threads and manages the lifecycle of a single client connection. It reads data from the socket, handles partial reads and writes, parses HTTP requests, and determines request type and target host.

### 5. Policy & Access Control Layer

This layer applies domain-based access control after request parsing. It checks the extracted destination host against the configured blocklist and decides whether the request should be allowed or rejected.

### 6. Forwarding & Tunneling Layer

This layer handles communication with upstream servers. It forwards HTTP requests using HTTP/1.0 semantics or establishes transparent TCP tunnels for HTTPS CONNECT requests without inspecting encrypted data.

### 7. Observability Layer

This layer records runtime behavior through thread-safe logging and metrics collection. It captures request outcomes, bytes transferred, and server lifecycle events for visibility and analysis.

---

## Source File Overview

- **`main.cpp`** – Server startup, configuration loading, shutdown handling  
- **`config.cpp`** – Parses and exposes runtime configuration values  
- **`server.cpp`** – Listening socket setup and connection acceptance  
- **`thread_pool.cpp`** – Thread pool and task queue management  
- **`client_handler.cpp`** – Handles complete lifecycle of one client connection  
- **`http_parser.cpp`** – Parses and validates incoming HTTP requests  
- **`forwarder.cpp`** – HTTP forwarding and HTTPS CONNECT tunneling  
- **`blocklist.cpp`** – Domain-based access control logic  
- **`logger.cpp`** – Thread-safe append-only request logging  
- **`metrics.cpp`** – Runtime metrics tracking and persistence  

---

## Concurrency Model and Rationale

### Chosen Concurrency Model

The proxy server uses a **thread-pool-based concurrency model** combined with **blocking socket I/O and explicit socket timeouts**. A fixed number of worker threads are created at startup and reused to process client connections throughout the lifetime of the server.

The main server thread is responsible only for accepting incoming TCP connections. Each accepted connection is encapsulated as a task and placed into a shared task queue. Worker threads continuously retrieve tasks from this queue and process client connections independently.

Each worker thread owns **exactly one client connection at a time**. All request parsing, policy enforcement, forwarding, tunneling, logging, and cleanup are performed synchronously within the worker using blocking I/O operations.

---

### How the Model Works

1. At startup, the server initializes a fixed-size thread pool.  
2. The main thread listens for incoming client TCP connections.  
3. When a client connects, the connection is accepted and enqueued as a task.  
4. An available worker thread dequeues the task and assumes exclusive ownership of the connection.  
5. The worker performs blocking read and write operations on the socket, handling partial reads and writes as needed.  
6. Socket timeouts are applied to prevent workers from blocking indefinitely on idle or stalled connections.  
7. The worker processes the request fully, including parsing, access control checks, and outbound communication.  
8. Once the request completes or an error occurs, the worker closes the connection, updates logs and metrics, and returns to the task queue.

Multiple worker threads execute this flow concurrently, allowing the server to handle multiple client connections in parallel while maintaining a simple and deterministic execution model.

---

### Rationale: Advantages and Trade-offs

**Advantages**

- Fixed thread pool provides bounded and predictable concurrency  
- Thread reuse avoids overhead of frequent thread creation and destruction  
- Blocking I/O simplifies request handling and error propagation  
- Explicit socket timeouts prevent worker starvation 
- Clean and graceful shutdown behavior
- Well-suited for learning, demonstration, and controlled workloads  

**Trade-offs**

- Each active connection consumes a worker thread  
- Maximum concurrency is limited by thread pool size  
- Blocking I/O can reduce scalability under very high connection counts  
- Less efficient than event-driven models for large numbers of idle clients  

---

This concurrency model was chosen to prioritize **correctness, clarity, and robustness** over maximum throughput, while still providing true parallel request handling.

---
## Request and Data Flow

This section describes how data flows through the proxy server during request handling, from the moment a client connection is accepted to the completion of outbound forwarding or tunneling. The flow is divided into logical phases to reflect responsibility boundaries within the system.

---

### Connection Acceptance and Assignment

-When a client initiates a TCP connection to the proxy, the main server thread accepts the connection on the listening socket. The accepted socket is wrapped as a task and enqueued into the shared task queue managed by the thread pool.

-An available worker thread dequeues the task and assumes exclusive ownership of the client connection for its entire lifetime. From this point onward, all processing occurs within the context of that worker thread.

---

### Request Reading and Parsing

-The worker thread reads data from the client socket using blocking I/O. Since TCP is a stream-oriented protocol, the worker explicitly handles partial reads and assembles incoming data until a complete HTTP request header is received.

-Once sufficient data is available, the HTTP request is parsed to extract essential fields such as the request method, target host, port, and request path. Requests that are malformed or incomplete are rejected early, and the connection is closed without initiating any outbound communication.

---

### Policy Evaluation

-After successful parsing, the extracted destination host is evaluated against the configured blocklist. This policy check determines whether the request is permitted to proceed.

-If the request is blocked, the worker sends an appropriate error response (for example, `403 Forbidden`) to the client and terminates the connection. If the request is allowed, processing continues to outbound communication.

---

### Outbound Communication

The outbound handling phase depends on the request type.

-For **HTTP requests**, the worker establishes a TCP connection to the upstream server and forwards the rewritten request using HTTP/1.0 semantics. The response from the upstream server is read incrementally and relayed back to the client, with partial writes handled explicitly. Once the response is fully transmitted, both connections are closed.

-For **HTTPS CONNECT requests**, the worker establishes a TCP connection to the specified target host and port and responds to the client with a `200 Connection Established` message. The worker then enters a bidirectional tunneling phase, transparently forwarding raw bytes between client and server without inspecting or modifying encrypted data. The tunnel remains active until either side closes the connection or a timeout occurs.

---

### Completion, Logging, and Metrics

-After request processing completes—whether due to normal completion, error, or timeout—the worker performs cleanup operations. Client and upstream sockets are closed, and runtime metrics such as request counts and bytes transferred are updated.

-A structured log entry is written to record the outcome of the request. The worker then returns to the task queue, ready to process the next client connection.

---

This staged data flow ensures that request handling is predictable and failures are contained within individual connections without affecting the overall stability of the proxy server.


