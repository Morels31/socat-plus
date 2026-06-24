
# Socat Plus

Socat Plus can be used to intercept TLS traffic to and from a local service.
Used in combo with tools like `tcpdump` or `tshark` can let you inspect HTTPS it GRPC traffic.


## Architecture

```
External request --TLS--> [tls_server] --plaintext-TCP--> [tcp_server] --TLS--> Local service
                                                         ^
                                                         |
                                                   Inspect here
```


## Usage
```
socat-plus -t <TLS port> -s <Service port> -x <type> [-p <TCP port>] [-c <Cert path>] [-k <Key path>] [-r <Root CA path>]
```

### Options

| Flag | Default      | Description                            |
|------|--------------|----------------------------------------|
| `-t` | -            | TLS listener port (required)           |
| `-s` | -            | Backend service port (required)        |
| `-x` | -            | Proxy type: `tls` or `grpc` (required) |
| `-p` | `8080`       | Internal plain-text TCP port           |
| `-c` | `./cert.pem` | TLS certificate file path              |
| `-k` | `./key.pem`  | TLS private key file path              |
| `-r` | none         | Root CA certificate path               |

**Important: Make sure to assign both `-t` and `-p` an unique non already binded port.**


## Tip

To intercept between the attackers and the service you teorically need to change the port of the service, and restart it.
By doing a little iptables magic, you do not need to need neither of those.

The idea is to DNAT (change packets destination mid flight) only the traffic coming from the outside to the `-t` port,
and then set the `-s` port to the original port of the service.

### TLDR

Start socat-plus, with settings:
- `-s` to the service port
- `-t` and `-p` to unique ports.
- `-c` to the service certificate
- `-k` to the service private key

When ready execute this command:
```
iptables -t nat -I PREROUTING ! -i lo -p tcp -m tcp --dport <SERVICE_PORT> -j DNAT --to-destination 127.0.0.1:<TLS_LISTENER_PORT>
```

To disable the intercepting:
```
iptables -t nat -D PREROUTING ! -i lo -p tcp -m tcp --dport <SERVICE_PORT> -j DNAT --to-destination 127.0.0.1:<TLS_LISTENER_PORT>
```


## Inspecting traffic

To inspect traffic you need to listen to 127.0.0.1:`-p`
