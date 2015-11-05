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
int main_flag = 0;              /* Used to store return value for main function */
int schedule_flag = 0;          /* Flag equls to 0 means using 'round robin' scheduling algorithm,
                                   flag equls to 1 means using 'first come first served' scheduling algorithm,
                                   default 0 */
int cnt_rr = 0;                 /* Counter for round robin, value means which processor is the one that we want to assign task */

pid_t pid;                      /* Used to store fork pid */
int *pid_array;                 /* Used to store all the child pid (as int) */
int processor_number_limit;     /* The max number of processors that can use now */
int processor_number_now = -1;  /* The number of processors that already been used now */

int *parent_to_child;           /* Pipe that used to transmit messagee from parent processor to child processor */
int *child_to_parent;           /* Pipe that used to transmit messagee from child processor to parent processor */

fd_set rfds;                    /* The set of file descriptor */
struct timeval tv;              /* Time interval used on select function */
int max_descriptor;             /* The max number of file descriptor */

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
    processor_number_limit = PROCESSOR_MAX_NUMBER;                      /* Max number of processors for this machine */

    parent_to_child = (int *)malloc(sizeof(int) * processor_number_limit * 2);
    if (parent_to_child == NULL) {
        get_time();
        printf("[%s] (Processor ID #%d) ERROR: Malloc parent_to_child failed!\n", out_time, getpid());
        free(out_time);
        exit(1);
    }

    child_to_parent = (int *)malloc(sizeof(int) * processor_number_limit * 2);
    if (child_to_parent == NULL) {
        get_time();
        printf("[%s] (Processor ID #%d) ERROR: Malloc child_to_parent failed!\n", out_time, getpid());
        free(out_time);
        free(parent_to_child);
        exit(1);
    }

    pid_array = (int *)malloc(sizeof(int) * processor_number_limit);
    if (pid_array == NULL) {
        get_time();
        printf("[%s] (Processor ID #%d) ERROR: Malloc pid_array failed!\n", out_time, getpid());
        free(out_time);
        free(parent_to_child);
        free(child_to_parent);
        exit(1);
    }

    memset(pid_array, 0, sizeof(int) * processor_number_limit);

    for (i = 0; i < processor_number_limit; ++i) {
        if (pipe(parent_to_child + i * 2)) {                            /* Create pipe from parent to child */
            get_time();
            printf("[%s] (Processor ID #%d) ERROR: Create parent to child pipe number %d failed!\n", out_time, getpid(), i);
            free(out_time);
            free(parent_to_child);
            free(child_to_parent);
            free(pid_array);
            exit(1);                                                    /* If create pipe failed */
        }
        if (pipe(child_to_parent + i * 2)) {                            /* Create pipe from child to parent */
            get_time();
            printf("[%s] (Processor ID #%d) ERROR: Create child to parent pipe number %d failed!\n", out_time, getpid(), i);
            free(out_time);
            free(parent_to_child);
            free(child_to_parent);
            free(pid_array);
            exit(1);                                                    /* If create pipe failed */
        }
    }
}

/*
 * Function: Close_ptc_pipe
 * -------------------
 *   This function is used to close the pipes from parent to child
 *
 *   Parameters:
 *      p: the number of the processor which we do not want to close its pipe
 *
 *   Returns:
 *      void
 */

void close_ptc_pipe(int p) {
    int i;
    for (i = 0; i < processor_number_limit * 2; ++i) {                  /* Close each pipe from parent to child except processor p */
        if (i/2 != p)
            close(parent_to_child[i]);
    }
}

/*
 * Function: Close_ctp_pipe
 * -------------------
 *   This function is used to close the pipes from child to parent
 *
 *   Parameters:
 *      p: the number of the processor which we do not want to close its pipe
 *
 *   Returns:
 *      void
 */

void close_ctp_pipe(int p) {
    int i;
    for (i = 0; i < processor_number_limit * 2; ++i) {                  /* Close each pipe from child to parent except processor p  */
        if (i/2 != p)
            close(child_to_parent[i]);
    }
}

/*
 * Function: Init_select
 * -------------------
 *   This function is used to prepare select function for pipe in different processors
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void init_select(void) {
    int i;
    int max = 0;
    FD_ZERO(&rfds);                                             /* First initialize set to empty */
    for (i = 0; i < processor_number_limit; ++i) {
        FD_SET(child_to_parent[i * 2], &rfds);                  /* Add file descriptor to set */
        if (child_to_parent[i * 2] > max)
            max = child_to_parent[i * 2];                       /* Calculate max file descriptor */
    }
    max_descriptor = max;
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
    size_t read = getline(&schedule, &len, fp);                         /* Get first line input */
    if ((int)read == -1) {                                              /* If this is an empty file */
        get_time();
        printf("[%s] (Processor ID #%d) ERROR: Empty file! Scheduling algorithm not found!\n", out_time, getpid());
        free(out_time);
        exit(1);
    } else if (!strcmp(trim_space(schedule), "round robin")) {          /* If first line equals to 'round robin' */
        schedule_flag = 0;
    } else if (!strcmp(trim_space(schedule), "fcfs")) {                 /* If first line equals to 'fcfs' */
        schedule_flag = 1;
    } else {                                                            /* If is other incorrect string */
        get_time();
        printf("[%s] (Processor ID #%d) ERROR: I don't know this scheduling algorithm! :C\n", out_time, getpid());
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
    free(pid_array);
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
    int i;

    out_time = (char *)malloc(sizeof(char) * TIME_MAXLENGTH);

    if (argc != 2) {                /* Check if the arguments number is right or not */
        get_time();
        printf("[%s] (Processor ID #%d) Arguments number is not right! Usage: ./lyrebird config_file_name_here.txt\n", out_time, getpid());
        free(out_time);
        return 1;
    }

    fp = fopen(argv[1], "r+");      /* Input config file */

    if (fp == NULL) {               /* Check if the input file is exist or not */
        get_time();
        printf("[%s] (Processor ID #%d) ERROR: Input file %s not exist!\n", out_time, getpid(), argv[1]);
        free(out_time);
        return 1;
    }

    get_schedule();                 /* Get scheduling algorithm */
    init_pipe();                    /* Initialize pipe */

    enc_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);
    dec_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);

    while (fscanf(fp, "%s", enc_txt) != EOF) {              /* Read until end of file */
        fscanf(fp, "%s", dec_txt);

        if (processor_number_now + 1 < processor_number_limit) {            /* If not exceed the max number of processor */
            processor_number_now++;
            pid = fork();                                   /* Fork! */
            if (pid < 0) {                                  /* If fork failed */
                get_time();
                printf("[%s] (Processor ID #%d) ERROR: Fork failed!\n", out_time, getpid());
                main_flag = 1;                              /* Exit non-zero value */
                break;                                      /* Break and still need to wait for all child processes */
            }
        } else
            processor_number_now = processor_number_limit;

        if (pid != 0) {                                     /* If fork successful and is in parent process */
            if (processor_number_now < processor_number_limit) {
                pid_array[processor_number_now] = (int)pid;             /* Store child pid into array */
                close(parent_to_child[processor_number_now * 2]);
                close(child_to_parent[processor_number_now * 2 + 1]);
                get_time();
                printf("[%s] Child process ID #%d will decrypt %s.\n", out_time, *(pid_array + processor_number_now), enc_txt);
                write(parent_to_child[processor_number_now * 2 + 1], enc_txt, sizeof(char) * FILE_MAXLENGTH);
                write(parent_to_child[processor_number_now * 2 + 1], dec_txt, sizeof(char) * FILE_MAXLENGTH);
            } else {
                if (schedule_flag == 0) {                   /* Round robin scheduling algorithm */
                    get_time();
                    printf("[%s] Child process ID #%d will decrypt %s.\n", out_time, *(pid_array + cnt_rr), enc_txt);
                    write(parent_to_child[cnt_rr * 2 + 1], enc_txt, sizeof(char) * FILE_MAXLENGTH);
                    write(parent_to_child[cnt_rr * 2 + 1], dec_txt, sizeof(char) * FILE_MAXLENGTH);
                    cnt_rr++;                               /* Update counter of round robin */
                    if (cnt_rr == processor_number_limit)
                        cnt_rr = 0;
                } else {                                    /* First come first serve scheduling algorithm */
                    init_select();
                    int state = select(max_descriptor + 1, &rfds, NULL, NULL, NULL);
                    if (state == -1) {                      /* If select function failed */
                        get_time();
                        printf("[%s] (Processor ID #%d) ERROR: Select function failed, return -1.\n", out_time, getpid());
                        main_flag = 1;
                        break;
                    } else if (state) {                     /* If select is OK */
                        for (i = 0; i < processor_number_limit; ++i) {
                            if (FD_ISSET(child_to_parent[i * 2], &rfds)) {
                                int tmp;
                                read(child_to_parent[i * 2], &tmp, sizeof(int));
                                get_time();
                                printf("[%s] Child process ID #%d will decrypt %s.\n", out_time, *(pid_array + i), enc_txt);
                                write(parent_to_child[i * 2 + 1], enc_txt, sizeof(char) * FILE_MAXLENGTH);
                                write(parent_to_child[i * 2 + 1], dec_txt, sizeof(char) * FILE_MAXLENGTH);
                                break;
                            }
                        }
                    } else {                                /* If select function time-out */
                        get_time();
                        printf("[%s] (Processor ID #%d) ERROR: Select function time-out, return 0.\n", out_time, getpid());
                        main_flag = 1;
                        break;
                    }
                }
            }
        }

        if (pid == 0) {                                     /* If in child process */
            int state = 0;
            char enc_txt[FILE_MAXLENGTH];
            char dec_txt[FILE_MAXLENGTH];

            close_ptc_pipe(processor_number_now);           /* Close other pipes except used ptc and ctp */
            close_ctp_pipe(processor_number_now);
            close(parent_to_child[processor_number_now * 2 + 1]);
            close(child_to_parent[processor_number_now * 2]);

            while (1) {
                if (read(parent_to_child[processor_number_now * 2], &enc_txt, sizeof(char) * FILE_MAXLENGTH) == 0) break;       /* Read until parent pipe closed */
                read(parent_to_child[processor_number_now * 2], &dec_txt, sizeof(char) * FILE_MAXLENGTH);
                state = decrypt(enc_txt, dec_txt);
                if (state == 1) break;                      /* If child decryption meet an fatal error */
                if (state == 0) {                           /* If child process is ready to decrypt another file */
                    write(child_to_parent[processor_number_now * 2 + 1], &processor_number_now, sizeof(int));
                }
            }

            clean_up();                                     /* Always remember to free all and close file pointer! */
            close(parent_to_child[processor_number_now * 2]);           /* Close used pipes */
            close(child_to_parent[processor_number_now * 2 + 1]);
            exit(state);
        }
    }

    close_ptc_pipe(processor_number_limit);                 /* Close all parent to child pipes */

    for (i = 0; i < processor_number_limit; i++) {          /* Read all remaining message in child processors */
        int tmp;
        while (read(child_to_parent[i * 2], &tmp, sizeof(int)));
    }

    for (i = 0; i < processor_number_limit; i++) {          /* Parent process wait for all child processes before exit */
        int state;
        pid_t pid = wait(&state);                           /* Wait until found one child process finished */
        get_time();
        if (state != 0) {                                   /* If child process terminate unexpectly! */
            printf("[%s] Child process ID #%d did not terminate successfully.\n", out_time, (int)pid);
        }
    }

    close_ctp_pipe(processor_number_limit);                 /* Close all child to parent pipes */

    clean_up();                                             /* Always remember to free all and close file pointer! */
    return main_flag;
}
