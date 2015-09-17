/*------------------------------------------------------
 *
 *  decrypt.h
 *
 *  Project         : LyreBird
 *  Name            : guo chong
 *  Student ID      : 301295753
 *  SFU username    : armourg
 *  Lecture section : D1
 *  Instructor      : Brain G.Booth
 *  TA              : Scott Kristjanson
 *
 *  Created by Armour on 16/09/2015
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *------------------------------------------------------
 */

#ifndef LYREBIRD_DECRYPT_H
#define LYREBIRD_DECRYPT_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define POW_EXPONENT 41
#define REGULAR_INTERVAL 8
#define CONSTANT_MULTIPLE 6
#define MOD_MODULUS 4294434817
#define MOD_EXPONENT 1921821779

/*  Translate each of the group of 6 characters into cipher number */
unsigned long long base41_ctoi(char *tweet, int pos);

/* Translate plain-text number into new group of 6 characters */
void base41_itoc(unsigned long long number, char *group_char);

/* Remove unnecessary characters in regular interval */
void rm_extra_char(char *input, char *output, int *len);

/* Map each cipher number onto a similar plain-text number  */
unsigned long long mapping_exp(unsigned long long number);

#endif
