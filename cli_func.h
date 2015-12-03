/*------------------------------------------------------
 *
 *  cli_func.h
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

#ifndef LYREBIRD_CLI_FUNC_H
#define LYREBIRD_CLI_FUNC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "lyre_macro.h"

extern int main_flag;                           /* Used to store return value for main function */
extern char *enc_txt;                           /* Used to store encrypted file name */
extern char *dec_txt;                           /* Used to store decrypted file name */
extern char *out_time;                          /* Used to store output time */

extern fd_set rfds;                             /* The set of file descriptor */
extern int max_descriptor;                      /* The max number of file descriptor */

extern pid_t pid;                               /* Used to store fork pid */
extern int *pid_array;                          /* Used to store all the child pid (as int) */
extern int *is_free;                            /* Used to show if a child process with a pid is free now */
extern int process_number_limit;                /* The max number of processes that can use now */
extern int process_number_now;                  /* The number of processes that already been used now */

extern int *parent_to_child;                    /* Pipe that used to transmit message from parent process to child process */
extern int *child_to_parent;                    /* Pipe that used to transmit message from child process to parent process */

extern int port_number;                             /* The server port number */
extern char host[NI_MAXHOST];                       /* Used to store the return value of getnameinfo function */
extern struct ifaddrs *ifaddr, *ifa;                /* Used to store the return value of getifaddrs function */

extern int sockfd;                                      /* Socket file descriptor */
extern socklen_t addr_len;                              /* The size of sockaddr_in */
extern struct sockaddr_in serv_addr, cli_addr;          /* Used to store the server and client address information */

extern uint32_t msg_len;                                /* The length of sent message */
extern char send_mark[MARK_MAXLENGTH];                  /* A buffer that used to store sent message */
extern char recv_mark[MARK_MAXLENGTH];                  /* A buffer that used to store received message */
extern char read_mark[MARK_MAXLENGTH];                  /* A buffer that used to store read message */
extern char write_mark[MARK_MAXLENGTH];                 /* A buffer that used to store write message */


/* Used to get current time in specify format */
void get_time(void);

/* Used to send a message through socket with its length send first */
void send_socket_msg(int socket, char *msg);

/* Used to receive a message through socket with its length at first */
void recv_socket_msg(int socket, char *msg);

/* Used to write a message through pipe with its length at first */
void write_pipe_msg(int pipe, char *msg);

/* Used to read a message through pipe with its length send first */
int read_pipe_msg(int pipe, char *msg);

/* Init some varibles (like malloc timestamp string, encrypt text string, etc.) */
void init(void);

/* Used to check the parameters in command */
void check_par(int argc, char *argv[]);

/* Used to get the port number */
void get_port_number(char *argv[]);

/* Used to create a socket in client side */
void create_socket(void);

/* Used to connect to server side */
void connect_socket(char *argv[]);

/* Used to print the info of connection */
void print_connect_info(char *argv[]);

/* Used to send the connection message to server */
void send_connect_msg(void);

/* Used to initialize the max number of pipe descriptor */
void init_pipe(void);

/* Used to prepare select function for pipe in different procesors  */
void init_select(void);

/* Used to prepare select function for pipe in different procesors  */
void init_select_with_socket(int sockfd);

/* Used to close the pipes from parent to child except one process's pipe */
void close_ptc_pipes_except(int p);

/* Used to close the pipes from child to parent except one process's pipe */
void close_ctp_pipes_except(int p);

/* Used to close all the pipes from child to parent */
void close_all_ptc_pipes(void);

/* Used to close pipe that belong to child process with specific pid */
void close_ctp_pipe_with_pid(int pid);

/* Used to read all remaining messages in each child process */
void read_rmng_msg(void);

/* Used to wait all child processes quit */
void wait_all_child(void);

/* Check client exit state and send it to server before exit */
void check_client_exit_state(void);

/* Used to free memory and close file pointer before program exit */
void clean_up(int step);

/* Used to handle signal */
void signal_handler(int sig_num);

#endif

