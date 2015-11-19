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

#define FILE_MAXLENGTH 1030

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

/* Get time format */
extern void get_time(void);

/* First come first serve scheduling algorithm */
void fcfs(void);

#endif
