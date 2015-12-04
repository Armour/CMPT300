<center><h1>Lyrebird Assignment #4</h1></center>

## Overview

This program is used to **decrypt large amounts of encrypted tweet files**. To reduce the overall running time and take advantages of both multiple machines and multiple CPU cores, it used a distributed system with **client-server** architecture. Server and client communicate using **sockets**, server regularly accept new client and assign decryption tasks to clients, client uses **fork** function to spread tasks across multiple processes and **pipes** to pass messages between parent and child processes, the result message of decryption will be send back to server and be **recorded in the log file**. As for scheduling, both of server and client use **"first come first served"** with **select** function.

## Design && Implementation

* Server side
    * The server first **get its ip address** using `getifaddrs` (filtered *'0.0.0.0'* and *'127.0.0.1'*), then it uses this ip address to **create, `bind` and `listen` at a socket** with a random port number, if all goes well, server will **print its welcome message** to the terminal which contains its ip address and port number (use `getsockname` to get value). 
    * After that, the server begins a while loop and keep running `select` function to see if there is **a new client ask for connect** or **new message received**. If there is a new client, server will `accept` it and record its information to log file. If there is a new message received from client, the server will handle it in different way according to the message type: 
        * DISPATCH\_MSG: `send` a new pair of encrypt and decrypt file name to client and let it do decryption. 
        * SUCCESS\_MSG: **record** the successful decrypted file name, the client ip address and pid **to log file**.
        * FAILURE\_MSG: **record** the error message, the client ip address and pid **to log file**.
        * DISCONNECT\_MSG: **record** the disconnect message for one client **to log file** and `close` the socket which connected to that client.
    
    * Once **all the decryption tasks has been assigned to clients**, the server will break the while loop and send each connected client a CLIENT\_EXIT\_MSG message to **ask them exit**. After that, server will **wait for all clients' remained messages** and handle it the same way like above until there is no client. Finally, server **prints a end message** and quit.

* Client side
    * The client first try to use `fork` function to **create child processes** with the number of CPU cores, each time a child process created, client will **send a DISPATCH\_MSG to server to ask for task**, and then:
        * In parent process, it creates a socket and try to `connect` to server using the input ip address and port number, once connected with server, it begins to receive both server and child processes' messages using `select` function. It will **assign the task that received from server to its child processes** (using `recv` and `write` function) or **send the decryption feedback message from child process to server side** (using `read` and `send` function).
        * In child process, it receive the decryption task from parent process, finish it and uses `pipe` to **pass the decryption feedback message to parent process**, if doesn't have fatal error, it will wait for new task again, always do like this **until pipe closed by parent process**.
    * When parent process receive the CLIENT\_EXIT\_MSG from server side, it will **close all the write pipes** to child processes, **read all remaining messages** from child processes and **send them to server side**, then it `wait` for all child processes exit and finally exit along with **DISCONNECT\_MSG send to server**.

## Testing

During my testing, to make sure that my decryption is still right, I tested it with **the data in assignment #2 #3** which includes **empty tweet**, **maximal size of tweet**, **vary long name file**, in order to test the robustness of my program, I tried with **wrong number of parameters** in main function, **wrong ip address**, **wrong port number**, **empty config file** and **nonexistent config file**.

After that, I tested the **socket function** through printing each times message in `recv` and `send`, As I expected, all messages (input & output file name, client message) passed well.

Besides that, I also tested:

* multiple clients
* cross platform (ubuntu + MacOS)
* huge amounts of decrypt files (10000+)
* log file line number (should equals to tasks number * 2 + clients number * 2 + 2)

Last but not least, there should be **no zombie process** after my program finished, so I used `ps` and `kill` to check it and result shows everything is well.
