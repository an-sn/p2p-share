# p2p-share

This project implements a hybrid P2P network where a central tracker stores file metadata, while peers independently manage the actual file transfers.

Peers can register files they are sharing, download specific file chunks from other peers, and use the central tracker for peer discovery and file metadata coordination.

![Hybrid P2P Architecture](assets/hybrid_p2p_architecture.jpg)

## Architecture

The tracker is implemented in C++ using the Boost Beast library for handling HTTP requests and responses. Peer endpoints and file/chunk information are stored in an in-memory Redis database to ensure fast response times.

## Building the server

The server is built and run inside a Docker container for ease of use and portability.

```
docker build -t p2p .
```

## Starting the application

Start Redis DB (could be hosted on Docker or on bare-metal).
Currently using Docker for illustration.

```
docker run --name my-redis-container -p <redis_port>:6379 -d redis
```

We can start the server now.
```
docker run -p <host_port>:8080 p2p <redis_ip> <redis_port>
```

## Client application

The client shares file metadata with the server and directly communicates with other peers to send and receive files.

Create a python virtual environment (recommended):
```
python3 -m venv venv
source venv/bin/activate
```

Installing dependencies:
```
pip install -r client/requirements.txt
```

Starting the client:
```
python3 client.py --server_ip <server_ip> --server_port <server_port> --client_ip <client_ip> --client_port <client_port>
```

A simple command line interface is provided to interact with the client application.
Multiple clients can be started on different client ports to simulate the hybrid P2P network.