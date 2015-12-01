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


int fcfs(void) {
    int i;
    uint32_t mark;
    char buffer[ERROR_MAXLENGTH];
    init_select_with_socket(sockfd);                          /* Every time we need to initialize file descriptor set */
    if (select(max_descriptor + 1, &rfds, NULL, NULL, NULL) == -1) {                      /* If select function failed */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Select function failed.\n", out_time, getpid());
        main_flag = 1;
        return FCFS_EXIT;
    }
    if (FD_ISSET(sockfd, &rfds)) {
        read(sockfd, &mark, sizeof(uint32_t));                /* Read message from server side */
        printf("Get server info! : %u\n", ntohl(mark));
        if (ntohl(mark) == CLIENT_EXIT_MSG) {
            return FCFS_EXIT;
        }
        read(sockfd, enc_txt, sizeof(char) * FILE_MAXLENGTH);
        read(sockfd, dec_txt, sizeof(char) * FILE_MAXLENGTH);
        for (i = 0; i < process_number_limit; ++i) {            /* If select is OK */
            if (is_free[i] == TRUE) {
                get_time();
                printf("[%s] Child process ID #%d will decrypt %s.\n", out_time, *(pid_array + i), enc_txt);
                write(parent_to_child[i * 2 + 1], enc_txt, sizeof(char) * FILE_MAXLENGTH);
                write(parent_to_child[i * 2 + 1], dec_txt, sizeof(char) * FILE_MAXLENGTH);
                is_free[i] = FALSE;
                break;
            }
        }
    } else {
        for (i = 0; i < process_number_limit; ++i) {            /* If select is OK */
            if (FD_ISSET(child_to_parent[i * 2], &rfds)) {
                uint32_t message;                               /* The message that read from child process's pipe */
                uint32_t mark;                                  /* The mark that used to write to server side */
                read(child_to_parent[i * 2], &message, sizeof(uint32_t));
                message = ntohl(message);
                switch (message) {
                    case CHILD_PROCESS_INIT:
                        mark = htonl(DISPATCH_MSG);
                        write(sockfd, &mark, sizeof(uint32_t));
                        fprintf(fuck, "Init: %u\n", ntohl(mark));
                        break;
                    case CHILD_PROCESS_SUCCESS:
                        is_free[i] = TRUE;
                        get_time();
                        read(child_to_parent[i * 2], buffer, sizeof(char) * FILE_MAXLENGTH);
                        printf("[%s] Child process ID #%d success! decrypt %s.\n", out_time, *(pid_array + i), buffer);
                        mark = htonl(SUCCESS_MSG);
                        write(sockfd, &mark, sizeof(uint32_t));
                        fprintf(fuck, "Success %u\n", ntohl(mark));
                        write(sockfd, buffer, sizeof(char) * FILE_MAXLENGTH);
                        fprintf(fuck, "Success file %s\n", buffer);
                        mark = htonl(pid_array[i]);
                        write(sockfd, &mark, sizeof(uint32_t));
                        fprintf(fuck, "Success pid %u\n", ntohl(mark));
                        break;
                    case CHILD_PROCESS_WARNING:
                        is_free[i] = TRUE;
                    case CHILD_PROCESS_FAILURE:
                        get_time();
                        read(child_to_parent[i * 2], buffer, sizeof(char) * ERROR_MAXLENGTH);
                        printf("[%s] Child process ID #%d have warning/error: %s!\n", out_time, *(pid_array + i), buffer);
                        mark = htonl(FAILURE_MSG);
                        write(sockfd, &mark, sizeof(uint32_t));
                        fprintf(fuck, "Error/warning %u\n", ntohl(mark));
                        write(sockfd, buffer, sizeof(char) * ERROR_MAXLENGTH);
                        fprintf(fuck, "Error/warning msg %s\n", buffer);
                        break;
                   default:                                /* If message is not right */
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
