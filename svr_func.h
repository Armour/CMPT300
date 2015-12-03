/*------------------------------------------------------
 *
 *  svr_func.h
 *
 *  Project         : LyreBird
 *  Name            : Chong Guo
 *  Student ID      : 301295753
 *  SFU username    : armourg
 *  Lecture section : D1
 *  Instructor      : Brain G.Booth
 *  TA              : Scott Kristjanson
 *
 *  Created by Armour on 01/12/2015
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *------------------------------------------------------
 */

#ifndef LYREBIRD_SVR_FUNC_H
#define LYREBIRD_SVR_FUNC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "svr_macro.h"
#include "common_macro.h"

extern int main_flag;                           /* Used to store return value for main function */
extern char *enc_txt;                           /* Used to store encrypted file name */
extern char *dec_txt;                           /* Used to store decrypted file name */
extern char *out_time;                          /* Used to store output time */

extern FILE *fcfg, *flog;                       /* The file pointer that used to open config file and log file */
extern FILE *fuck;

extern fd_set rfds;                             /* The set of file descriptor */
extern int max_fds;                             /* The max number of file descriptor */

extern char host[NI_MAXHOST];                   /* Used to store the return value of getnameinfo function */
extern struct ifaddrs *ifaddr, *ifa;            /* Used to store the return value of getifaddrs function */

extern int sockfd, sockfd_new;                         /* Socket file descriptor */
extern int sockfd_cli[CLIENT_MAXNUM];                  /* The array of client socket file descriptor */
extern char ipaddr_cli[CLIENT_MAXNUM][NI_MAXHOST];     /* The array of client IP address */
extern socklen_t addr_len;                             /* The size of sockaddr_in */
extern struct sockaddr_in serv_addr, cli_addr;         /* Used to store the server and client address information */

extern int read_flag;                       /* This is a flag that means program should read a new line from config file */
extern int finish_flag;                     /* This is a flag that indicate reaching the end of config file */
extern char *client_ip;                     /* A pointer to the IP address char array of one client */
extern int max_task;                        /* This is the number of the decryption tasks that has been assigned */
extern int cnt_task;                        /* This is the number of the decryption tasks that has already finished */
extern char read_type;                      /* This is the message that read from one socket to client side */

extern uint32_t msg_len;                               /* The length of sent message */
extern char ip_buffer[INET_ADDRSTRLEN];                /* A buffer that used to store client IP address */
extern char send_mark[MARK_MAXLENGTH];                 /* A buffer that used to store sent message */
extern char recv_mark[MARK_MAXLENGTH];                 /* A buffer that used to store received message */
extern char read_mark[MARK_MAXLENGTH];                 /* A buffer that used to store read message */
extern char write_mark[MARK_MAXLENGTH];                /* A buffer that used to store write message */


/* Used to send a message through socket with its length send first */
void send_socket_msg(int socket, char *msg);

/* Used to receive a message through socket with its length at first */
void recv_socket_msg(int socket, char *msg);

/* Used to check the parameters in command */
void check_par(int argc, char *argv[]);

/* Used to open config file and check if it failed */
void open_config(char *argv[]);

/* Used to open log file and check if it failed */
void open_log(char *argv[]);

/* Used to get current time in specify format */
void get_time(void);

/* Used to get host ip address through socket file descriptor */
char *get_host_by_sockfd(int sockfd);

/* Used to get the ip address of server machine */
void get_ipaddr(void);

/* Used to create a socket in server side */
void create_socket(void);

/* Used to bind a socket in serve side */
void bind_socket(void);

/* Used to listen a socket in server side */
void listen_socket(void);

/* Used to find out and print the info of server side */
void print_server_info(void);

/* Used to init socket array for client side */
void init_cli_sock(void);

/* Used to init FD before each time's select */
void init_select(void);

/* Used as a wrapper for select function */
int select_func(void);

/* Used to accept new client */
int accept_new_cli(void);

/* Used to check if a client connect successfully with a message */
int check_connect(void);

/* Used to find out and print the info of client side */
int print_client_info(void);

/* Used to store the client ip address into sockfd_cli array */
void store_client_ip(void);

/* Uead to handle success message */
void handle_success(int sock_num);

/* Uead to handle dispatch message */
void handle_dispatch(int sock_num);

/* Uead to handle failure message */
void handle_failure(int sock_num);

/* Used to handle different kinds of client message */
void handle_client_msg(int sock_num);

/* Used to ask all clients quit */
int ask_clients_quit(void);

/* Used to wait all clients quit */
void wait_clients_quit(int remained_cli);

/* Used to clean up everything and quit the server */
void quit_server(void);

/* Used to free memory and close file pointer before program exit */
void clean_up(int step);

#endif

