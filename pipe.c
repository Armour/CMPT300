/*------------------------------------------------------
 *
 *  pipe.c
 *
 *  this file contains many functions that related to pipe
 *  including initialize pipe, close pipe and select pipe.
 *
 *  Project         : LyreBird
 *  Name            : Guo Chong
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

#include "pipe.h"
#include "memwatch.h"

/*
 * Function: Init_pipe
 * -------------------
 *   This function is used to initialize the max number of pipe descriptor
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void init_pipe(void) {
    int i;
    processor_number_limit = PROCESSOR_MAX_NUMBER;                      /* Max number of processors for this machine */

    parent_to_child = (int *)malloc(sizeof(int) * processor_number_limit * 2);
    if (parent_to_child == NULL) {
        get_time();
        printf("[%s] (Processor ID #%d) ERROR: Malloc parent_to_child failed!\n", out_time, getpid());
        free(out_time);
        exit(1);
    }

    child_to_parent = (int *)malloc(sizeof(int) * processor_number_limit * 2);
    if (child_to_parent == NULL) {
        get_time();
        printf("[%s] (Processor ID #%d) ERROR: Malloc child_to_parent failed!\n", out_time, getpid());
        free(out_time);
        free(parent_to_child);
        exit(1);
    }

    pid_array = (int *)malloc(sizeof(int) * processor_number_limit);
    if (pid_array == NULL) {
        get_time();
        printf("[%s] (Processor ID #%d) ERROR: Malloc pid_array failed!\n", out_time, getpid());
        free(out_time);
        free(parent_to_child);
        free(child_to_parent);
        exit(1);
    }

    memset(pid_array, 0, sizeof(int) * processor_number_limit);

    for (i = 0; i < processor_number_limit; ++i) {
        if (pipe(parent_to_child + i * 2)) {                            /* Create pipe from parent to child */
            get_time();
            printf("[%s] (Processor ID #%d) ERROR: Create parent to child pipe number %d failed!\n", out_time, getpid(), i);
            free(out_time);
            free(parent_to_child);
            free(child_to_parent);
            free(pid_array);
            exit(1);                                                    /* If create pipe failed */
        }
        if (pipe(child_to_parent + i * 2)) {                            /* Create pipe from child to parent */
            get_time();
            printf("[%s] (Processor ID #%d) ERROR: Create child to parent pipe number %d failed!\n", out_time, getpid(), i);
            free(out_time);
            free(parent_to_child);
            free(child_to_parent);
            free(pid_array);
            exit(1);                                                    /* If create pipe failed */
        }
    }
}

/*
 * Function: Init_select
 * -------------------
 *   This function is used to prepare select function for pipe in different processors
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void init_select(void) {
    int i;
    int max = 0;
    FD_ZERO(&rfds);                                             /* First initialize set to empty */
    for (i = 0; i < processor_number_limit; ++i) {
        FD_SET(child_to_parent[i * 2], &rfds);                  /* Add file descriptor to set */
        if (child_to_parent[i * 2] > max)
            max = child_to_parent[i * 2];                       /* Calculate max file descriptor */
    }
    max_descriptor = max;
}

/*
 * Function: Close_ptc_pipes_except
 * -------------------
 *   This function is used to close the pipes from parent to child except processor p's pipe
 *
 *   Parameters:
 *      p: the number of the processor which we do not want to close its pipe
 *
 *   Returns:
 *      void
 */

void close_ptc_pipes_except(int p) {
    int i;
    for (i = 0; i < processor_number_limit * 2; ++i) {                  /* Close each pipe from parent to child except processor p */
        if (i/2 != p)
            close(parent_to_child[i]);
    }
}

/*
 * Function: Close_ctp_pipes_except
 * -------------------
 *   This function is used to close the pipes from child to parent except processor p's pipe
 *
 *   Parameters:
 *      p: the number of the processor which we do not want to close its pipe
 *
 *   Returns:
 *      void
 */

void close_ctp_pipes_except(int p) {
    int i;
    for (i = 0; i < processor_number_limit * 2; ++i) {                  /* Close each pipe from child to parent except processor p  */
        if (i/2 != p)
            close(child_to_parent[i]);
    }
}

/*
 * Function: Close_all_ptc_pipes
 * -------------------
 *   This function is used to close all the pipes from parent to child processor
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void close_all_ptc_pipes(void) {
    int i;
    for (i = 0; i < processor_number_limit * 2; ++i) {                  /* Close all the pipes from parent to child */
            close(parent_to_child[i]);
    }
}

/*
 * Function: Close_ctp_pipe_with_pid
 * -------------------
 *   This function is used to close pipe that belong to child processor with specific pid
 *
 *   Parameters:
 *      pid: the pid number that we want to match
 *
 *   Returns:
 *      void
 */

void close_ctp_pipe_with_pid(int pid) {
    int i;
    for (i = 0; i < processor_number_limit; i++) {
        if (pid_array[i] == pid) {
            close(child_to_parent[2 * i]);
            break;
        }
    }
}

/*
 * Function: Read_rmng_msg
 * -------------------
 *   This function is used to read all remaining messages in each child processor
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void read_rmng_msg(void) {
    int i;
    for (i = 0; i < processor_number_limit; i++) {          /* Read all remaining message in child processors */
        int message;
        while (read(child_to_parent[i * 2], &message, sizeof(int)));
    }
}
