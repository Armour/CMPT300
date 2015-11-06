/*------------------------------------------------------
 *
 *  lyrebird.h
 *
 *  Project         : LyreBird
 *  Name            : Guo Chong
 *  Student ID      : 301295753
 *  SFU username    : armourg
 *  Lecture section : D1
 *  Instructor      : Brain G.Booth
 *  TA              : Scott Kristjanson
 *
 *  Created by Armour on 03/10/2015
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *------------------------------------------------------
 */

#ifndef LYREBIRD_LYREBIRD_H
#define LYREBIRD_LYREBIRD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FILE_MAXLENGTH 1030
#define PROCESSOR_MAX_NUMBER sysconf(_SC_NPROCESSORS_ONLN) - 1

/* Used to get current time in specify format */
void get_time(void);

/* Used to initialize the max number of pipe descriptor */
void init_pipe(void);

/* Used to close the pipes from parent to child */
void close_ptc_pipe(int p);

/* Used to close the pipes from child to parent */
void close_ctp_pipe(int p);

/* Used to prepare select function for pipe in different procesors  */
void init_select(void);

/* Used to trim leading and trailing spaces in a string  */
char *trim_space(char *str);

/* Used to get scheduling algorithm name */
void get_schedule(void);

/* Used to free memory and close file pointer before program exit */
void clean_up(void);

#endif
