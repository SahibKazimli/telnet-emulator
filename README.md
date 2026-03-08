# Telnet Emulator

A simple TCP-based terminal emulator built in C. The client sends shell commands to the server, which executes them and sends the output back.

## Protocol

Uses a **length-prefixed** protocol on top of TCP:

1. Sender sends message length (4 bytes, network byte order)
2. Sender sends the message data
3. Server sends **length = 0** to signal end of output (EOM)

## Build & Run

```bash
make all        # compile both server and client
make clean      # remove binaries
```

```bash
# Terminal 1
./server

# Terminal 2
./client
```

## Example

```
> ls
Makefile
README.md
client.c
server.c
> pwd
/Users/you/telnet-emulator
> whoami
you
> exit
[Client] Exiting...
```

## Files

| File | Description |
|---|---|
| `server.c` | Listens on port 9999, receives commands, executes via `popen()`, sends output back |
| `client.c` | Connects to server, interactive prompt, sends commands, displays output |
| `Makefile` | Builds both binaries |
