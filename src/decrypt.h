/*------------------------------------------------------
 *
 *  decrypt.h
 *  LyreBird
 *
 *  Created by Armour on 16/09/2015
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *------------------------------------------------------
 */

#ifndef LYREBIRD_DECRYPT_H
#define LYREBIRD_DECRYPT_H

#include <stdio.h>
#include <math.h>

#define POW_EXPONENT 41
#define CONSTANT_MULTIPLE 6

/* Table used to transform from integer to character */
char table_itoc[41] = {' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                       'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
                       'u', 'v', 'w', 'x', 'y', 'z', '#', '.', ',', '\'',
                       '!', '?', '(', ')', '-', ':', '$', '/', '&', '\\'};

/* Table used to transform from character to integer */
int table_ctoi[128] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, -1,  0, 31, -1, 27, 37, -1, 39, 30,
                        33, 34, -1, -1, 29, 35, 28, 38, -1, -1,
                        -1, -1, -1, -1, -1, -1, -1, -1, 36, -1,
                        -1, -1, -1, 32, -1, -1, -1, -1, -1, -1,
                        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, -1, 40, -1, -1, -1, -1,  1,  2,  3,
                         4,  5,  6,  7,  8,  9, 10, 11, 12, 13,
                        14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                        24, 25, 26, -1, -1, -1, -1, -1};

/*  Translate each of the group of 6 characters into cipher number */
unsigned long long base41_ctoi(char *tweet, int pos);

/* Translate plain-text number into new group of 6 characters */
char *base41_itoc(unsigned long long number);

#endif
