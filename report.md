<center><h1>Lyrebird</h1></center>


## Overview
This program is used to decrypt large amounts of encrypted tweet files, to reduce the overall running time and take advantages of multiple CPU cores, it used fork function to spread tasks across multiple processes. 

## Design && Implementation
The entry point of this project is `lyrebird.c`, which contains the main loop function and uses fork function to do multiple processes decryption. For each child process, it calls decryption function. The implementation of decryption function is in `decrypt.c` and `dec_fun.c`, while `decrypt.c` has the control flow and `dec_fun.c` has all the functions used to decrypt tweet in each step. Also, I made the input and output of tweet into a single module, called `line_io.c`, I think it is good for future development. Last but not least, `memwatch.c` is used for detecting un-freed memory and some buffer problems like overflow and underflow.

Note that, in `lyrebird.c`, there is a while loop that used to read each line of config file, everytime we get one line input, we try to fork a child process to do the decryption, after all child processes has been created, parent process begin to wait all child processes until they finished, at last it free all the memory, close the file pointer and exit. In child process, if the decryption is done, it will free the memory, print a message and exit with 0, but if it failed, it will exit a non-zero value, parent process will catch that (`wait` function) and print a message with this child process's pid.

## Testing
In my testing, I first tried examples in assignment outline, and then to make sure my program can handle **minimal** and **maximal** size of tweet, I included the data file in assignment 1 into the config file. What's more, in order to test the robustness of my program, I tried with **wrong number of parameters** in main function and **misspelled the config file name** on purpose to see if it can give a error message like *"argument parameter is not right, usage: xxxx"* and *"input file not found"*. Also the most important part is to see whether my program's fork function is run on the right track, there should be **no zombie process** after my program finished, so I used `ps` and `kill` command to check that and result shows everything is well.
