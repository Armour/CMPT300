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

#define PROCESSOR_MAX_NUMBER sysconf(_SC_NPROCESSORS_ONLN)

FILE *fp;                       /* The file pointer that used to open config file */
char *enc_txt;                  /* Used to store encrypted file name */
char *dec_txt;                  /* Used to store decrypted file name */
char *out_time;                 /* Used to store output time */
int schedule_flag = 0;          /* Flag equls to 0 means using 'round robin' scheduling algorithm,
                                   flag equls to 1 means using 'first come first served' scheduling algorithm,
                                   default 0 */
pid_t pid;                      /* Used to store fork pid */
int processor_number_limit;     /* The max number of processors that can use now */
int processor_number_now = 0;   /* The number of processors that already been used now */
int *parent_to_child;           /* Pipe that used to transmit messagee from parent processor to child processor */
int *child_to_parent;           /* Pipe that used to transmit messagee from child processor to parent processor */
int main_flag = 0;              /* Used to store return value for main function */

/*
 * Function: Get_time
 * -------------------
 *   This function is used to get current time in specify format
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void get_time(void) {
    time_t raw_time;                /* Used to generate output time */
    struct tm *tmp_time;            /* Used to generate output time */
    time(&raw_time);
    tmp_time = localtime(&raw_time);
    strftime(out_time, TIME_MAXLENGTH, "%a %b %d %H:%M:%S %Y", tmp_time);       /* Format time */
}


/*
 * Function: Init_pipe
 * -------------------
 *   This function is used to initialize the max number of pipe descriptor
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void init_pipe(void) {
    int i;
    processor_number_limit = PROCESSOR_MAX_NUMBER;
    parent_to_child = (int *)malloc(sizeof(int) * processor_number_limit * 2);
    child_to_parent = (int *)malloc(sizeof(int) * processor_number_limit * 2);
    for (i = 0; i < processor_number_limit; ++i) {
        if (pipe (parent_to_child + i * 2)) {                            /* Create pipe! */
            get_time();
            printf("[%s] Create parent to child pipe #%d failed!\n", out_time, i);
            main_flag = 1;
            break;
        } else printf("pipe ptc #%d!\n", i);
        if (pipe (child_to_parent + i * 2)) {                        /* Create pipe! */
            get_time();
            printf("[%s] Create child to parent pipe #%d failed!\n", out_time, i);
            main_flag = 1;
            break;
        } else printf("pipe ctp #%d!\n", i);
    }
}

void close_ptc_pipe(int p) {
    int i;
    for (i = 0; i < processor_number_limit * 2; ++i) {
        if (i/2 != p)
            close(parent_to_child[i]);
    }
}

void close_ctp_pipe(int p) {
    int i;
    for (i = 0; i < processor_number_limit * 2; ++i) {
        if (i/2 != p)
            close(child_to_parent[i]);
    }
}

/*
 * Function: Trim_space
 * -------------------
 *   This function is used to trim leading and trailing spaces in a string.
 *
 *   Parameters:
 *      str: input string that needs to trimming
 *
 *   Returns:
 *      a new string without leading and trailing spaces
 */

char *trim_space(char *str) {
    char *end;
    end = str + strlen(str) - 1;
    while (*str == ' ') str++;                  /* Trim the leading spaces */
    if (*str == '\0')                           /* If all the string contains only spaces */
        return str;
    while (end > str && (*end == ' ' || *end == '\n')) end--;           /* Trim the trailing spaces and new line character */
    *(end + 1) = '\0';                          /* Add new terminator */
    return str;
}

/*
 * Function: Get_schedule
 * -------------------
 *   This function is uesd to get the scheduling algorithm before input tweet files
 *   If not get the right scheduling algorithm, it should return with non-zero state
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void get_schedule(void) {
    char *schedule;
    size_t len;
    size_t read = getline(&schedule, &len, fp);             /* Get first line input */
    if ((int)read == -1) {
        printf("Null!\n");                                  /* If this is an empty file */
        free(out_time);
        exit(1);
    } else if (!strcmp(trim_space(schedule), "round robin")) {        /* If first line equals to 'round robin' */
        schedule_flag = 0;
    } else if (!strcmp(trim_space(schedule), "fcfs")) {               /* If first line equals to 'fcfs' */
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
    free(parent_to_child);
    free(child_to_parent);
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

    out_time = (char *)malloc(sizeof(char) * TIME_MAXLENGTH);

    if (argc != 2) {                /* Check if the arguments number is right or not */
        get_time();
        printf("[%s] (#%d) Arguments number is not right! Usage: ./lyrebird config_file_name_here.txt\n", out_time, getpid());
        free(out_time);
        return 1;
    }

    fp = fopen(argv[1], "r+");      /* Input config file */

    if (!fp) {                      /* Check if the input file is exist or not */
        get_time();
        printf("[%s] (#%d) Error when opening file %s, input file not exist!\n", out_time, getpid(), argv[1]);
        free(out_time);
        return 1;
    }

    get_schedule();
    init_pipe();

    enc_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);
    dec_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);

    while (fscanf(fp, "%s", enc_txt) != EOF) {      /* Read until end of file */
        fscanf(fp, "%s", dec_txt);

        pid = fork();                               /* Fork! */

        if (pid < 0) {                              /* If fork failed */
            get_time();
            printf("[%s] Fork failed in create new process #%d!\n", out_time, getpid());
            main_flag = 1;                          /* Exit non-zero value */
            break;                                  /* Break and still need to wait for all child processes */
        }

        if (pid != 0) {                             /* If fork successful and is in parent process */
            count++;
            get_time();
            printf("[%s] Child process ID #%d will decrypt %s.\n", out_time, (int)pid, enc_txt);
        }

        if (pid == 0) {                             /* If in child process */
            int state = decrypt(enc_txt, dec_txt);
            get_time();
            if (state == 0) {                       /* If child process exit without error */
                printf("[%s] Decryption of %s complete. Process ID #%d Exiting.\n", out_time, enc_txt, getpid());

                int a = 5;
                close_ctp_pipe(0);
                close(child_to_parent[0]);
                write(child_to_parent[1], &a, sizeof(int));
                close(child_to_parent[1]);

            }
            clean_up();                             /* Always remember to free all and close file pointer! */
            exit(state);
        }
    }

    close_ctp_pipe(0);
    close(child_to_parent[1]);

    for (i = 0; i < count; i++) {                   /* Parent process wait for all child processes before exit */
        int state;
        int pid = wait(&state);                     /* Wait until found one child process finished */
        get_time();
        if (state != 0) {                           /* If child process terminate unexpectly! */
            printf("[%s] Child process ID #%d did not terminate successfully.\n", out_time, pid);
        } else {
            int xx = 0;
            read(child_to_parent[0], &xx, sizeof(int));
            printf("!!!!!!!!!!!!%d\n", xx);
        }
    }

    close(child_to_parent[0]);

    clean_up();                                     /* Always remember to free all and close file pointer! */
    return main_flag;
}
