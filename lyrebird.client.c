/*------------------------------------------------------
 *
 *  lyrebird.client.c
 *
 *  this file contains the main function that using 'fork'
 *  to create child processes to decrypt each tweet file.
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
#include "pipe.h"
#include "scheduling.h"
#include "time.h"
#include "memwatch.h"

char *enc_txt;                  /* Used to store encrypted file name */
char *dec_txt;                  /* Used to store decrypted file name */
char *out_time;                 /* Used to store output time */
int main_flag = EXIT_SUCCESS;   /* Used to store return value for main function */

pid_t pid;                      /* Used to store fork pid */
int *pid_array;                 /* Used to store all the child pid (as int) */
int *is_free;                   /* Used to show if a child process with a pid is free now */
int process_number_limit;       /* The max number of processes that can use now */
int process_number_now = -1;    /* The number of processes that already been used now */

int *parent_to_child;           /* Pipe that used to transmit message from parent process to child process */
int *child_to_parent;           /* Pipe that used to transmit message from child process to parent process */

fd_set rfds;                    /* The set of file descriptor */
int max_descriptor;             /* The max number of file descriptor */

int port_number;                        /* The server port number */
char host[NI_MAXHOST];                  /* Used to store the return value of getnameinfo function */
struct ifaddrs *ifaddr, *ifa;           /* Used to store the return value of getifaddrs function */

int sockfd;                                     /* Socket file descriptor */
socklen_t addr_len;                             /* The size of sockaddr_in */
struct sockaddr_in serv_addr, cli_addr;         /* Used to store the server and client address information */

FILE *fuck;

/*
 * Function: Get_time
 * -------------------
 *   This function is used to get current time in specify format
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void get_time(void) {
    time_t raw_time;                /* Used to generate output time */
    struct tm *tmp_time;            /* Used to generate output time */
    time(&raw_time);
    tmp_time = localtime(&raw_time);
    strftime(out_time, TIME_MAXLENGTH, "%a %b %d %H:%M:%S %Y", tmp_time);       /* Format time */
}

/*
 * Function: Clean_up
 * -------------------
 *   This function is used to free memory and close file pointer before program exit
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void clean_up(int step) {               /* Always remember to free all and close file pointer! */
    if (step >= CLEAN_TO_TXT){
        free(enc_txt);
        free(dec_txt);
        free(parent_to_child);
        free(child_to_parent);
        free(pid_array);
        free(is_free);
    }
    if (step >= CLEAN_TO_TIME) free(out_time);
    if (step >= CLEAN_TO_SOCKET) close(sockfd);
}

/*
 * Function: Signal_handler
 * -------------------
 *   This function is used to handle signal
 *
 *   Parameters:
 *      sig_num: the number of the signal
 *
 *   Returns:
 *      void
 */

void signal_handler(int sig_num) {
    switch (sig_num) {
        case SIGINT:
            printf("Can not interrupt (signal #%d) this process, because this is a VERY IMPORTANT task!\n", sig_num);
            break;
        case SIGQUIT:
            printf("Can not quit (signal #%d) this process, because this is a VERY IMPORTANT task!\n", sig_num);
            break;
        case SIGHUP:
            printf("Can not exit (signal #%d) this process, because this is a VERY IMPORTANT task!\n", sig_num);
            break;
    }
}

void check_par(int argc, char *argv[]) {
    if (argc != 3) {                        /* Check if the arguments number is right or not */
        get_time();
        printf("[%s] (Process ID #%d) Arguments number is not right! Usage: %s <ip address> <port number>\n", out_time, getpid(), argv[0]);
        clean_up(CLEAN_TO_TIME);
        exit(EXIT_FAILURE);
    }
}

void get_port_number(char *argv[]) {
    if ((port_number = atoi(argv[2])) == 0) {               /* Get the server port number */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Can not use this port number for TCP connection!\n", out_time, getpid());
        clean_up(CLEAN_TO_TIME);
        exit(EXIT_FAILURE);
    }
}

void create_socket(void) {
    int opt = TRUE;                 /* Used when set socket options */

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {                       /* Create socket to connect server */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Create socket failed!\n", out_time, getpid());
        clean_up(CLEAN_TO_TIME);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {      /* Set socket option to reuseable address (not necessary but good) */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Can not set socket option!\n", out_time, getpid());
        clean_up(CLEAN_TO_SOCKET);
        exit(EXIT_FAILURE);
    }
}

void connect_socket(char *argv[]) {
    memset(&serv_addr, 0, addr_len);                                            /* Initialize server address config */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_number);

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Server host in inet_pton function is not a valid IP address!\n", out_time, getpid());
        clean_up(CLEAN_TO_SOCKET);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, addr_len) < 0) {
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Connect to server %s:%d failed!\n", out_time, getpid(), argv[1], port_number);
        clean_up(CLEAN_TO_SOCKET);
        exit(EXIT_FAILURE);
    }
}

void print_connect_info(char *argv[]) {
    get_time();
    printf("[%s] lyrebird client: PID %d connected to server %s on port %d.\n", out_time, getpid(), argv[1], port_number);
}

void send_connect_msg(void) {
    char mark = CONNECT_MSG;
    send(sockfd, &mark, sizeof(char), 0);
}

void wait_all_child(void) {
    int i;
    for (i = 0; i < process_number_limit; i++) {            /* Parent process wait for all child processes before exit */
        int state;
        pid_t pid = wait(&state);                           /* Wait until found one child process finished */
        printf("Wait child state: %d\n", state);
        if (state != EXIT_SUCCESS) {                                   /* If child process terminate unexpectly! */
            get_time();
            printf("[%s] Child process ID #%d did not terminate successfully.\n", out_time, (int)pid);
        }
        close_ctp_pipe_with_pid(pid);                       /* Close the pipe that uses to read messages from exited child process */
    }
}

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
    char mark;

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

    init_pipe();                                            /* Initialize pipe */

    while (process_number_now + 1 < process_number_limit) {
        process_number_now++;
        pid = fork();                                   /* Fork! */
        if (pid < 0) {                                  /* If fork failed */
            get_time();
            printf("[%s] (Process ID #%d) ERROR: Fork failed! Client will exit now without finish its tasks!\n", out_time, getpid());
            main_flag = EXIT_FAILURE;                              /* Exit non-zero value */
            break;                                      /* Break and still need to wait for all child processes */
        }
        if (pid != 0 ) {                                                 /* If fork successful and is in parent process */
            pid_array[process_number_now] = (int)pid;               /* Store child pid into array */
            close(parent_to_child[process_number_now * 2]);
            close(child_to_parent[process_number_now * 2 + 1]);     /* Close pipes that will not be used */
            is_free[process_number_now] = TRUE;
        }
        if (pid == 0) {                                             /* If in child process */
            int state = 0;
            char mark;
            char enc_txt[FILE_MAXLENGTH];
            char dec_txt[FILE_MAXLENGTH];
            char err_buffer[ERROR_MAXLENGTH];                       /* A buffer used to store the error message */

            close_ptc_pipes_except(process_number_now);             /* Close other pipes except used ptc and ctp */
            close_ctp_pipes_except(process_number_now);
            close(parent_to_child[process_number_now * 2 + 1]);
            close(child_to_parent[process_number_now * 2]);

            mark = CHILD_PROCESS_INIT;
            write(child_to_parent[process_number_now * 2 + 1], &mark, sizeof(char));

            while (TRUE) {                                             /* Keep decrpting until break */
                if (read(parent_to_child[process_number_now * 2], &enc_txt, sizeof(char) * FILE_MAXLENGTH) == 0) break;    /* Break if parent process's pipe closed */
                read(parent_to_child[process_number_now * 2], &dec_txt, sizeof(char) * FILE_MAXLENGTH);
                state = decrypt(enc_txt, dec_txt, err_buffer);
                if (state == MALLOC_FAIL_ERROR) {                         /* If child decryption meet an fatal error */
                    printf("Malloc failed!\n");
                    mark = CHILD_PROCESS_FAILURE;
                    write(child_to_parent[process_number_now * 2 + 1], &mark, sizeof(char));
                    write(child_to_parent[process_number_now * 2 + 1], err_buffer, sizeof(char) * ERROR_MAXLENGTH);
                    break;
                }
                if (state == OPEN_FILE_ERROR) {                                   /* If child process is ready to decrypt another file */
                    printf("Open file failed!\n");
                    mark = CHILD_PROCESS_WARNING;
                    write(child_to_parent[process_number_now * 2 + 1], &mark, sizeof(char));
                    write(child_to_parent[process_number_now * 2 + 1], err_buffer, sizeof(char) * ERROR_MAXLENGTH);
                }
                if (state == EXIT_SUCCESS) {                                   /* If child process is ready to decrypt another file */
                    printf("Decrypt success!\n");
                    mark = CHILD_PROCESS_SUCCESS;
                    write(child_to_parent[process_number_now * 2 + 1], &mark, sizeof(char));
                    write(child_to_parent[process_number_now * 2 + 1], enc_txt, sizeof(char) * FILE_MAXLENGTH);
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

    create_socket();
    connect_socket(argv);
    print_connect_info(argv);
    send_connect_msg();

    while (TRUE) {
        if (fcfs() == FCFS_EXIT)
            break;
    }

    mark = DISCONNECT_SUCC_MSG;
    send(sockfd, &mark, sizeof(char), 0);

    close_all_ptc_pipes();                                  /* Close all parent to child pipes */
    read_rmng_msg();                                        /* Read remaining messages in all child processes */
    wait_all_child();

    if (main_flag == EXIT_SUCCESS) {
        mark = DISCONNECT_SUCC_MSG;
        send(sockfd, &mark, sizeof(char), 0);
        fprintf(fuck, "QUIT: %c\n", mark);
        get_time();
        printf("[%s] lyrebird client: PID %d completed its tasks and is exiting successfully.\n", out_time, getpid());
    } else {
        mark = DISCONNECT_FAIL_MSG;
        send(sockfd, &mark, sizeof(char), 0);
        fprintf(fuck, "QUIT: %c\n", mark);
    }

    clean_up(CLEAN_ALL);                                             /* Always remember to free all and close file pointer! */
    return main_flag;
}
