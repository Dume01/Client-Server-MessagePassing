# Client-Server-MessagePassing

**1) Write a POSIX-compliant C program `client.c`.**

a) Each instance of this program creates a separate client process when executed.

b) The client process prompts the user to enter a positive integer as its client-id, and the client will display a menu with options.

c) The menu options include:

- Enter 1 to contact the Ping Server
- Enter 2 to contact the File Search Server
- Enter 3 to contact the File Word Count Server
- Enter 4 if this Client wishes to exit

d) The client communicates user commands and necessary arguments to the main server using a message queue.

e) All client-server communications, both from client to main server and from server's child to client, occur through a single message queue using message types (mtype).

f) The client waits for a reply from the server's child, displays the output to the user, and redisplay the menu options until the user chooses option 4. Upon choosing option 4, the client gracefully terminates.

**2) Write a POSIX-compliant C program `server.c` (the main server).**

a) The main server creates the message queue for communication with clients.

b) The main server listens to the message queue for new client requests.

c) Upon receiving a request from a client, the main server spawns a child server to execute the specific task requested by the client. For each client request, a separate child server is created.

d) All communications between the main server and its children are done exclusively through pipes. No other IPC mechanism should be used.

e) For option 1 in the client menu, the client sends "hi" to the server via the message queue, and a child server replies with "hello" to the client through the message queue. After sending the reply, the child server performs cleanup activities and terminates.

f) For options 2 and 3 in the client menu, the child servers use exec() functions and relevant Linux commands to perform the tasks. The child server informs the client if the file exists or communicates the word count via the message queue.

g) The main server continuously listens for new client requests and spawns child servers to service them.

**3) Write a POSIX-compliant C program `cleanup.c`.**

a) The cleanup process runs alongside clients and the main server. It displays a menu as follows:

Do you want the server to terminate? Press Y for Yes and N for No.

b) If 'N' is given as input, the process continues running normally without communicating with any other process. If 'Y' is given as input, the process informs the main server via the message queue that the main server needs to terminate. The cleanup process then terminates. When the main server receives the terminate request from the cleanup process, it deletes the message queue and performs any necessary cleanup tasks before terminating.
