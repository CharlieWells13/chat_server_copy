# CSCI 2510: Lab `chat` - Network Chat Client and Server

Sockets programming is the fundamental interface to networking,
and the server/client connection protocol is the basic architecture
for programming networked applications. 

In this lab, you will:

1.  Write a chat server that manages multiple concurrent connections
2.  Write a chat client
3.  Perform asynchronous message passing between arbitrary numbers of
    clients
4.  Perform concurrent, nonblocking I/O

------------------------------------------------------------------------

### Parameters

This is a group assignment.

------------------------------------------------------------------------

### Usage:

`server <port number>`

`client <IP address> <port number>`

### Description

The goal of this assignment is to write a networked chat server and
client. Multiple users will connect concurrently to a single server
process via individual chat clients. The client program will send
messages from users to the server, and will print messages from the
server to the user\'s terminal screen. The server will keep track of
connected clients, and when it recieves a message from any client it
will pass that message along to all other connected clients. Each user
will also have a name that is used to identify their statements.

Each user, when they first connect, should get a default name consisting
of the string \"User\" followed by a unique set of digits. When a user
connects to the server any other connected users should recieve a
message stating \"`<user name> has connected`\".

Additionally, the client will support two special commands `name` and
`quit`.

`name` command: When the client enters the phrase \"`name`\" followed by
a space and at least one character, everything after the space will
become the user\'s name. The system will then send this name in front of
any subsequent chat messages. If the user specifies another name their
current name is replaced. Lastly, when any user changes their name then
all other users should recieve the message
\"`<old name> has changed their name to <new name>`\"

`quit` command: When the client enters the phrase \"`quit`\" the client
program will terminate. All other connected users should recieve the
message \"\<user name\> has quit\".

Messages should always be printed to all terminals in less than a
second, regardless of the current state of the system. (This means that
your client should be able to display text even while waiting for input,
and your server should be able to respond to an incoming request by
*any* client at *any* time.)

If a client suddenly finds its socket to be closed then it should quit
and print a message saying the server shut down unexpectedly. If the
server finds a socket unable to be used it should print the same message
as though the user quit normally.

### Hints

-   Test all your code locally. Specifying an IP address of `127.0.0.1`
    (the loopback address) will cause all traffic to stay local. Using
    this IP you can run your client and your server on the same machine.
    I won\'t require you to test your code across the internet, but it
    should work in theory. (Things like firewalls can make this harder
    than it it first seems. I\'m happy to give advice on how to get this
    running if you\'d like.)

-   Your programs should accept any 16-bit port number, but in practice you
    probably want to use a large, random number. Better yet, look up a
    list of commonly used ports online and pick one that\'s not used.
    If you're using our shared system, don't pick a port number like
    12345, because there's a good chance someone else in the class will pick
    the same number!
    Remember that ports under 1024 typically require superuser privileges, so
    pick a number in the range 1024-65535.

-   Your server and each of your clients must specify the *same* port
    number to communicate. How do you tell clients apart? The sockets
    interface makes this easy! You might have noticed in the studio that
    your server gets a different file descriptor from `accept()` than
    from `socket()`. In fact, `accept()` returns a different descriptor
    for each individual connection.

-   As a distributed system, some features (like keeping track of a
    name) may be done in the client or in the server. It\'s up to you to
    decide how to implement features.

-   The client should be prepared to display a message at all times,
    even if the program is also waiting for user input. A great way to
    accomplish this is to use a `pthread` to wait for server messages
    and print them to the screen when they arrive. Don\'t worry about
    incoming messages messing up the display of half-written messages.

-   The first major challenge in your server is keeping track of an
    arbitrary number of clients that can connect and drop out at any
    time. This requires a data structure such as a linked list.
    But, C doesn't provide a standard linked-list data structure, so
    you'll need to build your own! You can define a struct containing
    a set of data fields and a `next` pointer, use `malloc()` to
    allocate new nodes, and `free()` to remove existing nodes.

-   The second major challenge in writing your server is the
    asynchronous nature of the system. Your server needs to be able to
    respond to a message from any client quickly, and in particular your
    server cannot block (wait) on calls to `read()` sockets or calls to
    `accept()` new connections. You can set sockets to be *nonblocking*.
    With a nonblocking socket calls to `read()` will fail and return an
    error if there is nothing to read. Similarly, calls to `accept()`
    will fail and return an error if there are no new connections to
    accept. In both cases the function returns an error value and sets
    `errno` to be one of `EAGAIN` or `EWOULDBLOCK`.

-   Search the `accept4()` man page and the `socket()` man page for the
    phrase \"`SOCK_NONBLOCK`\" for details on how to make sockets
    nonblocking. You will want to set the socket returned from
    `socket()` as nonblocking, as well as the sockets returned from
    `accept()` (for this second one, use the `accept4()` variant).

-   Use `sprintf()` to format strings (char buffers) easily. For
    example, if you have a `name` string and a `message` string, and you
    want to combine these into a `toSend` string, you can use something
    like:

    `sprintf(toSend, "%s: %s", name, message);`

    If you find yourself wanting to use `strcpy()` or `strncpy()`,
    consider using `sprintf()` instead.

-   Make sure your client only uses the commands `name` and `quit` when
    it is appropriate. You shouldn\'t let clients set their names to be
    empty. The client shouldn\'t quit on the input \"quite right!\"

-   Each client should only make one connection. Call `connect()` only
    once in your client code. In particular, don\'t call `connect()`
    every time you send a message. The connection that is established
    can be used over and over.

### Useful man pages

* ip(7) 
* socket(2)
* bind(2)
* listen(2) 
* accept(2) 
* accept4(2) 
* connect(2) 
* shutdown(2) 
* pthread\_create(3) 
* fdopen(3)
* fgets(3)
* fprintf(3)
* snprintf(2)

### Questions

1.  There are no exercises for this lab. Please mark your group clearly
    in your client code, and your server code.

------------------------------------------------------------------------

### Submission

Submit your program code via the Git repository.
