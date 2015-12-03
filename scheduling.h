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
#include "lyre_macro.h"
#include "cli_func.h"

extern char fcfs_file_buf[FILE_MAXLENGTH];             /* The buffer that used to store file name that read from pipe */
extern char fcfs_err_buf[ERROR_MAXLENGTH];             /* The buffer that used to store error message that read from pipe */
extern char fcfs_pid_buf[PID_MAXLENGTH];               /* The buffer that used to store pid number that read from pipe */

/* First come first serve scheduling algorithm */
int fcfs(void);

#endif
