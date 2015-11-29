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
int process_number_limit;       /* The max number of processes that can use now */
int process_number_now = -1;    /* The number of processes that already been used now */

int *parent_to_child;           /* Pipe that used to transmit message from parent process to child process */
int *child_to_parent;           /* Pipe that used to transmit message from child process to parent process */

fd_set rfds;                    /* The set of file descriptor */
int max_descriptor;             /* The max number of file descriptor */

int port_number;
struct hostent *server;

int sockfd;
socklen_t addr_len;
struct sockaddr_in serv_addr, cli_addr;

struct ifaddrs *ifaddr, *ifa;   /* Used to store the return value of getifaddrs function */
char host[NI_MAXHOST];          /* Used to store the return value of getnameinfo function */

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
    if (step >= 1) free(out_time);
    if (step >= 2) freeifaddrs(ifaddr);
    if (step >= 3) close(sockfd);
    if (step >= 4) {
        free(enc_txt);
        free(dec_txt);
        free(parent_to_child);
        free(child_to_parent);
        free(pid_array);
    }
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
    int i;
    int opt = TRUE;                 /* Used when set socket options */
    int mark;

    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGHUP, signal_handler);

    out_time = (char *)malloc(sizeof(char) * TIME_MAXLENGTH);

    if (argc != 3) {                    /* Check if the arguments number is right or not */
        get_time();
        printf("[%s] (Process ID #%d) Arguments number is not right! Usage: %s <ip address> <port number>\n", out_time, getpid(), argv[0]);
        clean_up(1);
        exit(EXIT_FAILURE);
    }

    if ((port_number = atoi(argv[2])) == 0) {               /* Get the server port number */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Can not use this port number for TCP connection!\n", out_time, getpid());
        clean_up(1);
        exit(EXIT_FAILURE);
    }

    if (getifaddrs(&ifaddr) == -1) {        /* Get the ip address of this machine and check if it failed */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Can not get the interface address of this machine!\n", out_time, getpid());
        clean_up(1);
        exit(EXIT_FAILURE);
    }

    addr_len = sizeof(struct sockaddr_in);

    for (ifa = ifaddr, i = 0; ifa != NULL; ifa = ifa->ifa_next, i++) {          /* Iterate each interface addresses and get the IP address */
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family != AF_INET)                                                  /* If it is not the IPv4 interface */
            continue;

        if (getnameinfo(ifa->ifa_addr, addr_len, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0) {         /* If can not get the address information of interface address */
            get_time();
            printf("[%s] (Process ID #%d) ERROR: Can not get the address information of interface address!\n", out_time, getpid());
            clean_up(2);
            exit(EXIT_FAILURE);
        }

        if (strcmp(host, "0.0.0.0") == 0 || strcmp(host, "127.0.0.1") == 0)     /* Filter the special IP address "0.0.0.0" and "127.0.0.1" */
            continue;

        break;                                                                  /* If found a avaliable Ip address, then we break */
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {                       /* Create socket to connect server */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Create socket failed!\n", out_time, getpid());
        clean_up(2);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {      /* Set socket option to reuseable address (not necessary but good) */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Can not set socket option!\n", out_time, getpid());
        clean_up(3);
        exit(EXIT_FAILURE);
    }

    memset(&cli_addr, 0, addr_len);                                            /* Initialize client address config */
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_port = htons(0);
    if (inet_pton(AF_INET, host, &cli_addr.sin_addr) <= 0) {
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Client host in inet_pton function is not a valid IP address!\n", out_time, getpid());
        clean_up(3);
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, addr_len);                                            /* Initialize server address config */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_number);
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Server host in inet_pton function is not a valid IP address!\n", out_time, getpid());
        clean_up(3);
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)&cli_addr, addr_len) < 0) {            /* Bind the client socket */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Bind socket failed!\n", out_time, getpid());
        clean_up(3);
        exit(EXIT_FAILURE);
    }

    //fcntl(sockfd, F_SETFL, O_NONBLOCK);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, addr_len) < 0) {
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Connect to server %s:%d failed!\n", out_time, getpid(), argv[1], port_number);
        clean_up(3);
        exit(EXIT_FAILURE);
    }

    //fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) & ~O_NONBLOCK);

    init_pipe();                                            /* Initialize pipe */

    get_time();
    printf("[%s] lyrebird client: PID %d connected to server %s on port %d.\n", out_time, getpid(), argv[1], port_number);
    mark = htonl(CONNECT_MSG);
    write(sockfd, &mark, sizeof(int));
    for (i = 0; i < process_number_limit; ++i) {
        mark = htonl(DISPATCH_MSG);
        write(sockfd, &mark, sizeof(int));
    }

    enc_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);
    dec_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);

    while (read(sockfd, &mark, sizeof(mark))) {                  /* Always waiting for message from server side */

        if (ntohl(mark) == 0) break;
        read(sockfd, enc_txt, sizeof(char) * FILE_MAXLENGTH);
        read(sockfd, dec_txt, sizeof(char) * FILE_MAXLENGTH);

        if (process_number_now + 1 < process_number_limit) {            /* If not exceed the max number of process */
            process_number_now++;
            pid = fork();                                   /* Fork! */
            if (pid < 0) {                                  /* If fork failed */
                get_time();
                printf("[%s] (Process ID #%d) ERROR: Fork failed! Client will exit now without finish its tasks!\n", out_time, getpid());
                main_flag = EXIT_FAILURE;                              /* Exit non-zero value */
                break;                                      /* Break and still need to wait for all child processes */
            }
        } else
            process_number_now = process_number_limit;

        if (pid != 0) {                                     /* If fork successful and is in parent process */
            if (process_number_now < process_number_limit) {            /* First round we just assgin each task to each child process */
                pid_array[process_number_now] = (int)pid;               /* Store child pid into array */
                close(parent_to_child[process_number_now * 2]);
                close(child_to_parent[process_number_now * 2 + 1]);     /* Close pipes that will not be used */
                get_time();
                printf("[%s] Child process ID #%d will decrypt %s.\n", out_time, *(pid_array + process_number_now), enc_txt);
                write(parent_to_child[process_number_now * 2 + 1], enc_txt, sizeof(char) * FILE_MAXLENGTH);
                write(parent_to_child[process_number_now * 2 + 1], dec_txt, sizeof(char) * FILE_MAXLENGTH);
            } else {
                fcfs();
                if (main_flag != EXIT_SUCCESS) break;
            }
        }

        if (pid == 0) {                                             /* If in child process */
            int state = 0;
            char enc_txt[FILE_MAXLENGTH];
            char dec_txt[FILE_MAXLENGTH];

            close_ptc_pipes_except(process_number_now);             /* Close other pipes except used ptc and ctp */
            close_ctp_pipes_except(process_number_now);
            close(parent_to_child[process_number_now * 2 + 1]);
            close(child_to_parent[process_number_now * 2]);

            while (1) {                                             /* Keep decrpting until break */
                if (read(parent_to_child[process_number_now * 2], &enc_txt, sizeof(char) * FILE_MAXLENGTH) == 0) break;    /* Break if parent process's pipe closed */
                read(parent_to_child[process_number_now * 2], &dec_txt, sizeof(char) * FILE_MAXLENGTH);
                state = decrypt(enc_txt, dec_txt);
                if (state == MALLOC_FAIL_ERROR) break;                         /* If child decryption meet an fatal error */
                if (state == EXIT_SUCCESS) {                                   /* If child process is ready to decrypt another file */
                    mark = htonl(SUCCESS_MSG);
                    write(sockfd, &mark, sizeof(int));
                    write(sockfd, enc_txt, sizeof(char) * FILE_MAXLENGTH);
                    mark = htonl(getpid());
                    write(sockfd, &mark, sizeof(int));
                }
                write(child_to_parent[process_number_now * 2 + 1], &process_number_now, sizeof(int));
            }

            clean_up(4);                                                /* Always remember to free all and close file pointer! */
            close(parent_to_child[process_number_now * 2]);             /* Close used pipes */
            close(child_to_parent[process_number_now * 2 + 1]);
            exit(state);
        }
    }

    close_all_ptc_pipes();                                  /* Close all parent to child pipes */
    read_rmng_msg();                                        /* Read remaining messages in all child processes */

    for (i = 0; i < process_number_limit; i++) {            /* Parent process wait for all child processes before exit */
        int state;
        pid_t pid = wait(&state);                           /* Wait until found one child process finished */
        printf("%d\n", state);
        if (state != EXIT_SUCCESS) {                                   /* If child process terminate unexpectly! */
            get_time();
            printf("[%s] Child process ID #%d did not terminate successfully.\n", out_time, (int)pid);
        }
        close_ctp_pipe_with_pid(pid);                       /* Close the pipe that uses to read messages from exited child process */
    }

    if (main_flag == EXIT_SUCCESS) {
        mark = htonl(DISCONNECT_SUCC_MSG);
        write(sockfd, &mark, sizeof(int));
        get_time();
        printf("[%s] lyrebird client: PID %d completed its tasks and is exiting successfully.\n", out_time, getpid());
    } else {
        mark = htonl(DISCONNECT_FAIL_MSG);
        write(sockfd, &mark, sizeof(int));
    }

    clean_up(4);                                             /* Always remember to free all and close file pointer! */
    return main_flag;
}
