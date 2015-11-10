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
#include "pipe.h"
#include "scheduling.h"
#include "time.h"
#include "memwatch.h"

FILE *fp;                       /* The file pointer that used to open config file */
char *enc_txt;                  /* Used to store encrypted file name */
char *dec_txt;                  /* Used to store decrypted file name */
char *out_time;                 /* Used to store output time */
int main_flag = 0;              /* Used to store return value for main function */
int schedule_flag = 0;          /* Flag equls to 0 means using 'round robin' scheduling algorithm,
                                   flag equls to 1 means using 'first come first served' scheduling algorithm,
                                   default 0 */
int cnt_rr = 0;                 /* Counter for round robin, value means which process is the one that we want to assign task */

pid_t pid;                      /* Used to store fork pid */
int *pid_array;                 /* Used to store all the child pid (as int) */
int process_number_limit;       /* The max number of processes that can use now */
int process_number_now = -1;    /* The number of processes that already been used now */

int *parent_to_child;           /* Pipe that used to transmit message from parent process to child process */
int *child_to_parent;           /* Pipe that used to transmit message from child process to parent process */

fd_set rfds;                    /* The set of file descriptor */
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
 * Function: Get_sched_algo
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

void get_sched_algo(void) {
    char *schedule;
    size_t len;
    size_t read = getline(&schedule, &len, fp);                         /* Get first line input */
    if ((int)read == -1) {                                              /* If this is an empty file */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Empty file! Scheduling algorithm not found!\n", out_time, getpid());
        free(out_time);
        exit(1);
    } else if (!strcmp(trim_space(schedule), "round robin")) {          /* If first line equals to 'round robin' */
        schedule_flag = 0;
    } else if (!strcmp(trim_space(schedule), "fcfs")) {                 /* If first line equals to 'fcfs' */
        schedule_flag = 1;
    } else {                                                            /* If is other incorrect string */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: I don't know this scheduling algorithm! :C\n", out_time, getpid());
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

    if (argc != 2) {                    /* Check if the arguments number is right or not */
        get_time();
        printf("[%s] (Process ID #%d) Arguments number is not right! Usage: ./lyrebird config_file_name_here.txt\n", out_time, getpid());
        free(out_time);
        return 1;
    }

    fp = fopen(argv[1], "r+");          /* Input config file */

    if (fp == NULL) {                   /* Check if the input file is exist or not */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Input file %s not exist!\n", out_time, getpid(), argv[1]);
        free(out_time);
        return 1;
    }

    get_sched_algo();                   /* Get scheduling algorithm */
    init_pipe();                        /* Initialize pipe */

    enc_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);
    dec_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);

    while (fscanf(fp, "%s", enc_txt) != EOF) {              /* Read until end of file */
        fscanf(fp, "%s", dec_txt);

        if (process_number_now + 1 < process_number_limit) {            /* If not exceed the max number of process */
            process_number_now++;
            pid = fork();                                   /* Fork! */
            if (pid < 0) {                                  /* If fork failed */
                get_time();
                printf("[%s] (Process ID #%d) ERROR: Fork failed!\n", out_time, getpid());
                main_flag = 1;                              /* Exit non-zero value */
                break;                                      /* Break and still need to wait for all child processes */
            }
        } else
            process_number_now = process_number_limit;

        if (pid != 0) {                                     /* If fork successful and is in parent process */
            if (process_number_now < process_number_limit) {            /* First round we just assgin each task to each child process */
                pid_array[process_number_now] = (int)pid;               /* Store child pid into array */
                close(parent_to_child[process_number_now * 2]);
                close(child_to_parent[process_number_now * 2 + 1]);     /* Close pipes that will not be used */
                get_time();
                printf("[%s] Child process ID #%d will decrypt %s.\n", out_time, *(pid_array + process_number_now), enc_txt);
                write(parent_to_child[process_number_now * 2 + 1], enc_txt, sizeof(char) * FILE_MAXLENGTH);
                write(parent_to_child[process_number_now * 2 + 1], dec_txt, sizeof(char) * FILE_MAXLENGTH);
            } else {
                if (schedule_flag == 0) {                   /* "Round robin" scheduling algorithm */
                    round_robin();
                } else {                                    /* "First come first serve" scheduling algorithm */
                    fcfs();
                    if (main_flag) break;
                }
            }
        }

        if (pid == 0) {                                             /* If in child process */
            int state = 0;
            char enc_txt[FILE_MAXLENGTH];
            char dec_txt[FILE_MAXLENGTH];

            close_ptc_pipes_except(process_number_now);             /* Close other pipes except used ptc and ctp */
            close_ctp_pipes_except(process_number_now);
            close(parent_to_child[process_number_now * 2 + 1]);
            close(child_to_parent[process_number_now * 2]);

            while (1) {                                             /* Keep decrpting until break */
                if (read(parent_to_child[process_number_now * 2], &enc_txt, sizeof(char) * FILE_MAXLENGTH) == 0) break;    /* Break if parent process's pipe closed */
                read(parent_to_child[process_number_now * 2], &dec_txt, sizeof(char) * FILE_MAXLENGTH);
                state = decrypt(enc_txt, dec_txt);
                if (state == 1) break;                              /* If child decryption meet an fatal error */
                if (state == 0) {                                   /* If child process is ready to decrypt another file */
                    write(child_to_parent[process_number_now * 2 + 1], &process_number_now, sizeof(int));
                }
            }

            clean_up();                                             /* Always remember to free all and close file pointer! */
            close(parent_to_child[process_number_now * 2]);         /* Close used pipes */
            close(child_to_parent[process_number_now * 2 + 1]);
            exit(state);
        }
    }

    close_all_ptc_pipes();                                  /* Close all parent to child pipes */
    read_rmng_msg();                                        /* Read remaining messages in all child processes */

    for (i = 0; i < process_number_limit; i++) {            /* Parent process wait for all child processes before exit */
        int state;
        pid_t pid = wait(&state);                           /* Wait until found one child process finished */
        if (state != 0) {                                   /* If child process terminate unexpectly! */
            get_time();
            printf("[%s] Child process ID #%d did not terminate successfully.\n", out_time, (int)pid);
        }
        close_ctp_pipe_with_pid(pid);                       /* Close the pipe that uses to read messages from exited child process */
    }

    clean_up();                                             /* Always remember to free all and close file pointer! */
    return main_flag;
}
