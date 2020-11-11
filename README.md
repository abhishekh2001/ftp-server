# FTP Server

## Instructions

The server and the client directories contain the code for the server and the client respectively.

The server can be compiled and executed from within the server directory as

```bash
gcc server.c -o server.out
./server.out
```

and for the client from the client directory as

```bash
gcc client.c -o client.out
./client.out
```

Note that a server must be runnning for a client to access it.

## Client

Retrieving files: Client can access files that are found in the server directory by issuing `get <files.files>`, supporting multiple file requests per command.

Exitting the client: `exit` will stop the client as well as the server running

The server terminates its execution once it has serviced a single client until the connection is broken.
