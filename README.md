# Lyrebird

<img src="https://cloud.githubusercontent.com/assets/5276065/11606033/66bf0d02-9ac6-11e5-8bac-a38612b32e72.png" alt= "lyrebird" height="150" weight="150">

## Description

This program aims to **decrypt large numbers of encrypted tweets**. To **take advantages of multiple machines**, it uses a distributed system with *client-server* architecture. Server will assign tasks to the clients and store messages to the log file; client will decrypt each task assigned and send result message back. To **take advantages of multiple CPU cores**, each client uses fork function to spread the decryption tasks across multiple processes and uses pipes to communicate between the parent and child processes using "first come first serve" scheduling algorithm.

Same as before, each tweet’s decryption follows four steps:

1. Remove unnecessary characters in a regular interval.

2. Transform each group of 6 characters into an integer using base 41.

3. Map each cipher number onto a similar plain-text number.

4. Get the final decrypted text by using the inverse function of Step 2.

(More details can be found in [Assignment 4 Outline - A Distributed Flock of Lyrebirds](https://github.com/Armour/CMPT300-Assignment/files/51661/Assignment_4_Outline.pdf))

## Project Structure

* **"lyrebird.server.c"**   : contains the main loop function for the server side which :
  1. regularly accept new clients and assign tasks to them;
  2. keep receiving messages from clients and write them to the log file;
  3. wait for all clients finish their tasks;
  4. exit.
* **"lyrebird.client.c"**   : contains the main loop function for the client side which :
  1. use fork function to create child processes to decrypt each tweet file;
  2. keep communicating with the server side.
* **"svr_func.c"**   : contains many functions that used in the server side
* **"cli_func.c"**   : contains many functions that used in the client side
* **"scheduling.c"** : contains one function, which is 'fcfs' scheduling algorithm that is used to assign tasks to the child processes in the parent process.
* **"decrypt.c"**    : contains a decryption function, which goes through 4 steps. In each step it calls some functions in dec_func.c to handle decryption.
* **"dec_func.c"**   : contains many functions that are used to decrypt the tweets, like removing extra characters, transforming between characters and integers.
* **"line_io.c"**    : contains two functions, which input one line data from file and output one line data to file
* **"memwatch.c"**   : mainly used for un-freed memory detection, overflow and underflow to memory buffers.

## How to Use

1. Unzip all the files into one folder and cd into it.
2. Run command `make`, having all the source file compiled and linked.
3. Run command `./lyrebird.server <config_file> <log_file>` to set up a server.
   * (e.g. "./lyrebird.server config_file.txt log_file.txt")
4. Run command `./lyrebird.client <ip address> <port number>` to set up a client and connect it to the server (multiple clients supported).
   * (e.g. "./lyrebird.client 207.23.199.37 58295")
5. Run command `make clean` to remove all intermediate files.

## Robustness

* Check the number of command line parameters.
* Check the existence of input and output file.
* Dynamically manage memory and gaurantee to free all when program finishs (also check any failure of malloc function).
* Check IP address and port number.
* Parent process does not exit until all the child processes finish.
* Server does not exit until all the clients quit.
* Close all unnecessary pipes in each process.
* Test empty data and largest data.

## Reference List

[Beej's Guide to Network Programming](http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html)

[Linux man page](http://linux.die.net/man)

## Author Information
*  Name            : Chong Guo
*  Student ID      : 301295753
*  SFU username    : armourg
*  Lecture section : D1
*  Instructor      : Brain G.Booth
*  TA              : Scott Kristjanson
