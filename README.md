# Lyrebird

[![Build Status](https://travis-ci.org/Armour/CMPT300-Assignment.svg)](https://travis-ci.org/Armour/CMPT300-Assignment)

---

<img src="https://cloud.githubusercontent.com/assets/5276065/11606033/66bf0d02-9ac6-11e5-8bac-a38612b32e72.png" alt= "lyrebird" height="150" weight="150">

## Description

This program aims to **decrypt large numbers of encrypted tweets**. To speed up and **take advantages of multiple machines**, it used a distributed system with *client-server* architecture, server will assign tasks to clients and store messages to log file, client will decrypt each task that server assigned and send result message back to server side. To **take advantages of multiple CPU cores**, in each client side, it also use fork function to spread the decryption task across multiple processes and use pipes to pass messages between the parent process and child processes (using "first come first serve" scheduling algorithm).

Same as before, each tweet’s decryption follow four steps below:

1. Remove unnecessary characters in regular interval.

2. Transform each group of 6 characters into an integer using base 41.

3. Map each cipher number onto a similar plain-text number.

4. Get the final decrypted text by use the inverse function of Step 2.

(More Details can be found in [Assignment 4 Outline - A Distributed Flock of Lyrebirds](https://github.com/Armour/CMPT300-Assignment/files/51661/Assignment_4_Outline.pdf))

## Project Structure

* **"lyrebird.server.c"**   : contains the main loop function for server side which regularly accept new client and assign tasks to them, keep receiving messages from clients and write them to log file, wait for all clients finish their tasks and then exit.
* **"lyrebird.client.c"**   : contains the main loop function for client side which uses fork function to create child processes to do the decryption for each tweet file and keep communicating with server side.
* **"svr_func.c"**   : contains many functions that used in server side
* **"cli_func.c"**   : contains many functions that used in client side
* **"scheduling.c"** : contains one function, which is 'fcfs' scheduling algorithm that used to assign tasks to child processes in parent process.
* **"decrypt.c"**    : contains a decrypt function, which go through 4 steps and in each step it call some functions in dec_func.c to handle decryption.
* **"dec_func.c"**   : contains many functions that used to decrypt the tweet, like remove extra characters, mapping characters to integer value and mapping integer to characters.
* **"line_io.c"**    : contains two functions, input one line data from file and output one line data to file
* **"memwatch.c"**   : mainly used for detecting un-freed memory, overflow and underflow to memory buffers.

## How to use

1. Unzip all the file into one folder and cd into that folder.
2. Run command `make`, you will see all the source file been compiled and linked.
3. Run command `./lyrebird.server <config_file> <log_file>` to set up a server.
   * (e.g. "./lyrebird.server config_file.txt log_file.txt")
4. Run command `./lyrebird.client <ip address> <port number>` to set up a client and connect it to the server (you can have multiple clients).
   * (e.g. "./lyrebird.client 207.23.199.37 58295")
5. You can run command `make clean` to remove all intermediate files.

## Robustness

* Check the number of command line parameters.
* Check whether the input and output file exist or not.
* Dynamic manage memory and guarantee to free all when program finished (also checked malloc function to see if it failed).
* IP address and port number checked.
* Parent process wait until all his child processes finished before exit.
* Server wait until all the clients quit before exit.
* Close all pipes that do not need in each processes.
* Empty data and largest data tested.

## Reference list

[Beej's Guide to Network Programming](http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html)

[Linux man page](http://linux.die.net/man)

## Author Information
*  Name            : Chong Guo
*  Student ID      : 301295753
*  SFU username    : armourg
*  Lecture section : D1
*  Instructor      : Brain G.Booth
*  TA              : Scott Kristjanson
