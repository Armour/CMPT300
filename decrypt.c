/*------------------------------------------------------
 *
 *  decrypt.c
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
#include "memwatch.h"

int len;                                    /* Length of tweet */
int flag = 0;                               /* Used to show program running status, if equal to 1, means there is some problems */
char *tweets_enc;                           /* Used to store the encrypted tweet */
char *tweets_dec;                           /* Used to store the decrypted tweet */
char *group_char;                           /* Used to store each group of characters */
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

    /* Step 1: Remove unnecessary characters in regular interval */
    rm_extra_char(tweets_enc, tweets_dec, &len);

    if (len % CONSTANT_MULTIPLE != 0) {         /* Check if len is multiple of 6 */
        flag = 1;
        printf("There is a tweet that is not multiple of 6! Check your input file please.\n");
        return tweets_dec;
    }

    /* Step 2: Transform each group of 6 characters into an integer using base 41 */
    num_len = len / CONSTANT_MULTIPLE;
    cipher_number = (unsigned long long *)malloc(sizeof(unsigned long long) * num_len);

    for (i = 0; i < num_len; ++i) {
        cipher_number[i] = base41_ctoi(tweets_dec, i * CONSTANT_MULTIPLE);
    }

    /* Step 3: Map each cipher number onto a similar plain-text number */
    ptext_number = (unsigned long long *)malloc(sizeof(unsigned long long) * num_len);

    for (i = 0; i < num_len; ++i) {
        ptext_number[i] = mapping_exp(cipher_number[i]);
    }

    /* Step 4: Get the final decrypted text by use the inverse function of Step 2 */
    group_char = (char *)malloc(sizeof(char) * CONSTANT_MULTIPLE);
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
 *   Main function of lyrebird
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      return 0 for normal exit
 */

int decrypt(int argc, char *argv[]) {
    FILE *fin, *fout;

    if (argc != 3) {                                /* Check the number of arguments */
        printf("Arguments number is not right, here is a good example:\n    ./lyrebird encrypted_tweets.txt decrypted_output.txt \n");
        return 0;
    }

    fin = fopen(argv[1], "r");
    fout = fopen(argv[2], "w+");

    if (fin == NULL) {                              /* Check input file is exist or not */
        printf("Error when opening file, input file not exist!\n");
        return 0;
    }

    while (!feof(fin)) {
        tweets_enc = input_line(fin, &len);         /* Get each line's tweet from input file */

        if (len != -1) {
            tweets_dec = decrypt_each(tweets_enc);      /* Decrypt each line's tweet */
            if (flag) {                                 /* If there is some unexpected problems, exit */
                free(tweets_dec);
                free(tweets_enc);
                break;
            }
            output_line(fout, tweets_dec, len);     /* Ouput the decrypted tweet */
            free(tweets_dec);
        }

        free(tweets_enc);
    }

    fclose(fin);
    fclose(fout);

    return 0;
}

