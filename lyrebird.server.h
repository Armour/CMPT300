/*------------------------------------------------------
 *
 *  lyrebird.server.h
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

#ifndef LYREBIRD_LYREBIRD_SERVER_H
#define LYREBIRD_LYREBIRD_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>

#define TRUE 1
#define FALSE 0
#define CLIENT_MAXNUM 32

/* Used to get current time in specify format */
void get_time(void);

/* Used to get host ip address through socket file descriptor */
char *get_host_by_sockfd(int sockfd);

/* Used to free memory and close file pointer before program exit */
void clean_up(int step);

#endif
