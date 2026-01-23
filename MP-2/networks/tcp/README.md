# Basic TCP Implementation over UDP

## Overview

This project implements a reliable data transfer protocol on top of UDP. It mimics core functionalities of the Transmission Control Protocol (TCP), specifically **Data Sequencing** and **Retransmissions**, to ensure ordered and reliable delivery of messages over an unreliable channel.

The application serves as both a client and a server, capable of splitting large text messages into chunks, transmitting them, handling acknowledgments (ACKs), and reassembling the data at the receiver's end.

## Features

### 1. Data Sequencing

* **Fragmentation:** Large text data is divided into fixed-size chunks (default: 5 bytes) before transmission.
* **Ordering:** Each packet contains a sequence number and the total chunk count in its header.
* **Reassembly:** The receiver buffers incoming out-of-order chunks and reassembles them into the original message once all chunks are received.

### 2. Retransmission Mechanism

* **Reliability:** The sender tracks the timestamp of every transmitted packet. If an ACK is not received within a specified timeout (0.1 seconds), the specific packet is retransmitted.
* **Non-blocking Transmission:** The sender uses a pipelining approach. It transmits all data chunks immediately without waiting for individual ACKs, satisfying the requirement for high-throughput transmission.
* **Selective Retransmission:** Only specific chunks that time out are resent, rather than resending the entire window.

### 3. Simulated Network Unreliability

* **Packet Loss Simulation:** To demonstrate the retransmission capability, the receiver is programmed to intentionally drop (skip sending an ACK for) every 3rd packet. This forces the sender to time out and retransmit the missing data.

## Implementation Details

### Protocol Header

Binary packet headers are constructed using the Python `struct` module. Each packet consists of a header followed by the payload.

**Format:** `!III` (Big-endian, 3 Unsigned Integers)

* **Sequence Number (4 bytes):** The order of the chunk.
* **Total Chunks (4 bytes):** The total number of chunks in the message.
* **Packet Type (4 bytes):** `0` for Data, `1` for ACK.

### Architecture

* **Language:** Python 3 (Standard Library: `socket`, `struct`, `threading`, `time`).
* **Transport Layer:** UDP (`SOCK_DGRAM`) is used as the underlying transport to allow manual implementation of reliability features.
* **Concurrency:** A multi-threaded architecture is used. A background thread listens for incoming packets (Data or ACKs), allowing the main thread to handle user input and packet transmission simultaneously.

## Usage

The script `minitcp.py` acts as a unified node. You must run two instances of the script to simulate communication.

### Prerequisites

* Python 3.x

### 1. Start the Server

The server initializes a listener on a specific port. In this implementation, the server is configured to listen on the specified port and send responses to `port + 1`.

```bash
python minitcp.py server <port>

```

**Example:**

```bash
python minitcp.py server 5000

```

### 2. Start the Client

The client initializes on `port + 1` and targets the server's port. It automatically sends a test string upon startup.

```bash
python minitcp.py client <server_port>

```

**Example:**

```bash
python minitcp.py client 5000

```

## Configuration

The following constants in `minitcp.py` can be modified to tune performance or testing behavior:

* `CHUNK_SIZE`: Size of the data payload per packet (default: 5 bytes).
* `TIMEOUT`: Duration in seconds before a packet is considered lost (default: 0.1s).
* `DROP_RATE_N`: Frequency of simulated ACK drops (default: 3).

## Disclaimer

The packet loss simulation (dropping every 3rd ACK) is strictly for demonstration purposes as per the project requirements. To enable a fully reliable production-like environment, comment out the loss simulation logic in the `handle_data` method.