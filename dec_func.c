/*------------------------------------------------------
 *
 *  dec_func.c
 *
 *  this file contains many functions that used to decrypt
 *  the tweet, like remove extra characters, mapping characters
 *  to integer value and mapping integer to characters, etc.
 *
 *  Project         : LyreBird
 *  Name            : Guo Chong
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

#include "dec_func.h"
#include "memwatch.h"

/* Table used to transform from integer to character */
char table_itoc[41] = {' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                       'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
                       'u', 'v', 'w', 'x', 'y', 'z', '#', '.', ',', '\'',
                       '!', '?', '(', ')', '-', ':', '$', '/', '&', '\\'};

/* Table used to transform from character to integer, -1 for character which can't be used */
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

/*
 * Function: base41_ctoi
 * -------------------
 *   Translate each of the group of 6 characters into cipher number
 *
 *   Parameters:
 *     tweet: the tweet which contained the group of 6 characters
 *     pos: the position of group in tweet
 *
 *   Returns:
 *      cipher number that generated by a group of 6 characters
 */

unsigned long long base41_ctoi(char *tweet, int pos) {
    unsigned long long ans = 0;             /* Cipher number of a group of characters */
    int i;
    for (i = 0; i < CONSTANT_MULTIPLE; ++i) {
        ans += table_ctoi[(int)tweet[pos + i]] * pow(POW_EXPONENT, CONSTANT_MULTIPLE - i - 1);
    }
    return ans;
}

/*
 * Function: base41_itoc
 * -------------------
 *   Translate plain-text number into new group of 6 characters
 *
 *   Parameters:
 *     number: the plain text which need to translate
 *     group_char: a group of characters which need to translate
 *
 *   Returns:
 *      void
 */

void base41_itoc(unsigned long long number, char *group_char) {
    int i;
    for (i = 0; i < CONSTANT_MULTIPLE; ++i) {
        group_char[i] = table_itoc[(int)(number / pow(POW_EXPONENT, CONSTANT_MULTIPLE - i - 1))];
        number = number % (int)pow(POW_EXPONENT, CONSTANT_MULTIPLE - i - 1);
    }
}

/*
 * Function:  rm_extra_char
 * -------------------
 *   Remove unnecessary characters in regular interval
 *
 *   Parameters:
 *      input: input characters array
 *      output: output the array that some characters has been removed
 *      len: length of the output characters array
 *
 *   Returns:
 *      void
 */

void rm_extra_char(char *input, char *output, int *len){
    int i;
    int count = 0;              /* Count for regular interval */
    int tmp_len = 0;

    for (i = 0; i < *len; ++i) {
        count++;
        if (count == REGULAR_INTERVAL) {        /* Remove regulare interval */
            count = 0;
            continue;
        }
        output[tmp_len++] = input[i];
    }

    *len = tmp_len;             /* Update length after remove some characters */
}

/*
 * Function:  mapping_exp
 * -------------------
 *   Map each cipher number onto a similar plain-text number
 *
 *   Parameters:
 *      number: the cipher number that need to mapped
 *
 *   Returns:
 *      plain-text number that you get from mapping
 */

unsigned long long mapping_exp(unsigned long long number) {
    unsigned long long result = 1;                  /* Result for this mapping */
    unsigned long long exp = MOD_EXPONENT;          /* Exponent number for this mapping */

    number = number % MOD_MODULUS;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * number) % MOD_MODULUS;
        }
        exp = exp >> 1;
        number = (number * number) % MOD_MODULUS;
    }

    return result;
}

