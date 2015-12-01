/*------------------------------------------------------
 *
 *  lyrebird.client.h
 *
 *  Project         : LyreBird
 *  Name            : Chong Guo
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
#include <signal.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define TRUE 1
#define FALSE 0

#define CLEAN_TO_TIME 1
#define CLEAN_TO_IFADDR 2
#define CLEAN_TO_SOCKET 3
#define CLEAN_ALL 4

/* Used to get current time in specify format */
void get_time(void);

/* Used to free memory and close file pointer before program exit */
void clean_up(int step);

#endif
