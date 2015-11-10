<center><h1>Lyrebird</h1></center>


## Overview
This program is used to **decrypt large amounts of encrypted tweet files**, to reduce the overall running time and take advantages of multiple CPU cores, it used **fork** function to spread tasks across multiple processes and **pipes** to pass messages between the parent process and child processes. As for **scheduling** problem, it will choose one algorithm between "round-robin" and "first come first served" which depends on the input config file.

## Design && Implementation

#### Structure Design
The entry point of this project is `lyrebird.c`, which contains the main loop function and uses fork function to create child processes to do decryption. `pipe.c` used to manage all the function related to pipe and `scheduling.c` contains two scheduling algorithms that used in parent process to assign tasks to child processes. For each child process, it calls decryption function. The implementation of decryption function is in `decrypt.c` and `dec_fun.c`, while `decrypt.c` has the control flow and `dec_fun.c` has all the functions used to decrypt tweet in each step. Also, I made the input and output of tweet into a single module, called `line_io.c`, I think it is good for future development. The last one, `memwatch.c` is used for detecting un-freed memory and some buffer problems like overflow and underflow.

#### Function Implementation
In `lyrebird.c`, there is a while loop that used to read each line of config file, after it read one line input in parent process, it will use `fork` to **create a child process** (if not exceed the number of CPU cores) or use `scheduling algorithm` to **choose one exist child process**, then it uses `pipe` to **pass the file names to child process** (`write` function), child process will get the file names (`read` function) and begin to do the decryption. 

After all tasks has been assigned, parent process will **close all the write pipes** to child processes, begin to **read remaining messages** from child processes until it closes pipe, and then `wait` for all child processes until they finished, everytime one child process exit, parent process will first **close the pipe that uses to read messages from that child process**, and then print a message with this child process's pid. In child process, if the decryption is done, it will free the memory, print a message, **write a message with its process ID to pipe (this message will be used in FCFS algorithm)**  and exit with 0, if it failed, it will exit a non-zero value.

#### Scheduling Algorithm Implementation

* "Round Robin"
    *  used a **counter** to indicate which process should be used next time.
* "FCFS"
    * use `select` function to get a free process, `read` and check its message, if the message has right format with process ID then assign task to this process. 

## Testing
During my testing, to make sure that my decryption is still right, I first tested it with **the data in assignment 2** which includes **maximal size of tweet**, because this program needs a scheduling name in config file, so I **randomly added an algorithm name to each config file**, in order to test the robustness of my program, I tried with **wrong number of parameters** in main function, **wrong scheduling algorithm name** and **nonexistent config file**. 

After that, I tested the **pipe function** through printing each times message in `read` and `write`, make sure **all the pipes that is not used in each process has been closed**. What's more, I checked my **select function**, make sure that before each times select, the **file descriptor set will be reset**. As I expected, all messages (input & output file name, child process exit message) passed well and program exits without blocking.

Another important thing is **scheduling algorithm**, **Round-Robin** is easy to check from **the order of process ID** in printed message, as for **FCFS**, I used the third config file in assignment 2 which contains a huge encrypted tweet file, **the process which decrypt the huge file start as first one but end as the last one**, which conform to the FCFS algorithm. 

Last but not least, there should be **no zombie process** after my program finished, so I used `ps` and `kill` command to check that and result shows everything is well.
