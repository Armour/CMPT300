/*------------------------------------------------------
 *
 *  decrypt.h
 *
 *  Project         : LyreBird
 *  Name            : Guo Chong
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

/* Decrypt each tweet through 4 steps */
char *decrypt_each(char *tweet);

/* Decrypt all tweets in config file */
int decrypt(char *input, char *output);

#endif
