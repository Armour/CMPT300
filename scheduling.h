/*------------------------------------------------------
 *
 *  scheduing.h
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

#ifndef LYREBIRD_SCHEDULING_H
#define LYREBIRD_SCHEDULING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define TRUE 1
#define FALSE 0

#define FILE_MAXLENGTH 1030
#define ERROR_MAXLENGTH (FILE_MAXLENGTH + 100)
#define PID_MAXLENGTH 10

#define CONNECT_MSG '1'
#define DISPATCH_MSG '2'
#define SUCCESS_MSG '3'
#define FAILURE_MSG '4'
#define DISCONNECT_SUCC_MSG '5'
#define DISCONNECT_FAIL_MSG '6'

#define CLIENT_EXIT_MSG '0'
#define CLIENT_WORK_MSG '1'

#define CHILD_PROCESS_INIT '0'
#define CHILD_PROCESS_READY '1'
#define CHILD_PROCESS_SUCCESS '2'
#define CHILD_PROCESS_WARNING '3'
#define CHILD_PROCESS_FAILURE '4'

#define FCFS_CONT '0'
#define FCFS_EXIT '1'

extern char *enc_txt;                   /* Used to store encrypted file name */
extern char *dec_txt;                   /* Used to store decrypted file name */
extern char *out_time;                  /* Used to store output time */
extern int main_flag;                   /* Used to store return value for main function */

extern int *pid_array;                  /* Used to store all the child pid (as int) */
extern int process_number_limit;        /* The max number of processes that can use now */
extern int process_number_now;          /* The number of processes that already been used now */

extern int *parent_to_child;            /* Pipe that used to transmit message from parent process to child process */
extern int *child_to_parent;            /* Pipe that used to transmit message from child process to parent process */

extern fd_set rfds;                     /* The set of file descriptor */
extern int max_descriptor;              /* The max number of file descriptor */

extern FILE *fuck;
extern int sockfd;

/* Get time format */
extern void get_time(void);

/* First come first serve scheduling algorithm */
int fcfs(void);

#endif
