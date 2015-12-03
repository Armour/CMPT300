/*------------------------------------------------------
 *
 *  decrypt.c
 *
 *  this file contains the main decryption functions which go
 *  through 4 steps and in each step, call some functions in
 *  dec_func.c to handle decryption.
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

#include "decrypt.h"
#include "dec_func.h"
#include "line_io.h"
#include "memwatch.h"

int len = 0;                                /* Length of tweet */
int flag = 0;                               /* Used to show program running status, if not equals to 0, means there is some problems */
char *tweets_enc;                           /* Used to store the encrypted tweet */
char *tweets_dec;                           /* Used to store the decrypted tweet */
char *group_char;                           /* Used to store each group of characters */
char *tweets_time;                          /* Used to store output time */
unsigned long long *cipher_number;          /* Used to store cipher number */
unsigned long long *ptext_number;           /* Used to store plain-text number */

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
    if (tweets_dec == NULL) {                   /* Failed on malloc */
        flag = MALLOC_FAIL_ERROR;
        return tweets_dec;
    }

    /* Step 1: Remove unnecessary characters in regular interval */
    rm_extra_char(tweets_enc, tweets_dec, &len);

    /* Step 2: Transform each group of 6 characters into an integer using base 41 */
    num_len = len / CONSTANT_MULTIPLE;
    cipher_number = (unsigned long long *)malloc(sizeof(unsigned long long) * num_len);
    if (cipher_number == NULL) {                /* Failed on malloc */
        flag = MALLOC_FAIL_ERROR;
        return tweets_dec;
    }

    for (i = 0; i < num_len; ++i) {
        cipher_number[i] = base41_ctoi(tweets_dec, i * CONSTANT_MULTIPLE);
    }

    /* Step 3: Map each cipher number onto a similar plain-text number */
    ptext_number = (unsigned long long *)malloc(sizeof(unsigned long long) * num_len);
    if (ptext_number == NULL) {                 /* Failed on malloc */
        flag = MALLOC_FAIL_ERROR;
        return tweets_dec;
    }

    for (i = 0; i < num_len; ++i) {
        ptext_number[i] = mapping_exp(cipher_number[i]);
    }

    /* Step 4: Get the final decrypted text by use the inverse function of Step 2 */
    group_char = (char *)malloc(sizeof(char) * CONSTANT_MULTIPLE);
    if (group_char == NULL) {                   /* Failed on malloc */
        flag = MALLOC_FAIL_ERROR;
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
 *      sockfd: the socket file descriptor to the server
 *      host: the IP address of this client machine
 *
 *   Returns:
 *      return 0 for normal exit
 */

int decrypt(char *input, char *output, char *err_buffer) {
    FILE *fin, *fout;

    fin = fopen(input, "r");

    if (fin == NULL) {                                  /* Check input file is exist or not */
        snprintf(err_buffer, ERROR_MAXLENGTH, "Unable to open input file %s in process %d!\n", input, getpid());
        return OPEN_FILE_ERROR;
    }

    fout = fopen(output, "w+");

    if (fout == NULL) {                                 /* Check if output file opened successfully */
        snprintf(err_buffer, ERROR_MAXLENGTH, "Create output file failed when decrypt file %s in process %d!\n", input, getpid());
        fclose(fin);
        return OPEN_FILE_ERROR;
    }

    while (!feof(fin)) {
        tweets_enc = input_line(fin, &len);             /* Get each line's tweet from input file */
        if (tweets_enc == NULL) {
            snprintf(err_buffer, ERROR_MAXLENGTH, "Malloc tweets_enc string failed when decrypt file %s in process %d!\n", input, getpid());
            fclose(fin);
            return MALLOC_FAIL_ERROR;
        }
        if (len != -1) {
            tweets_dec = decrypt_each(tweets_enc);      /* Decrypt each line's tweet */
            if (flag == MALLOC_FAIL_ERROR) {            /* If there is some unexpected problems, exit */
                snprintf(err_buffer, ERROR_MAXLENGTH, "Malloc failed in decrypt_each function when decrypt file %s in process %d!\n", input, getpid());
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

    return flag? flag: EXIT_SUCCESS;
}
