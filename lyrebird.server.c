/*------------------------------------------------------
 *
 *  lyrebird.server.c
 *
 *  this file contains the main function that used to build
 *  a server and then assign tasks to client sides, more detail
 *  functions are in the svr_func.c.
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

#include "lyrebird.server.h"
#include "memwatch.h"

int main_flag = EXIT_SUCCESS;           /* Used to store return value for main function */
char *enc_txt;                          /* Used to store encrypted file name */
char *dec_txt;                          /* Used to store decrypted file name */
char *out_time;                         /* Used to store output time */
FILE *fcfg, *flog;                      /* The file pointer that used to open config file and log file */

int max_fds;                            /* The max number of file descriptor */
fd_set rfds;                            /* The set of file descriptor */

char host[NI_MAXHOST];                  /* Used to store the return value of getnameinfo function */
struct ifaddrs *ifaddr, *ifa;           /* Used to store the return value of getifaddrs function */

int sockfd, sockfd_new;                         /* Socket file descriptor */
int sockfd_cli[CLIENT_MAXNUM];                  /* The array of client socket file descriptor */
char ipaddr_cli[CLIENT_MAXNUM][NI_MAXHOST];     /* The array of client IP address */
socklen_t addr_len;                             /* The size of sockaddr_in */
struct sockaddr_in serv_addr, cli_addr;         /* Used to store the server and client address information */

int read_flag = 1;                  /* This is a flag that means program should read a new line from config file */
int finish_flag = 0;                /* This is a flag that indicate reaching the end of config file */
char *client_ip;                    /* A pointer to the IP address char array of one client */
char read_type;                     /* This is the message that read from one socket to client side */

uint32_t msg_len;                               /* The length of sent message */
int remained_cli;                               /* The number of the remained clients */
char ip_buffer[INET_ADDRSTRLEN];                /* A buffer that used to store client IP address */
char send_mark[MARK_MAXLENGTH];                 /* A buffer that used to store sent message */
char recv_mark[MARK_MAXLENGTH];                 /* A buffer that used to store received message */
char read_mark[MARK_MAXLENGTH];                 /* A buffer that used to store read message */
char write_mark[MARK_MAXLENGTH];                /* A buffer that used to store write message */

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

    init();                                 /* Init some variables (like malloc timestamp string, encrypt text string, etc.) */

    check_par(argc, argv);                  /* Check command arguments number */
    open_config(argv);                      /* Open config file and check if it failed */
    open_log(argv);                         /* Open log file and check if it failed */

    get_ipaddr();                           /* Get server IP address */

    create_socket();                        /* Create a socket */
    bind_socket();                          /* Bind the socket */
    listen_socket();                        /* Listen at the socket */

    print_server_info();                    /* Print server information */

    while (TRUE) {                          /* Read until the end of file */
        if (read_flag) {
            if (fscanf(fcfg, "%s", enc_txt) == EOF) {
                finish_flag = 1;
                break;
            } else {
                fscanf(fcfg, "%s", dec_txt);
            }
        }
        read_flag = 0;

        init_select();                      /* Select function */
        if (select_func() == -1) break;

        for (i = 0; i < max_fds + 1; i++) {
            if (FD_ISSET(i, &rfds)) {
                if (i == sockfd) {                              /* If have a new client connect */
                    if (accept_new_cli() == -1) break;          /* Try to accept new client */
                    if (check_connect() == -1) break;           /* Check connect message from client */
                    if (print_client_info() == -1) break;       /* Print the information of client side */
                    store_client_ip();                          /* Store the client ip address */
                    break;
                } else {                                        /* If have new message from client side */
                    client_ip = get_host_by_sockfd(i);          /* Get the client ip address by socket */
                    recv_socket_msg(i, recv_mark);              /* Get the message from socket */
                    handle_client_msg(i);                       /* Handle client message (SUCCESS_MSG, FAILURE_MSG, DISPATCH_MSG, etc.) */
                    break;
                }
            }
            if (main_flag == EXIT_FAILURE) break;
        }
        if (main_flag == EXIT_FAILURE) break;
    }

    remained_cli = ask_clients_quit();                          /* Ask clients quit and count the remain clients number */
    printf("Total Remained Client: %d\n", remained_cli);
    wait_clients_quit();                                        /* Wait for all clients quit */
    quit_server();                                              /* Clean up and quit server, also print the message to log */

    return main_flag;
}
