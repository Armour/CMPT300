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
FILE *fuck;

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
int max_task = 0;                   /* This is the number of the decryption tasks that has been assigned */
int cnt_task = 0;                   /* This is the number of the decryption tasks that has already finished */
char read_type;                     /* This is the message that read from one socket to client side */

uint32_t msg_len;                               /* The length of sent message */
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

    out_time = (char *)malloc(sizeof(char) * TIME_MAXLENGTH);
    enc_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);
    dec_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);

    check_par(argc, argv);
    open_config(argv);
    open_log(argv);

    get_ipaddr();

    create_socket();
    bind_socket();
    listen_socket();

    print_server_info();

    init_cli_sock();

    while (TRUE) {                /* Read until end of file */
        if (read_flag) {
            if (fscanf(fcfg, "%s", enc_txt) == EOF) {
                finish_flag = 1;
            } else {
                fscanf(fcfg, "%s", dec_txt);
            }
        }
        read_flag = 0;

        if (finish_flag && cnt_task == max_task) break;

        init_select();
        if (select_func() == -1) break;

        for (i = 0; i < max_fds + 1; i++) {
            if (FD_ISSET(i, &rfds)) {
                if (i == sockfd) {                              /* If have a new client connect */
                    printf("New client!!\n");
                    if (accept_new_cli() == -1) break;
                    if (check_connect() == -1) break;
                    if (print_client_info() == -1) break;
                    store_client_ip();
                    break;
                } else {
                    client_ip = get_host_by_sockfd(i);
                    recv_socket_msg(i, recv_mark);
                    handle_client_msg(i);
                    break;
                }
            }
            if (main_flag == EXIT_FAILURE) break;
        }
        if (main_flag == EXIT_FAILURE) break;
    }

    int remained_cli = ask_clients_quit();
    printf("Total Remained Client: %d\n", remained_cli);
    wait_clients_quit(remained_cli);
    quit_server();

    return main_flag;
}
