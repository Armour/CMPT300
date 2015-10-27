/*------------------------------------------------------
 *
 *  lyrebird.c
 *
 *  this file contains the main function that using 'fork'
 *  to create child processes to decrypt each tweet file.
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
int schedule_flag = 0;          /* Flag equls to 0 means using 'round robin' scheduling algorithm,
                                   flag equls to 1 means using 'first come first served' scheduling algorithm,
                                   default 0 */
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
    strftime(out_time, TIME_MAXLENGTH, "%a %b %d %H:%M:%S %Y", tmp_time);       /* Format time */
}

/*
 * Function: Get_schedule
 * -------------------
 *   This function is uesd to get the scheduling algorithm before input tweet files
 *   If not get the right scheduling algorithm, it should return with non-zero state
 *
 *   Parameters:
 *      out_time: it needs to be free before exit
 *
 *   Returns:
 *      void
 */

void get_schedule(char *out_time) {
    char *schedule;
    size_t len;
    size_t read = getline(&schedule, &len, fp);             /* Get first line input */
    if ((int)read == -1) {
        printf("Null!\n");                                  /* If this is an empty file */
        free(out_time);
        exit(1);
    } else if (!strcmp(schedule, "round robin\n")) {        /* If first line equals to 'round robin' */
        schedule_flag = 0;
    } else if (!strcmp(schedule, "fcfs\n")) {               /* If first line equals to 'fcfs' */
        schedule_flag = 1;
    } else {                                                /* If is other incorrect string */
        printf("No!\n");
        free(out_time);
        exit(1);
    }
}

/*
 * Function: Clean_up
 * -------------------
 *   This function is used to free memory and close file pointer before program exit
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void clean_up(void) {               /* Always remember to free all and close file pointer! */
    free(enc_txt);
    free(dec_txt);
    free(out_time);
    fclose(fp);
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
    int flag = 0;                   /* Used to store return value for main function */

    out_time = (char *)malloc(sizeof(char) * TIME_MAXLENGTH);

    if (argc != 2) {                /* Check if the arguments number is right or not */
        get_time(out_time);
        printf("[%s] (#%d) Arguments number is not right! Usage: ./lyrebird config_file_name_here.txt\n", out_time, getpid());
        free(out_time);
        return 1;
    }

    fp = fopen(argv[1], "r+");      /* Input config file */

    if (!fp) {                      /* Check if the input file is exist or not */
        get_time(out_time);
        printf("[%s] (#%d) Error when opening file %s, input file not exist!\n", out_time, getpid(), argv[1]);
        free(out_time);
        return 1;
    }

    get_schedule(out_time);
    printf("%d\n", schedule_flag);

    enc_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);
    dec_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);

    while (fscanf(fp, "%s", enc_txt) != EOF) {      /* Read until end of file */
        fscanf(fp, "%s", dec_txt);

        pid = fork();                               /* Fork! */

        if (pid < 0) {                              /* If fork failed */
            get_time(out_time);
            printf("[%s] Fork failed in create new process #%d!\n", out_time, getpid());
            clean_up();                             /* Always remember to free all and close file pointer! */
            flag = 1;                               /* Exit non-zero value */
            break;                                  /* Break and still need to wait for all child processes */
        }

        if (pid != 0) {                             /* If fork successful and is in parent process */
            count++;
            get_time(out_time);
            printf("[%s] Child process ID #%d created to decrypt %s\n", out_time, (int)pid, enc_txt);
        }

        if (pid == 0) {                             /* If in child process */
            int state = decrypt(enc_txt, dec_txt);
            get_time(out_time);
            if (state == 0)                         /* If child process exit without error */
                printf("[%s] Decryption of %s complete. Process ID #%d Exiting.\n", out_time, enc_txt, getpid());
            clean_up();                             /* Always remember to free all and close file pointer! */
            exit(state);
        }
    }

    for (i = 0; i < count; i++) {                   /* Parent process wait for all child processes before exit */
        int state;
        int pid = wait(&state);                     /* Wait until found one child process finished */
        get_time(out_time);
        if (state != 0) {                           /* If child process terminate unexpectly! */
            printf("[%s] Child process ID #%d did not terminate successfully.\n", out_time, pid);
        }
    }

    clean_up();                                     /* Always remember to free all and close file pointer! */
    return flag;
}
