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


void fcfs(void) {
    int i;
    char buffer[ERROR_MAXLENGTH];
    printf("Before fcfs select! \n");
    init_select();                          /* Every time we need to initialize file descriptor set */
    if (select(max_descriptor + 1, &rfds, NULL, NULL, NULL) == -1) {                      /* If select function failed */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Select function failed.\n", out_time, getpid());
        main_flag = 1;
        return;
    }
    printf("After fcfs select! \n");
    for (i = 0; i < process_number_limit; ++i) {            /* If select is OK */
        if (FD_ISSET(child_to_parent[i * 2], &rfds)) {
            uint32_t message;                               /* The message that read from child process's pipe */
            uint32_t mark;                                  /* The mark that used to write to server side */
            read(child_to_parent[i * 2], &message, sizeof(uint32_t));
            message = ntohl(message);
            printf("Get child message: %u!!!!\n", message);
            switch (message) {
                case CHILD_PROCESS_READY:                    /* If message from child process shows it is ready */
                    get_time();
                    printf("[%s] Child process ID #%d will decrypt %s.\n", out_time, *(pid_array + i), enc_txt);
                    write(parent_to_child[i * 2 + 1], enc_txt, sizeof(char) * FILE_MAXLENGTH);
                    write(parent_to_child[i * 2 + 1], dec_txt, sizeof(char) * FILE_MAXLENGTH);
                    break;
                case CHILD_PROCESS_SUCCESS:
                    get_time();
                    printf("[%s] Child process ID #%d success! decrypt %s.\n", out_time, *(pid_array + i), enc_txt);
                    mark = htonl(SUCCESS_MSG);
                    write(sockfd, &mark, sizeof(uint32_t));
                    fprintf(fuck, "%u\n", ntohl(mark));
                    write(sockfd, enc_txt, sizeof(char) * FILE_MAXLENGTH);
                    fprintf(fuck, "%s\n", enc_txt);
                    mark = htonl(getpid());
                    write(sockfd, &mark, sizeof(uint32_t));
                    fprintf(fuck, "%u\n", ntohl(mark));
                    break;
                case CHILD_PROCESS_FAILURE:
                    get_time();
                    printf("[%s] Child process ID #%d have error when decrypt %s!\n", out_time, *(pid_array + i), enc_txt);
                    read(child_to_parent[i * 2], buffer, sizeof(char) * ERROR_MAXLENGTH);
                    mark = htonl(FAILURE_MSG);
                    write(sockfd, &mark, sizeof(uint32_t));
                    fprintf(fuck, "%u\n", mark);
                    write(sockfd, buffer, sizeof(char) * ERROR_MAXLENGTH);
                    fprintf(fuck, "%s\n", buffer);
                    break;
                case CHILD_PROCESS_WARNING:
                    get_time();
                    printf("[%s] Child process ID #%d have warning when decrypt %s!\n", out_time, *(pid_array + i), enc_txt);
                    read(child_to_parent[i * 2], buffer, sizeof(char) * ERROR_MAXLENGTH);
                    mark = htonl(FAILURE_MSG);
                    write(sockfd, &mark, sizeof(uint32_t));
                    fprintf(fuck, "%u\n", mark);
                    write(sockfd, buffer, sizeof(char) * ERROR_MAXLENGTH);
                    fprintf(fuck, "%s\n", buffer);
                    break;
               default:                                /* If message is not right */
                    get_time();
                    printf("[%s] (Process ID #%d) ERROR: Wrong message has been read from pipe after select.\n", out_time, getpid());
                    main_flag = 1;
                    break;
            }
        }
    }
    printf("After fcfs!\n");
}
