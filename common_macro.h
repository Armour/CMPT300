/*------------------------------------------------------
 *
 *  common_macro.h
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

#ifndef LYREBIRD_COMMON_MACRO_H
#define LYREBIRD_COMMON_MACRO_H

#define TRUE 1
#define FALSE 0

#define FILE_MAXLENGTH 1030
#define ERROR_MAXLENGTH (FILE_MAXLENGTH + 100)
#define PID_MAXLENGTH 10
#define MARK_MAXLENGTH 4

#define CONNECT_MSG "1"
#define DISPATCH_MSG "2"
#define SUCCESS_MSG "3"
#define FAILURE_MSG "4"
#define DISCONNECT_SUCC_MSG "5"
#define DISCONNECT_FAIL_MSG "6"

#define CLIENT_EXIT_MSG "0"
#define CLIENT_WORK_MSG "1"

#define CHILD_PROCESS_INIT "0"
#define CHILD_PROCESS_READY "1"
#define CHILD_PROCESS_SUCCESS "2"
#define CHILD_PROCESS_WARNING "3"
#define CHILD_PROCESS_FAILURE "4"

#define FCFS_CONT 0
#define FCFS_EXIT 1

#define PROCESS_MAX_NUMBER sysconf(_SC_NPROCESSORS_ONLN) - 1

#define TIME_MAXLENGTH 50
#define OPEN_FILE_ERROR 1
#define MALLOC_FAIL_ERROR 2

#endif


