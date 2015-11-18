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
#include <unistd.h>

/* Used to get current time in specify format */
void get_time(void);

/* Used to free memory and close file pointer before program exit */
void clean_up(void);

#endif
