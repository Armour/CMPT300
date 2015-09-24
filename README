# ------------------------------------------------------------------------------------ #
#  ___           ___    ___ ________  _______   ________  ___  ________  ________      #
# |\  \         |\  \  /  /|\   __  \|\  ___ \ |\   __  \|\  \|\   __  \|\   ___ \     #
# \ \  \        \ \  \/  / | \  \|\  \ \   __/|\ \  \|\ /\ \  \ \  \|\  \ \  \_|\ \    #
#  \ \  \        \ \    / / \ \   _  _\ \  \_|/_\ \   __  \ \  \ \   _  _\ \  \ \\ \   #
#   \ \  \____    \/  /  /   \ \  \\  \\ \  \_|\ \ \  \|\  \ \  \ \  \\  \\ \  \_\\ \  #
#    \ \_______\__/  / /      \ \__\\ _\\ \_______\ \_______\ \__\ \__\\ _\\ \_______\ #
#     \|_______|\___/ /        \|__|\|__|\|_______|\|_______|\|__|\|__|\|__|\|_______| #
#              \|___|/                                                                 #
# ------------------------------------------------------------------------------------ #

---

# Basic
This is a program to decrypt encrypted tweet follow four steps below:
1. Remove unnecessary characters in regular interval.
2. Transform each group of 6 characters into an integer using base 41.
3. Map each cipher number onto a similar plain-text number.
4. Get the final decrypted text by use the inverse function of Step 2.
More Details can be found in [Assignment 1 - LyreBird](https://courses.cs.sfu.ca/2015fa-cmpt-300-d1/pages/AssignmentOne/view)

## Author Information
*  Name            : Guo Chong
*  Student ID      : 301295753
*  SFU username    : armourg
*  Lecture section : D1
*  Instructor      : Brain G.Booth
*  TA              : Scott Kristjanson

## Project Structure
* "lyrebird.c" : mainly two functions, one controls the main loop of this program, another is a decrypt function,
                which go through 4 steps and in each step it call some functions in decrypt.c to handle decryption.
* "line_io.c"  : two functions, input one line from file and output one line to file
* "decrypt.c"  : many functions that used to decrypt the tweet, like remove extra characters, mapping characters to
                interger value and mapping integer to characters. (Detail information can be found by reading comments of this file.)
* "memwatch.c" : mainly used for detecting unfreed memory, overflow and underflow to memory buffers.

## How to use
1. Unzip all the file into one folder and cd into that folder.
2. Run command "make", you will see all the source file been compiled and linked.
3. Run command "./lyrebird input_file_name output_file_name".
    (e.g. "./lyrebird encrypted_tweets.txt decrypted_output.txt")
4. You can run command "make clean" to remove all intermediate files.

## Robustness
* Check the number of main function arguments.
* Check whether the input file exist or not.
* Check whether the tweet is multiple of 6 after removeing extra characters.
* Dynamic manage memory and guarantee to free all when program finished.

