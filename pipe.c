/*------------------------------------------------------
 *
 *  pipe.c
 *
 *  this file contains many functions that related to pipe
 *  including initialize pipe, close pipe and select pipe.
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
    process_number_limit = PROCESS_MAX_NUMBER;                          /* Max number of processes for this machine */

    parent_to_child = (int *)malloc(sizeof(int) * process_number_limit * 2);
    if (parent_to_child == NULL) {                                      /* Create parent to child pipes and check whether failed or not */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Malloc parent_to_child failed!\n", out_time, getpid());
        free(out_time);
        exit(EXIT_FAILURE);
    }

    child_to_parent = (int *)malloc(sizeof(int) * process_number_limit * 2);
    if (child_to_parent == NULL) {                                      /* Create child to parent pipes and check whether failed or not */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Malloc child_to_parent failed!\n", out_time, getpid());
        free(out_time);
        free(parent_to_child);
        exit(EXIT_FAILURE);
    }

    pid_array = (int *)malloc(sizeof(int) * process_number_limit);
    if (pid_array == NULL) {                                            /* Create child pid array and check whether failed or not */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Malloc pid_array failed!\n", out_time, getpid());
        free(out_time);
        free(parent_to_child);
        free(child_to_parent);
        exit(EXIT_FAILURE);
    }

    is_free = (int *)malloc(sizeof(int) * process_number_limit);
    if (is_free == NULL) {                                            /* Create child pid array and check whether failed or not */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Malloc is_free failed!\n", out_time, getpid());
        free(out_time);
        free(parent_to_child);
        free(child_to_parent);
        free(pid_array);
        exit(EXIT_FAILURE);
    }

    memset(pid_array, 0, sizeof(int) * process_number_limit);
    memset(is_free, 0, sizeof(int) * process_number_limit);

    for (i = 0; i < process_number_limit; ++i) {
        if (pipe(parent_to_child + i * 2)) {                            /* Create pipe from parent to child */
            get_time();
            printf("[%s] (Process ID #%d) ERROR: Create parent to child pipe number %d failed!\n", out_time, getpid(), i);
            free(out_time);
            free(parent_to_child);
            free(child_to_parent);
            free(pid_array);
            free(is_free);
            exit(EXIT_FAILURE);                                                    /* If create pipe failed */
        }
        if (pipe(child_to_parent + i * 2)) {                            /* Create pipe from child to parent */
            get_time();
            printf("[%s] (Process ID #%d) ERROR: Create child to parent pipe number %d failed!\n", out_time, getpid(), i);
            free(out_time);
            free(parent_to_child);
            free(child_to_parent);
            free(pid_array);
            free(is_free);
            exit(EXIT_FAILURE);                                                    /* If create pipe failed */
        }
    }
}

/*
 * Function: Init_select
 * -------------------
 *   This function is used to prepare select function for pipe in different processes
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
    for (i = 0; i < process_number_limit; ++i) {
        FD_SET(child_to_parent[i * 2], &rfds);                  /* Add file descriptor to set */
        if (child_to_parent[i * 2] > max)
            max = child_to_parent[i * 2];                       /* Calculate max file descriptor */
    }
    max_descriptor = max;
}

/*
 * Function: Init_select
 * -------------------
 *   This function is used to prepare select function for pipe in different processes
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void init_select_with_socket(int sockfd) {
    int i;
    int max = 0;
    FD_ZERO(&rfds);                                             /* First initialize set to empty */
    for (i = 0; i < process_number_limit; ++i) {
        FD_SET(child_to_parent[i * 2], &rfds);                  /* Add file descriptor to set */
        if (child_to_parent[i * 2] > max)
            max = child_to_parent[i * 2];                       /* Calculate max file descriptor */
    }
    FD_SET(sockfd, &rfds);                  /* Add file descriptor to set */
    if (sockfd > max)
        max = sockfd;
    max_descriptor = max;
}

/*
 * Function: Close_ptc_pipes_except
 * -------------------
 *   This function is used to close the pipes from parent to child except process p's pipe
 *
 *   Parameters:
 *      p: the number of the process which we do not want to close its pipe
 *
 *   Returns:
 *      void
 */

void close_ptc_pipes_except(int p) {
    int i;
    for (i = 0; i < process_number_limit * 2; ++i) {                  /* Close each pipe from parent to child except process p */
        if (i/2 != p)
            close(parent_to_child[i]);
    }
}

/*
 * Function: Close_ctp_pipes_except
 * -------------------
 *   This function is used to close the pipes from child to parent except process p's pipe
 *
 *   Parameters:
 *      p: the number of the process which we do not want to close its pipe
 *
 *   Returns:
 *      void
 */

void close_ctp_pipes_except(int p) {
    int i;
    for (i = 0; i < process_number_limit * 2; ++i) {                  /* Close each pipe from child to parent except process p  */
        if (i/2 != p)
            close(child_to_parent[i]);
    }
}

/*
 * Function: Close_all_ptc_pipes
 * -------------------
 *   This function is used to close all the pipes from parent to child process
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void close_all_ptc_pipes(void) {
    int i;
    for (i = 0; i < process_number_limit * 2; ++i) {                  /* Close all the pipes from parent to child */
            close(parent_to_child[i]);
    }
}

/*
 * Function: Close_ctp_pipe_with_pid
 * -------------------
 *   This function is used to close pipe that belong to child process with specific pid
 *
 *   Parameters:
 *      pid: the pid number that we want to match
 *
 *   Returns:
 *      void
 */

void close_ctp_pipe_with_pid(int pid) {
    int i;
    for (i = 0; i < process_number_limit; i++) {
        if (pid_array[i] == pid) {                                      /* Close pipes with specific pid */
            close(child_to_parent[2 * i]);
            break;
        }
    }
}

/*
 * Function: Read_rmng_msg
 * -------------------
 *   This function is used to read all remaining messages in each child process
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void read_rmng_msg(void) {
    int i;
    for (i = 0; i < process_number_limit; i++) {          /* Read all remaining message in child processes */
        char message;
        while (read(child_to_parent[i * 2], &message, sizeof(char)));
    }
}
