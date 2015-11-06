/*------------------------------------------------------
 *
 *  pipe.h
 *
 *  Project         : LyreBird
 *  Name            : Guo Chong
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

#ifndef LYREBIRD_PIPE_H
#define LYREBIRD_PIPE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROCESSOR_MAX_NUMBER sysconf(_SC_NPROCESSORS_ONLN) - 1

extern char *out_time;                 /* Used to store output time */

extern pid_t pid;                      /* Used to store fork pid */
extern int *pid_array;                 /* Used to store all the child pid (as int) */
extern int processor_number_limit;     /* The max number of processors that can use now */
extern int processor_number_now;       /* The number of processors that already been used now */

extern int *parent_to_child;           /* Pipe that used to transmit messagee from parent processor to child processor */
extern int *child_to_parent;           /* Pipe that used to transmit messagee from child processor to parent processor */

extern fd_set rfds;                    /* The set of file descriptor */
extern int max_descriptor;             /* The max number of file descriptor */

/* Get time format */
extern void get_time(void);

/* Used to initialize the max number of pipe descriptor */
void init_pipe(void);

/* Used to prepare select function for pipe in different procesors  */
void init_select(void);

/* Used to close the pipes from parent to child except one processor's pipe */
void close_ptc_pipes_except(int p);

/* Used to close the pipes from child to parent except one processor's pipe */
void close_ctp_pipes_except(int p);

/* Used to close all the pipes from child to parent */
void close_all_ptc_pipes(void);

/* Used to close pipe that belong to child processor with specific pid */
void close_ctp_pipe_with_pid(int pid);

/* Used to read all remaining messages in each child processor */
void read_rmng_msg(void);

#endif