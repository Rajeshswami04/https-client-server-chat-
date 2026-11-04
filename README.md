# TLS Client-Server Chat

A small command-line chat application written in C. The server and client communicate over a TCP connection protected by TLS using OpenSSL. Messages are exchanged one at a time: the client sends a message, then the server replies.

## Requirements

- GCC or another C compiler
- OpenSSL development headers and libraries
- The OpenSSL command-line tool (to create a local certificate)

The source uses POSIX socket APIs, so these instructions target Linux and other Unix-like systems.

## Create a certificate

Both programs expect `cert.pem` and `key.pem` in the directory from which they are run. For local testing, create a self-signed certificate:

```sh
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem \
  -days 365 -nodes -subj "/CN=localhost"
```

The client uses `cert.pem` as a trusted certificate. Keep `key.pem private`; do not share or commit it.

## Build

Run these commands from the project directory:

```sh
gcc server.c -o server -lssl -lcrypto
gcc client.c -o client -lssl -lcrypto
```

If OpenSSL is installed in a non-standard location, provide the appropriate include and library paths to your compiler.

## Run

Open two terminals in the project directory. Start the server first:

```sh
./server
```

Then start the client:

```sh
./client
```

Type a message at the client prompt and press Enter. The server displays it and prompts the operator for a reply; the client displays that reply. Repeat to continue the conversation.

The server listens on TCP port `8080` on all network interfaces. The client connects to `127.0.0.1:8080`, so the default setup is for both programs to run on the same machine. To connect from another machine, update the address in `client.c` and ensure the server's port is reachable.

## Message format

Each message is sent over TLS as a four-byte unsigned length in network byte order, followed by that many message bytes. The receive buffers allow at most 2047 message bytes.

## Current limitations

- The server accepts one client connection and handles messages synchronously; it does not support multiple simultaneous clients.
- The client checks for `quit` locally before sending a message. This exits the client but does not notify the server, so stop the server separately (for example, with Ctrl+C).
- The client trusts the provided certificate but does not verify that its identity matches the server address. This example is for learning and local testing, not production use.
