/*------------------------------------------------------
 *
 *  lyrebird.c
 *
 *  Project         : LyreBird
 *  Name            : Guo Chong
 *  Student ID      : 301295753
 *  SFU username    : armourg
 *  Lecture section : D1
 *  Instructor      : Brain G.Booth
 *  TA              : Scott Kristjanson
 *
 *  Created by Armour on 25/09/2015
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *------------------------------------------------------
 */

#include "lyrebird.h"
#include "memwatch.h"

int main(int argc, char *argv[]) {
    printf("--beginning of program\n");

    int counter = 0;
    pid_t pid = fork();

    if (pid == 0) {
        // child process
        int i;
        for (i = 0; i < 500; ++i) {
            printf("child process: counter=%d\n", ++counter);
        }
    } else if (pid > 0) {
        // parent process
        int j;
        for (j = 0; j < 500; ++j) {
            printf("parent process: counter=%d\n", ++counter);
        }
    } else {
        // fork failed
        printf("fork() failed!\n");
        return 1;
    }

    printf("--end of program--\n");

    return 0;
}
