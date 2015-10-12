/*------------------------------------------------------
 *
 *  lyrebird.c
 *
 *  Project         : LyreBird
 *  Name            : Guo Chong
 *  Student ID      : 301295753
 *  SFU username    : armourg
 *  Lecture section : D1
 *  Instructor      : Brain G.Booth
 *  TA              : Scott Kristjanson
 *
 *  Created by Armour on 25/09/2015
 *  Copyright (c) 2015 Armour. All rights reserved.
 *
 *------------------------------------------------------
 */

#include "lyrebird.h"
#include "decrypt.h"
#include "memwatch.h"
#include "time.h"

FILE *fp;                       /* The file pointer that used to open config file */
char *enc_txt;                  /* Used to store encrypted file name */
char *dec_txt;                  /* Used to store decrypted file name */
char *out_time;                 /* Used to store output time */
pid_t pid;                      /* Used to store fork pid */

/*
 * Function: Get_time
 * -------------------
 *   This function is used to get current time in specify format
 *
 *   Parameters:
 *      out_time: the string that used to store current time
 *
 *   Returns:
 *      void
 */

void get_time(char *out_time) {
    time_t raw_time;                /* Used to generate output time */
    struct tm *tmp_time;            /* Used to generate output time */
    time(&raw_time);
    tmp_time = localtime(&raw_time);
    strftime(out_time, TIME_MAXLENGTH, "%a %b %d %H:%M:%S %Y", tmp_time);
}

/*
 * Function: Main
 * -------------------
 *   Main function of lyrebird
 *
 *   Parameters:
 *      argc: the number of parameters in main function
 *      argv[]: the array of parameters in main function
 *
 *   Returns:
 *      void
 */

int main(int argc, char *argv[]) {
    int i, count = 0;

    if (argc != 2) {                /* Check if the arguments number is right or not */
        printf("Arguments number is not right! Usage: ./lyrebird config_file_name_here.txt\n");
        return 1;
    }

    fp = fopen(argv[1], "r+");      /* Input config file */

    if (!fp) {                      /* Check if the input file is exist or not */
        printf("Error when opening file %s, input file not exist!\n", argv[1]);
        return 1;
    }

    enc_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);
    dec_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);
    out_time = (char *)malloc(sizeof(char) * TIME_MAXLENGTH);

    while (fscanf(fp, "%s", enc_txt) != EOF) {      /* Read until end of file */
        fscanf(fp, "%s", dec_txt);

        pid = fork();               /* Fork! */

        if (pid < 0) {              /* If fork failed */
            get_time(out_time);
            printf("[%s] Fork failed in process #%d!\n", out_time, getpid());
            return 1;               /* Exit non-zero value */
        }

        if (pid != 0) {             /* If fork successful and is in parent process */
            count++;
            get_time(out_time);
            printf("[%s] Child process ID #%d created to decrypt %s\n", out_time, (int)pid, enc_txt);
        } else if (pid == 0) {      /* If is in child process */
            int status = decrypt(enc_txt, dec_txt);
            get_time(out_time);
            if (status == 0) {      /* If child process exit without error */
                printf("[%s] Decryption of %s complete. Process ID #%d Exiting.\n", out_time, enc_txt, getpid());
            } else {                /* If child process terminate unexpectly! */
                printf("[%s] Child process ID #%d did not terminate successfully.\n", out_time, getpid());
            }
            free(enc_txt);          /* Remember to free all and close file pointer */
            free(dec_txt);
            free(out_time);
            fclose(fp);
            return 0;
        }
    }

    free(enc_txt);                  /* Remember to free all and close file pointer */
    free(dec_txt);
    free(out_time);
    fclose(fp);

    for(i = 0; i < count; i++) {    /* Parent process wait for all child processes before exit */
        wait(NULL);
    }

    return 0;
}
