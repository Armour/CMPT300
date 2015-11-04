/*------------------------------------------------------
 *
 *  decrypt.c
 *
 *  this file contains the main decryption functions which go
 *  through 4 steps and in each step, call some functions in
 *  dec_func.c to handle decryption.
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

#include "decrypt.h"
#include "dec_func.h"
#include "line_io.h"
#include "time.h"
#include "memwatch.h"

int len = 0;                                /* Length of tweet */
int flag = 0;                               /* Used to show program running status, if equal to 1, means there is some problems */
char *tweets_enc;                           /* Used to store the encrypted tweet */
char *tweets_dec;                           /* Used to store the decrypted tweet */
char *group_char;                           /* Used to store each group of characters */
char *tweets_time;                          /* Used to store output time */
unsigned long long *cipher_number;          /* Used to store cipher number */
unsigned long long *ptext_number;           /* Used to store plain-text number */

/*
 * Function: Get_tweets_time
 * -------------------
 *   This function is used to get current time in specify format
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void get_tweets_time(void) {
    time_t raw_time;                /* Used to generate output time */
    struct tm *tmp_time;            /* Used to generate output time */
    time(&raw_time);
    tmp_time = localtime(&raw_time);
    strftime(tweets_time, TIME_MAXLENGTH, "%a %b %d %H:%M:%S %Y", tmp_time);       /* Format time */
}

/*
 * Function: decrypt_each
 * -------------------
 *   Decrypt each tweet through 4 steps
 *
 *   Parameters:
 *      tweets_enc: the encrypted tweet that we want to decrypt
 *
 *   Returns:
 *      decrypted tweet information
 */

char *decrypt_each(char *tweets_enc) {
    int i, j;
    int num_len;

    tweets_dec = (char *)malloc(sizeof(char) * TWEETS_MAX_LENGTH);
    if (tweets_dec == NULL) {                   /* Failed on alloc */
        get_tweets_time();                      /* Print error message in child process */
        printf("[%s] Child process ID #%d encountered an error: Malloc failed! Process exit.\n", tweets_time, getpid());
        flag = 1;
        return tweets_dec;
    }

    /* Step 1: Remove unnecessary characters in regular interval */
    rm_extra_char(tweets_enc, tweets_dec, &len);

    if (len % CONSTANT_MULTIPLE != 0) {         /* Check if len is multiple of 6 */
        get_tweets_time();                      /* Print error message in child process */
        printf("[%s] Child process ID #%d encountered an error: There is a tweet that is not multiple of 6!\n", tweets_time, getpid());
        flag = 0;
        return tweets_dec;
    }

    /* Step 2: Transform each group of 6 characters into an integer using base 41 */
    num_len = len / CONSTANT_MULTIPLE;
    cipher_number = (unsigned long long *)malloc(sizeof(unsigned long long) * num_len);
    if (cipher_number == NULL) {                /* Failed on alloc */
        get_tweets_time();                      /* Print error message in child process */
        printf("[%s] Child process ID #%d encountered an error: Malloc failed! Process exit.\n", tweets_time, getpid());
        flag = 1;
        return tweets_dec;
    }

    for (i = 0; i < num_len; ++i) {
        cipher_number[i] = base41_ctoi(tweets_dec, i * CONSTANT_MULTIPLE);
    }

    /* Step 3: Map each cipher number onto a similar plain-text number */
    ptext_number = (unsigned long long *)malloc(sizeof(unsigned long long) * num_len);
    if (ptext_number == NULL) {                 /* Failed on alloc */
        get_tweets_time();                      /* Print error message in child process */
        printf("[%s] Child process ID #%d encountered an error: Malloc failed! Process exit.\n", tweets_time, getpid());
        flag = 1;
        return tweets_dec;
    }

    for (i = 0; i < num_len; ++i) {
        ptext_number[i] = mapping_exp(cipher_number[i]);
    }

    /* Step 4: Get the final decrypted text by use the inverse function of Step 2 */
    group_char = (char *)malloc(sizeof(char) * CONSTANT_MULTIPLE);
    if (group_char == NULL) {                   /* Failed on alloc */
        get_tweets_time();                      /* Print error message in child process */
        printf("[%s] Child process ID #%d encountered an error: Malloc failed! Process exit.\n", tweets_time, getpid());
        flag = 1;
        return tweets_dec;
    }

    for (i = 0; i < num_len; ++i) {
        base41_itoc(ptext_number[i], group_char);
        for (j = 0; j < CONSTANT_MULTIPLE; ++j) {
            tweets_dec[i * CONSTANT_MULTIPLE + j] = group_char[j];
        }
    }

    free(group_char);
    free(cipher_number);
    free(ptext_number);

    return tweets_dec;
}

/*
 * Function: decrypt
 * -------------------
 *   Main function of decryption
 *
 *   Parameters:
 *      input: the file name of encrypted file
 *      output: the file name of decrypted file
 *
 *   Returns:
 *      return 0 for normal exit
 */

int decrypt(char *input, char *output) {
    FILE *fin, *fout;

    tweets_time = (char *)malloc(sizeof(char) * TIME_MAXLENGTH);

    fin = fopen(input, "r");

    if (fin == NULL) {                              /* Check input file is exist or not */
        get_tweets_time();                          /* Print error message in child process */
        printf("[%s] Child process ID #%d encounter error: Error when opening file %s, input file not exist!\n", tweets_time, getpid(), input);
        free(tweets_time);
        return 0;
    }

    fout = fopen(output, "w+");

    if (fout == NULL) {                             /* Check if output file opened successfully */
        get_tweets_time();                          /* Print error message in child process */
        printf("[%s] Child process ID #%d encounter error: Error when opening file %s, output file create failed!\n", tweets_time, getpid(), input);
        free(tweets_time);
        fclose(fin);
        return 0;
    }

    while (!feof(fin)) {
        tweets_enc = input_line(fin, &len);             /* Get each line's tweet from input file */
        if (len != -1) {
            tweets_dec = decrypt_each(tweets_enc);      /* Decrypt each line's tweet */
            if (flag) {                                 /* If there is some unexpected problems, exit */
                free(tweets_dec);
                free(tweets_enc);
                break;
            }
            output_line(fout, tweets_dec, len);         /* Ouput the decrypted tweet */
            free(tweets_dec);
        }
        free(tweets_enc);
    }

    fclose(fin);
    fclose(fout);

    if (flag == 0) {                                    /* If decrypt tweets without error */
        get_tweets_time();                              /* Print error message in child process */
        printf("[%s] Process ID #%d decrypted %s successfully.\n", tweets_time, getpid(), input);
        free(tweets_time);
    }

    return flag;
}
