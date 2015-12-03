/*------------------------------------------------------
 *
 *  lyrebird.client.c
 *
 *  this file contains the main function that using 'fork'
 *  to create child processes to decrypt each tweet file,
 *  more detail functions are in the cli_func.c.
 *
 *  Project         : LyreBird
 *  Name            : Chong Guo
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

#include "lyrebird.client.h"
#include "decrypt.h"
#include "scheduling.h"
#include "memwatch.h"

int main_flag = EXIT_SUCCESS;           /* Used to store return value for main function */
char *enc_txt;                          /* Used to store encrypted file name */
char *dec_txt;                          /* Used to store decrypted file name */
char *out_time;                         /* Used to store output time */

pid_t pid;                              /* Used to store fork pid */
int *pid_array;                         /* Used to store all the child pid (as int) */
int *is_free;                           /* Used to show if a child process with a pid is free now */
int process_number_limit;               /* The max number of processes that can use now */
int process_number_now = -1;            /* The number of processes that already been used now */

int *parent_to_child;                   /* Pipe that used to transmit message from parent process to child process */
int *child_to_parent;                   /* Pipe that used to transmit message from child process to parent process */

fd_set rfds;                            /* The set of file descriptor */
int max_descriptor;                     /* The max number of file descriptor */

int port_number;                            /* The server port number */
char host[NI_MAXHOST];                      /* Used to store the return value of getnameinfo function */
struct ifaddrs *ifaddr, *ifa;               /* Used to store the return value of getifaddrs function */

int sockfd;                                     /* Socket file descriptor */
socklen_t addr_len;                             /* The size of sockaddr_in */
struct sockaddr_in serv_addr, cli_addr;         /* Used to store the server and client address information */

uint32_t msg_len;                               /* The length of sent message */
char send_mark[MARK_MAXLENGTH];                 /* A buffer that used to store sent message */
char recv_mark[MARK_MAXLENGTH];                 /* A buffer that used to store received message */
char read_mark[MARK_MAXLENGTH];                 /* A buffer that used to store read message */
char write_mark[MARK_MAXLENGTH];                /* A buffer that used to store write message */

FILE *fuck;

/*
 * Function: Main
 * -------------------
 *   Main function of lyrebird
 *
 *   Parameters:
 *      argc: the number of parameters in main function
 *      argv[]: the array of parameters in main function
 *
 *   Returns:
 *      void
 */

int main(int argc, char *argv[]) {
    fuck = fopen("shit.txt", "w");             /* Output log file */

    //signal(SIGINT, signal_handler);
    //signal(SIGQUIT, signal_handler);
    //signal(SIGHUP, signal_handler);

    addr_len = sizeof(struct sockaddr_in);
    out_time = (char *)malloc(sizeof(char) * TIME_MAXLENGTH);
    enc_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);
    dec_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);

    check_par(argc, argv);
    get_port_number(argv);

    create_socket();
    init_pipe();                                            /* Initialize pipe */

    while (process_number_now + 1 < process_number_limit) {
        process_number_now++;
        pid = fork();                                       /* Fork! */
        if (pid < 0) {                                      /* If fork failed */
            get_time();
            printf("[%s] (Process ID #%d) ERROR: Fork failed! Client will exit now without finish its tasks!\n", out_time, getpid());
            main_flag = EXIT_FAILURE;                       /* Exit non-zero value */
            break;                                          /* Break and still need to wait for all child processes */
        }
        if (pid != 0 ) {                                            /* If fork successful and is in parent process */
            pid_array[process_number_now] = (int)pid;               /* Store child pid into array */
            close(parent_to_child[process_number_now * 2]);
            close(child_to_parent[process_number_now * 2 + 1]);     /* Close pipes that will not be used */
            is_free[process_number_now] = TRUE;
        }
        if (pid == 0) {                                             /* If in child process */
            int state = 0;
            char enc_txt[FILE_MAXLENGTH];
            char dec_txt[FILE_MAXLENGTH];
            char err_buffer[ERROR_MAXLENGTH];                       /* A buffer used to store the error message */

            close_ptc_pipes_except(process_number_now);             /* Close other pipes except used ptc and ctp */
            close_ctp_pipes_except(process_number_now);
            close(parent_to_child[process_number_now * 2 + 1]);
            close(child_to_parent[process_number_now * 2]);

            strcpy(write_mark, CHILD_PROCESS_INIT);
            write_pipe_msg(child_to_parent[process_number_now * 2 + 1], write_mark);

            while (TRUE) {                                                  /* Keep decrpting until break */
                if (read_pipe_msg(parent_to_child[process_number_now * 2], enc_txt) == 0) break;     /* Break if parent process's pipe closed */
                read_pipe_msg(parent_to_child[process_number_now * 2], dec_txt);
                printf("Child process decrypt %s and %s now!\n", enc_txt, dec_txt);
                state = decrypt(enc_txt, dec_txt, err_buffer);
                if (state == MALLOC_FAIL_ERROR) {                           /* If child decryption meet an fatal error */
                    printf("Malloc failed!\n");
                    strcpy(write_mark, CHILD_PROCESS_FAILURE);
                    write_pipe_msg(child_to_parent[process_number_now * 2 + 1], write_mark);
                    write_pipe_msg(child_to_parent[process_number_now * 2 + 1], err_buffer);
                    break;
                }
                if (state == OPEN_FILE_ERROR) {                                   /* If child process is ready to decrypt another file */
                    printf("Open file failed!\n");
                    strcpy(write_mark, CHILD_PROCESS_WARNING);
                    write_pipe_msg(child_to_parent[process_number_now * 2 + 1], write_mark);
                    write_pipe_msg(child_to_parent[process_number_now * 2 + 1], err_buffer);
                }
                if (state == EXIT_SUCCESS) {                                   /* If child process is ready to decrypt another file */
                    printf("Decrypt success!\n");
                    strcpy(write_mark, CHILD_PROCESS_SUCCESS);
                    write_pipe_msg(child_to_parent[process_number_now * 2 + 1], write_mark);
                    write_pipe_msg(child_to_parent[process_number_now * 2 + 1], enc_txt);
                }
            }

            printf("Parent close!\n");
            close(parent_to_child[process_number_now * 2]);             /* Close used pipes */
            close(child_to_parent[process_number_now * 2 + 1]);
            clean_up(CLEAN_ALL);                                                /* Always remember to free all and close file pointer! */
            exit(state == MALLOC_FAIL_ERROR? EXIT_FAILURE: EXIT_SUCCESS);
        }
    }

    process_number_now = process_number_limit;

    connect_socket(argv);
    print_connect_info(argv);
    send_connect_msg();

    while (TRUE) {
        if (fcfs() == FCFS_EXIT)
            break;
    }

    close_all_ptc_pipes();                                  /* Close all parent to child pipes */
    read_rmng_msg();                                        /* Read remaining messages in all child processes */
    wait_all_child();

    if (main_flag == EXIT_SUCCESS) {
        printf("Send exit success mark!\n");
        strcpy(send_mark, DISCONNECT_SUCC_MSG);
        send_socket_msg(sockfd, send_mark);
        get_time();
        printf("[%s] lyrebird client: PID %d completed its tasks and is exiting successfully.\n", out_time, getpid());
    } else {
        printf("Send exit failed mark!\n");
        strcpy(send_mark, DISCONNECT_FAIL_MSG);
        send_socket_msg(sockfd, send_mark);
    }

    clean_up(CLEAN_ALL);                                             /* Always remember to free all and close file pointer! */
    return main_flag;
}
