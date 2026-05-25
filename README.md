# ft_irc early setup

This project builds `ircserv`, a C++98 IRC server run as:

```sh
./ircserv <port> <password>
```

Subject constraints reflected in the scaffold:

- C++98, `-Wall -Wextra -Werror`, no external or Boost libraries.
- Mandatory Makefile rules: `$(NAME)`, `all`, `clean`, `fclean`, `re`.
- TCP server only; no IRC client and no server-to-server implementation.
- Multiple clients handled without `fork()`.
- Listening, reading, and writing are routed through a single `poll()` loop.
- File descriptors are set to non-blocking mode.
- Per-client input buffers rebuild IRC lines from partial TCP packets.
- Per-client output buffers preserve unsent data after partial `send()`.

Current implementation scope:

- Starts a listening socket on the requested port.
- Accepts multiple clients.
- Parses complete IRC command lines ending in `\n` or `\r\n`.
- Handles early `CAP`, `PASS`, `NICK`, `USER`, `PING`, and `QUIT`.
- Marks a client registered after valid `PASS`, `NICK`, and `USER`.
- Sends basic IRC numeric replies for welcome and common registration errors.

Next build steps:

1. Add channel state and `JOIN`/`PART`.
2. Add `PRIVMSG` routing to users and channels.
3. Add channel operators and `KICK`, `INVITE`, `TOPIC`, `MODE`.
4. Extend numeric replies for reference-client compatibility.
5. Add focused `nc` tests for partial commands and malformed input.
