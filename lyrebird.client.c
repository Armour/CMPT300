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

int port_number;                                /* The server port number */
char host[NI_MAXHOST];                          /* Used to store the return value of getnameinfo function */
struct ifaddrs *ifaddr, *ifa;                   /* Used to store the return value of getifaddrs function */

int sockfd;                                     /* Socket file descriptor */
socklen_t addr_len;                             /* The size of sockaddr_in */
struct sockaddr_in serv_addr, cli_addr;         /* Used to store the server and client address information */

uint32_t msg_len;                               /* The length of sent message */
char send_mark[MARK_MAXLENGTH];                 /* A buffer that used to store sent message */
char recv_mark[MARK_MAXLENGTH];                 /* A buffer that used to store received message */
char read_mark[MARK_MAXLENGTH];                 /* A buffer that used to store read message */
char write_mark[MARK_MAXLENGTH];                /* A buffer that used to store write message */

char fcfs_file_buf[FILE_MAXLENGTH];             /* The buffer that used to store file name that read from pipe */
char fcfs_err_buf[ERROR_MAXLENGTH];             /* The buffer that used to store error message that read from pipe */
char fcfs_pid_buf[PID_MAXLENGTH];               /* The buffer that used to store pid number that read from pipe */


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

    init();                                 /* Init some variables (like malloc timestamp string, encrypt text string, etc.) */

    check_par(argc, argv);                  /* Check command arguments number */
    get_port_number(argv);                  /* Get the port number */

    create_socket();                        /* Create a socket */
    init_pipe();                            /* Initalize pipe file descriptors */

    while (process_number_now + 1 < process_number_limit) {             /* Until fork all the child processes */
        process_number_now++;
        pid = fork();                                       /* Fork! */
        if (pid < 0) {                                      /* If fork failed */
            get_time();
            printf("[%s] (Process ID #%d) ERROR: Fork failed! Client will exit now without finish its tasks!\n", out_time, getpid());
            main_flag = EXIT_FAILURE;                       /* Exit non-zero value */
            break;                                          /* Break and still need to wait for all child processes */
        }
        if (pid != 0 ) {                                                /* If fork successful and is in parent process */
            pid_array[process_number_now] = (int)pid;                   /* Store child pid into array */
            close(parent_to_child[process_number_now * 2]);             /* Close pipes that will not be used */
            close(child_to_parent[process_number_now * 2 + 1]);
            is_free[process_number_now] = TRUE;                         /* Set the process state to be FREE */
        }
        if (pid == 0) {                                                 /* If in child process */
            int state = 0;
            char enc_txt[FILE_MAXLENGTH];
            char dec_txt[FILE_MAXLENGTH];
            char err_buffer[ERROR_MAXLENGTH];                           /* A buffer used to store the error message */

            close_ptc_pipes_except(process_number_now);                 /* Close other pipes except used ptc and ctp */
            close_ctp_pipes_except(process_number_now);
            close(parent_to_child[process_number_now * 2 + 1]);
            close(child_to_parent[process_number_now * 2]);

            strcpy(write_mark, CHILD_PROCESS_INIT);                     /* Ask child process state to be INIT */
            write_pipe_msg(child_to_parent[process_number_now * 2 + 1], write_mark);

            while (TRUE) {                                                      /* Keep decrpting until break */
                if (read_pipe_msg(parent_to_child[process_number_now * 2], enc_txt) == 0) break;        /* Break if parent process's pipe closed */
                read_pipe_msg(parent_to_child[process_number_now * 2], dec_txt);
                state = decrypt(enc_txt, dec_txt, err_buffer);
                if (state == MALLOC_FAIL_ERROR) {                               /* If child decryption meet an fatal error */
                    strcpy(write_mark, CHILD_PROCESS_FAILURE);
                    write_pipe_msg(child_to_parent[process_number_now * 2 + 1], write_mark);
                    write_pipe_msg(child_to_parent[process_number_now * 2 + 1], err_buffer);
                    break;
                }
                if (state == OPEN_FILE_ERROR) {                                 /* If child process open file failed, it will still wait for new task */
                    strcpy(write_mark, CHILD_PROCESS_WARNING);
                    write_pipe_msg(child_to_parent[process_number_now * 2 + 1], write_mark);
                    write_pipe_msg(child_to_parent[process_number_now * 2 + 1], err_buffer);
                }
                if (state == EXIT_SUCCESS) {                                    /* If child process is ready to decrypt another file */
                    strcpy(write_mark, CHILD_PROCESS_SUCCESS);
                    write_pipe_msg(child_to_parent[process_number_now * 2 + 1], write_mark);
                    write_pipe_msg(child_to_parent[process_number_now * 2 + 1], enc_txt);
                }
            }

            close(parent_to_child[process_number_now * 2]);                     /* Close used pipes */
            close(child_to_parent[process_number_now * 2 + 1]);
            clean_up(CLEAN_ALL);                                                /* Always remember to free all and close file pointer! */
            exit(state == MALLOC_FAIL_ERROR? EXIT_FAILURE: EXIT_SUCCESS);
        }
    }

    process_number_now = process_number_limit;
    connect_socket(argv);                                   /* Conncet to server socket */
    print_connect_info(argv);                               /* Print connect information */
    send_connect_msg();                                     /* Send conncet message to server */

    while (TRUE) {                                          /* FCFS until server ask to quit */
        if (fcfs() == FCFS_EXIT)
            break;
    }

    close_all_ptc_pipes();                                  /* Close all parent to child pipes */
    read_rmng_msg();                                        /* Read remaining messages in all child processes */
    wait_all_child();                                       /* Wait for all child exit */

    check_client_exit_state();                              /* Check client exit state and send it to server before exit */

    clean_up(CLEAN_ALL);                                    /* Always remember to free all and close file pointer! */
    return main_flag;
}
