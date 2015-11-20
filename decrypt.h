/*------------------------------------------------------
 *
 *  decrypt.h
 *
 *  Project         : LyreBird
 *  Name            : Chong Guo
 *  Student ID      : 301295753
 *  SFU username    : armourg
 *  Lecture section : D1
 *  Instructor      : Brain G.Booth
 *  TA              : Scott Kristjanson
 *
 *  Created by Armour on 14/09/2015
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *------------------------------------------------------
 */

#ifndef LYREBIRD_DECRYPT_H
#define LYREBIRD_DECRYPT_H

#include <stdio.h>
#include <unistd.h>
#include <netdb.h>

#define TIME_MAXLENGTH 50
#define FATAL_ERROR 1
#define OPEN_FILE_ERROR 2
#define MALLOC_FAIL_ERROR 3

extern int sockfd;
extern char host[NI_MAXHOST];          /* Used to store the return value of getnameinfo function */

/* Decrypt each tweet through 4 steps */
char *decrypt_each(char *tweet);

/* Decrypt all tweets in config file */
int decrypt(char *input, char *output);

#endif
