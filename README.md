# Lyrebird

<img src="https://cloud.githubusercontent.com/assets/5276065/11606033/66bf0d02-9ac6-11e5-8bac-a38612b32e72.png" alt= "lyrebird" height="150" weight="150">

## Description

This program aims to **decrypt large numbers of encrypted tweets**.  The program uses a distributed system with *client-server* architecture to **take advantages of multiple machines**. The server assigns tasks to the clients and stores messages in the log file. The client decrypts each task assigned and sends the result message back to the server. The program **takes advantages of multiple CPU cores** by allowing each client to use the fork function to allocate the decryption tasks across multiple processes and pipes to communicate between the parent and child processes with "first come first serve" scheduling algorithm.

Each tweet’s decryption follows four steps:

1. Remove unnecessary characters in a regular interval.

2. Transform each group of 6 characters into an integer using base 41.

3. Map each cipher number onto a similar plain-text number.

4. Get the final decrypted text by using the inverse function of Step 2.

(More details can be found in [Assignment 4 Outline - A Distributed Flock of Lyrebirds](https://github.com/Armour/CMPT300-Assignment/files/51661/Assignment_4_Outline.pdf))

## Project Structure

* **"lyrebird.server.c"**   : contains the main loop function for the server side:
  1. accept new clients regularly and assign tasks to them;
  2. continuously receive messages from clients and write them to the log file;
  3. wait for all clients to finish their tasks;
  4. exit.
* **"lyrebird.client.c"**   : contains the main loop function for the client side:
  1. use fork function to create child processes to decrypt each tweet file;
  2. continuously communicates with the server side.
* **"svr_func.c"**   : contains server side functions
* **"cli_func.c"**   : contains client side functions
* **"scheduling.c"** : contains a single 'fcfs' scheduling algorithm for assigning tasks to the child processes in the parent process.
* **"decrypt.c"**    : contains a decryption function of 4 steps. In each step, the decryption function calls functions in dec_func.c to handle decryption.
* **"dec_func.c"**   : contains functions for decrypting the tweets, such as removing extra characters,and transforming between characters and integers.
* **"line_io.c"**    : contains two functions for input/output one line of data from/to files
* **"memwatch.c"**   : mainly for un-freed memory detection, overflow and underflow to memory buffers.

## How to Use

1. Unzip all the files into one folder and cd into it.
2. Run command `make` to link and compile the source files.
3. Run command `./lyrebird.server <config_file> <log_file>` to set up a server.
   * (e.g. "./lyrebird.server config_file.txt log_file.txt")
4. Run command `./lyrebird.client <ip address> <port number>` to set up a client and connect it to the server (multiple clients supported).
   * (e.g. "./lyrebird.client 207.23.199.37 58295")
5. Run command `make clean` to remove all intermediate files.

## Robustness

* Check the number of command line parameters.
* Check the existence of input and output file.
* Dynamically manage memory and guarantee to free all when program finishes (also check any failure of malloc function).
* Check IP address and port number.
* Parent process does not exit until all the child processes terminate.
* Server does not exit until all the clients quit.
* Close all unnecessary pipes in each process.
* Test empty data and largest data.

## References

[Beej's Guide to Network Programming](http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html)

[Linux man page](http://linux.die.net/man)

## Plagiarism

This repo is intend as a reference for your personal project, **DO NOT copy code in this repo for your SFU CMPT413 assignment!! Plagiarism is a serious academic offense, which applies as much to code as it does to essays and exams.**
