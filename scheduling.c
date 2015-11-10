/*------------------------------------------------------
 *
 *  scheduing.c
 *
 *  this file contains two functions, each contains a scheduing algorithm
 *  that used to assign tasks to child processes in parent process.
 *
 *  Project         : LyreBird
 *  Name            : Chong Guo
 *  Student ID      : 301295753
 *  SFU username    : armourg
 *  Lecture section : D1
 *  Instructor      : Brain G.Booth
 *  TA              : Scott Kristjanson
 *
 *  Created by Armour on 05/11/2015
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *------------------------------------------------------
 */

#include "scheduling.h"
#include "pipe.h"
#include "memwatch.h"

/*
 * Function: Round_robin
 * -------------------
 *   This function is "round robin" scheduling algorithm which is
 *   used to assign tasks to child processes in parent process.
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void round_robin(void) {
    get_time();                             /* Let process with number cnt_rr decrypt this tweet */
    printf("[%s] Child process ID #%d will decrypt %s.\n", out_time, *(pid_array + cnt_rr), enc_txt);
    write(parent_to_child[cnt_rr * 2 + 1], enc_txt, sizeof(char) * FILE_MAXLENGTH);
    write(parent_to_child[cnt_rr * 2 + 1], dec_txt, sizeof(char) * FILE_MAXLENGTH);

    cnt_rr++;                               /* Update counter of round robin */
    if (cnt_rr == process_number_limit)
        cnt_rr = 0;
}

/*
 * Function: Fcfs
 * -------------------
 *   This function is "first come first serve" scheduling algorithm which is
 *   used to assign tasks to child processes in parent process.
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */


void fcfs(void) {
    int i;
    int state;                              /* Return value of select function */
    init_select();                          /* Every time we need to initialize file descriptor set */
    state = select(max_descriptor + 1, &rfds, NULL, NULL, NULL);
    if (state == -1) {                      /* If select function failed */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Select function failed, it returned -1.\n", out_time, getpid());
        main_flag = 1;
    } else if (state) {                     /* If select is OK */
        for (i = 0; i < process_number_limit; ++i) {
            if (FD_ISSET(child_to_parent[i * 2], &rfds)) {
                int message;                            /* The message that read from child process's pipe */
                read(child_to_parent[i * 2], &message, sizeof(int));
                if (message == i) {                     /* If message from child process shows it is ready */
                    get_time();
                    printf("[%s] Child process ID #%d will decrypt %s.\n", out_time, *(pid_array + i), enc_txt);
                    write(parent_to_child[i * 2 + 1], enc_txt, sizeof(char) * FILE_MAXLENGTH);
                    write(parent_to_child[i * 2 + 1], dec_txt, sizeof(char) * FILE_MAXLENGTH);
                    break;
                } else {                                /* If message is not right */
                    get_time();
                    printf("[%s] (Process ID #%d) ERROR: Wrong message has been read from pipe after select.\n", out_time, getpid());
                    main_flag = 1;
                    break;
                }
            }
        }
    }
}
