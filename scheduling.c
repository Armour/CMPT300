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
#include "memwatch.h"

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
 *      FCFS_CONT if success, FCFS_EXIT if failed
 */

int fcfs(void) {
    int i;
    init_select_with_socket(sockfd);                        /* Every time we need to initialize file descriptor set */
    if (select(max_descriptor + 1, &rfds, NULL, NULL, NULL) == -1) {                      /* If select function failed */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Select function failed.\n", out_time, getpid());
        main_flag = 1;
        return FCFS_EXIT;
    }
    if (FD_ISSET(sockfd, &rfds)) {                          /* If get message from server */
        recv_socket_msg(sockfd, recv_mark);                 /* Read message from server side */
        if (strcmp(recv_mark, CLIENT_EXIT_MSG) == 0) {
            return FCFS_EXIT;
        }
        recv_socket_msg(sockfd, enc_txt);                   /* If not ask exit, then need to do decryption, so we recv enc_txt and dec_txt */
        recv_socket_msg(sockfd, dec_txt);
        for (i = 0; i < process_number_limit; ++i) {                /* If select is OK */
            if (is_free[i] == TRUE) {
                write_pipe_msg(parent_to_child[i * 2 + 1], enc_txt);                /* Assign task to child process */
                write_pipe_msg(parent_to_child[i * 2 + 1], dec_txt);
                is_free[i] = FALSE;                                 /* Set this child process state to NOT FREE */
                break;
            }
        }
    } else {                                                /* If get message from child process */
        for (i = 0; i < process_number_limit; ++i) {
            if (FD_ISSET(child_to_parent[i * 2], &rfds)) {
                read_pipe_msg(child_to_parent[i * 2], read_mark);
                if (strcmp(read_mark, CHILD_PROCESS_INIT) == 0) {           /* If messge is init message */
                    strcpy(send_mark, DISPATCH_MSG);
                    send_socket_msg(sockfd, send_mark);
                } else if (strcmp(read_mark, CHILD_PROCESS_SUCCESS) == 0) {         /* If message is success messge */
                    is_free[i] = TRUE;
                    read_pipe_msg(child_to_parent[i * 2], fcfs_file_buf);           /* Read successful decrypted file name */
                    strcpy(send_mark, SUCCESS_MSG);
                    sprintf(fcfs_pid_buf, "%d", pid_array[i]);
                    send_socket_msg(sockfd, send_mark);                             /* Send message to server side about this successful decryption */
                    send_socket_msg(sockfd, fcfs_file_buf);
                    send_socket_msg(sockfd, fcfs_pid_buf);
                } else if (strcmp(read_mark, CHILD_PROCESS_WARNING) == 0 || strcmp(read_mark, CHILD_PROCESS_FAILURE) == 0) {        /* If meet warning or failure */
                    if (strcmp(read_mark, CHILD_PROCESS_WARNING) == 0) {            /* If message is warning message */
                        is_free[i] = TRUE;                                          /* Set this child process state to FREE */
                    }
                    read_pipe_msg(child_to_parent[i * 2], fcfs_err_buf);            /* Read error messge from child process */
                    strcpy(send_mark, FAILURE_MSG);
                    send_socket_msg(sockfd, send_mark);                             /* Send error message to server side */
                    send_socket_msg(sockfd, fcfs_err_buf);
                } else {                                                    /* If message can not be identify */
                    get_time();
                    printf("[%s] (Process ID #%d) ERROR: Wrong message has been read from pipe after select.\n", out_time, getpid());
                    main_flag = EXIT_FAILURE;
                    return FCFS_EXIT;
                }
            }
        }
    }
    return FCFS_CONT;
}
