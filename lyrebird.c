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
#include "memwatch.h"
#include "time.h"
//#include "string.h"

int main(int argc, char *argv[]) {
    FILE *fp;
    char *enc_txt;
    char *dec_txt;
    char *out_time;
    time_t raw_time;
    int i, count = 0;

    if (argc != 2) {
        printf("erorr!\n");
        return 1;
    }

    fp = fopen(argv[1], "r+");
    enc_txt = (char *)malloc(sizeof(char) * 1025);
    dec_txt = (char *)malloc(sizeof(char) * 1025);
    pid_t pid;

    while (1) {
        fscanf(fp, "%s", enc_txt);
        fscanf(fp, "%s", dec_txt);
        if (feof(fp)) {
            free(enc_txt);
            free(dec_txt);
            fclose(fp);
            break;
        }

        pid = fork();
        time(&raw_time);
        out_time = ctime(&raw_time);

        if (pid < 0) {
            // fork failed
            printf("fork() failed!\n");
            return 1;
        }

        if (pid != 0) {
            count++;
            printf("[%s] Child process ID #%d created to decrypt %s\n", out_time, (int)pid, enc_txt);
        } else if (pid == 0) {
            // child process
            printf("in child process %d\n", getpid());
            sleep(3);
            printf("Done child %d\n", getpid());
            free(enc_txt);
            free(dec_txt);
            return 0;
        }
    }

    for(i = 0; i < count; i++) {
        wait(NULL);
        printf("Done %d\n", i);
    }

    printf("exit!\n");
    return 0;
}
