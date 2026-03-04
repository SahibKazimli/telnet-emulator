# Telnet Emulator

A simple TCP-based terminal emulator built in C 

## The Idea

The **client** connects to the **server** over TCP. You type shell commands on the client, the server executes them using `popen()`, and sends the output back.

## Protocol

Uses a **length-prefixed** protocol on top of TCP:

1. Sender sends the message length
2. Sender sends the **message data**
3. Server sends **length = 0** (End of Message)

## Build & Run
I'll add this when I'm actually done


