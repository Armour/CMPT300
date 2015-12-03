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

char fcfs_file_buf[FILE_MAXLENGTH];
char fcfs_err_buf[ERROR_MAXLENGTH];
char fcfs_pid_buf[PID_MAXLENGTH];

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
    init_select_with_socket(sockfd);                          /* Every time we need to initialize file descriptor set */
    if (select(max_descriptor + 1, &rfds, NULL, NULL, NULL) == -1) {                      /* If select function failed */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Select function failed.\n", out_time, getpid());
        main_flag = 1;
        return FCFS_EXIT;
    }
    if (FD_ISSET(sockfd, &rfds)) {
        printf("Recv server msg!\n");
        recv_socket_msg(sockfd, recv_mark);                  /* Read message from server side */
        if (strcmp(recv_mark, CLIENT_EXIT_MSG) == 0) {
            printf("Server ask to quit!\n");
            return FCFS_EXIT;
        }
        recv_socket_msg(sockfd, enc_txt);
        //printf("Recv enc_txt: !%s!\n", enc_txt);
        recv_socket_msg(sockfd, dec_txt);
        //printf("Recv dec_txt: !%s!\n", dec_txt);

        for (i = 0; i < process_number_limit; ++i) {            /* If select is OK */
            if (is_free[i] == TRUE) {
                get_time();
                printf("[%s] Child process ID #%d will decrypt %s.\n", out_time, *(pid_array + i), enc_txt);
                write_pipe_msg(parent_to_child[i * 2 + 1], enc_txt);
                write_pipe_msg(parent_to_child[i * 2 + 1], dec_txt);
                is_free[i] = FALSE;
                break;
            }
        }
    } else {
        for (i = 0; i < process_number_limit; ++i) {            /* If select is OK */
            if (FD_ISSET(child_to_parent[i * 2], &rfds)) {
                read_pipe_msg(child_to_parent[i * 2], read_mark);
                if (strcmp(read_mark, CHILD_PROCESS_INIT) == 0) {
                    printf("Send init msg!\n");
                    strcpy(send_mark, DISPATCH_MSG);
                    send_socket_msg(sockfd, send_mark);
                } else if (strcmp(read_mark, CHILD_PROCESS_SUCCESS) == 0) {
                    is_free[i] = TRUE;
                    get_time();
                    read_pipe_msg(child_to_parent[i * 2], fcfs_file_buf);
                    printf("[%s] Child process ID #%d success! decrypt %s.\n", out_time, *(pid_array + i), fcfs_file_buf);

                    printf("Send success msg!\n");
                    strcpy(send_mark, SUCCESS_MSG);
                    sprintf(fcfs_pid_buf, "%d", pid_array[i]);
                    send_socket_msg(sockfd, send_mark);
                    send_socket_msg(sockfd, fcfs_file_buf);
                    send_socket_msg(sockfd, fcfs_pid_buf);
                } else if (strcmp(read_mark, CHILD_PROCESS_WARNING) == 0 || strcmp(read_mark, CHILD_PROCESS_FAILURE) == 0) {
                    if (strcmp(read_mark, CHILD_PROCESS_WARNING) == 0) {
                        is_free[i] = TRUE;
                    }
                    read_pipe_msg(child_to_parent[i * 2], fcfs_err_buf);
                    //get_time();
                    //printf("[%s] Child process ID #%d have warning/error: %s!\n", out_time, *(pid_array + i), fcfs_err_buf);
                    printf("Send failed msg!\n");
                    strcpy(send_mark, FAILURE_MSG);
                    send_socket_msg(sockfd, send_mark);
                    send_socket_msg(sockfd, fcfs_err_buf);
                } else {
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
