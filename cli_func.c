/*------------------------------------------------------
 *
 *  cli_func.c
 *
 *  this file contains many functions that used in lyrebird client side
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

#include "cli_func.h"
#include "time.h"
#include "memwatch.h"

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
 * Function: Send_socket_msg
 * -------------------
 *   This function is used to send a message through socket with its length send first
 *
 *   Parameters:
 *      socket: the socket file descriptor
 *      msg: the message that need to send
 *
 *   Returns:
 *      void
 */

void send_socket_msg(int socket, char *msg) {
    msg_len = htonl(strlen(msg));
    send(socket, &msg_len, sizeof(uint32_t), 0);                /* Send the message length first */
    send(socket, msg, ntohl(msg_len), 0);                       /* Send the message content */
    //printf("Send msg len: !%u!\n", ntohl(msg_len));
    //printf("Send msg content: !%s!\n", msg);
}

/*
 * Function: Recv_socket_msg
 * -------------------
 *   This function is used to receive a message through socket with its length at first
 *
 *   Parameters:
 *      socket: the socket file descriptor
 *      msg: the message that need to receive
 *
 *   Returns:
 *      void
 */

void recv_socket_msg(int socket, char *msg) {
    recv(socket, &msg_len, sizeof(uint32_t), 0);                /* Recv the message length first */
    recv(socket, msg, ntohl(msg_len), 0);                       /* Recv the message content */
    msg[ntohl(msg_len)] = '\0';
    //printf("Recv msg len: !%u!\n", ntohl(msg_len));
    //printf("Recv msg content: !%s!\n", msg);
}

/*
 * Function: Write_pipe_msg
 * -------------------
 *   This function is used to write a message through pipe with its length write first
 *
 *   Parameters:
 *      pipe: the pipe file descriptor
 *      msg: the message that need to send
 *
 *   Returns:
 *      void
 */

void write_pipe_msg(int pipe, char *msg) {
    msg_len = htonl(strlen(msg));
    write(pipe, &msg_len, sizeof(uint32_t));                    /* Write the message length first */
    write(pipe, msg, ntohl(msg_len));                           /* Write the message content */
    //printf("Write msg len: !%u!\n", ntohl(msg_len));
    //printf("Write msg content: !%s!\n", msg);
}

/*
 * Function: Read_pipe_msg
 * -------------------
 *   This function is used to read a message through pipe with its length at first
 *
 *   Parameters:
 *      pipe: the pipe file descriptor
 *      msg: the message that need to receive
 *
 *   Returns:
 *      void
 */

int read_pipe_msg(int pipe, char *msg) {
    int ret;
    if ((ret = read(pipe, &msg_len, sizeof(uint32_t))) == 0) return 0;          /* Read the message length first */
    if ((ret = read(pipe, msg, ntohl(msg_len))) == 0) return 0;                 /* Read the message content */
    msg[ntohl(msg_len)] = '\0';
    //printf("Read msg len: !%u!\n", ntohl(msg_len));
    //printf("Read msg content: !%s!\n", msg);
    return ret;
}

/*
 * Function: Init
 * -------------------
 *   This function is used to init some variables,
 *   like malloc timestamp string, encrypt text string, etc.
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void init(void) {
    //signal(SIGINT, signal_handler);
    //signal(SIGQUIT, signal_handler);
    //signal(SIGHUP, signal_handler);
    addr_len = sizeof(struct sockaddr_in);
    out_time = (char *)malloc(sizeof(char) * TIME_MAXLENGTH);
    enc_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);
    dec_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);
}

/*
 * Function: Check_par
 * -------------------
 *   This function is used to check the parameters in command
 *
 *   Parameters:
 *      argc: the count of arguments
 *      argv: the command arguments
 *
 *   Returns:
 *      void
 */

void check_par(int argc, char *argv[]) {
    if (argc != 3) {                                        /* Check if the arguments number is right or not */
        get_time();
        printf("[%s] (Process ID #%d) Arguments number is not right! Usage: %s <ip address> <port number>\n", out_time, getpid(), argv[0]);
        clean_up(CLEAN_TO_TIME);
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: Get_port_number
 * -------------------
 *   This function is used to get the port number
 *
 *   Parameters:
 *      argv: the command arguments
 *
 *   Returns:
 *      void
 */

void get_port_number(char *argv[]) {
    if ((port_number = atoi(argv[2])) == 0) {               /* Get the server port number */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Can not use this port number for TCP connection!\n", out_time, getpid());
        clean_up(CLEAN_TO_TIME);
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: Create_socket
 * -------------------
 *   This function is used to create a socket in client side
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

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

/*
 * Function: Connect_socket
 * -------------------
 *   This function is used to connect to server side
 *
 *   Parameters:
 *      argv: the command arguments
 *
 *   Returns:
 *      void
 */

void connect_socket(char *argv[]) {
    memset(&serv_addr, 0, addr_len);                                            /* Initialize server address config */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_number);

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {                 /* Check IP address */
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

/*
 * Function: Print_connect_info
 * -------------------
 *   This function is used to print the info of connection
 *
 *   Parameters:
 *      argv: the command arguments
 *
 *   Returns:
 *      void
 */

void print_connect_info(char *argv[]) {
    get_time();
    printf("[%s] lyrebird client: PID %d connected to server %s on port %d.\n", out_time, getpid(), argv[1], port_number);
}

/*
 * Function: Send_connect_msg
 * -------------------
 *   This function is used to send the connection message to server
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void send_connect_msg(void) {
    strcpy(send_mark, CONNECT_MSG);                                 /* Send connect message to server */
    send_socket_msg(sockfd, send_mark);
}

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
        clean_up(CLEAN_TO_SOCKET);
        exit(EXIT_FAILURE);
    }

    child_to_parent = (int *)malloc(sizeof(int) * process_number_limit * 2);
    if (child_to_parent == NULL) {                                      /* Create child to parent pipes and check whether failed or not */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Malloc child_to_parent failed!\n", out_time, getpid());
        clean_up(CLEAN_TO_SOCKET);
        free(parent_to_child);
        exit(EXIT_FAILURE);
    }

    pid_array = (int *)malloc(sizeof(int) * process_number_limit);
    if (pid_array == NULL) {                                            /* Create child pid array and check whether failed or not */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Malloc pid_array failed!\n", out_time, getpid());
        clean_up(CLEAN_TO_SOCKET);
        free(parent_to_child);
        free(child_to_parent);
        exit(EXIT_FAILURE);
    }

    is_free = (int *)malloc(sizeof(int) * process_number_limit);
    if (is_free == NULL) {                                            /* Create child pid array and check whether failed or not */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Malloc is_free failed!\n", out_time, getpid());
        clean_up(CLEAN_TO_SOCKET);
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
            clean_up(CLEAN_TO_SOCKET);
            free(parent_to_child);
            free(child_to_parent);
            free(pid_array);
            free(is_free);
            exit(EXIT_FAILURE);                                                    /* If create pipe failed */
        }
        if (pipe(child_to_parent + i * 2)) {                            /* Create pipe from child to parent */
            get_time();
            printf("[%s] (Process ID #%d) ERROR: Create child to parent pipe number %d failed!\n", out_time, getpid(), i);
            clean_up(CLEAN_TO_SOCKET);
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
 * Function: Init_select_with_sockfd
 * -------------------
 *   This function is used to prepare select function for pipe in different processes (with socket)
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
    FD_SET(sockfd, &rfds);                                      /* Add file descriptor to set */
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
        while (read_pipe_msg(child_to_parent[i * 2], read_mark)) {
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
            }
        }
    }
}

/*
 * Function: Wait_all_child
 * -------------------
 *   This function is used to wait all child processes quit
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void wait_all_child(void) {
    int i;
    for (i = 0; i < process_number_limit; i++) {            /* Parent process wait for all child processes before exit */
        int state;
        pid_t pid = wait(&state);                           /* Wait until found one child process finished */
        close_ctp_pipe_with_pid(pid);                       /* Close the pipe that uses to read messages from exited child process */
    }
}

/*
 * Function: Check_client_exit_state
 * -------------------
 *   This function is used to check client's exit state
 *   and send it to server before exit
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void check_client_exit_state(void) {
    if (main_flag == EXIT_SUCCESS) {                        /* If client exit with state EXIT_SUCCESS */
        strcpy(send_mark, DISCONNECT_SUCC_MSG);
        send_socket_msg(sockfd, send_mark);
        get_time();
        printf("[%s] lyrebird client: PID %d completed its tasks and is exiting successfully.\n", out_time, getpid());
    } else {                                                /* If client exit with other state */
        strcpy(send_mark, DISCONNECT_FAIL_MSG);
        send_socket_msg(sockfd, send_mark);
        get_time();
        printf("[%s] lyrebird client: PID %d completed its tasks and is exiting successfully.\n", out_time, getpid());
    }
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

void clean_up(int step) {
    if (step >= CLEAN_TO_TIME){
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
