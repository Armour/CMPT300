/*------------------------------------------------------
 *
 *  lyrebird.server.c
 *
 *  this file contains the main function that used to build
 *  a server and then assign tasks to client machine.
 *
 *  Project         : LyreBird
 *  Name            : Chong Guo
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

#include "lyrebird.server.h"
#include "decrypt.h"
#include "pipe.h"
#include "scheduling.h"
#include "time.h"
#include "memwatch.h"

int main_flag = 0;              /* Used to store return value for main function */
char *enc_txt;                  /* Used to store encrypted file name */
char *dec_txt;                  /* Used to store decrypted file name */
char *out_time;                 /* Used to store output time */
FILE *fcfg, *flog;              /* The file pointer that used to open config file and log file */

int max_descriptor;             /* The max number of file descriptor */
fd_set rfds;                    /* The set of file descriptor */

struct ifaddrs *ifaddr, *ifa;   /* Used to store the return value of getifaddrs function */
char host[NI_MAXHOST];          /* Used to store the return value of getnameinfo function */

int sockfd, sockfd_cli;
socklen_t addr_len;
struct sockaddr_in serv_addr, cli_addr;

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

void clean_up(int step) {               /* Always remember to free all and close file pointer! */
    if (step >= 1) free(out_time);
    if (step >= 2) fclose(fcfg);
    if (step >= 3) fclose(flog);
    if (step >= 4) freeifaddrs(ifaddr);
    if (step >= 5) close(sockfd);
    if (step >= 6) close(sockfd_cli);
    if (step > 6) free(enc_txt);
    if (step > 6) free(dec_txt);
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

    if (argc != 3) {                        /* Check if the arguments number is right or not */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) Arguments number is not right! Usage: ./lyrebird.server config_file.txt log_file.txt\n", out_time, getpid());
        clean_up(1);
        exit(EXIT_FAILURE);
    }

    fcfg = fopen(argv[1], "r+");            /* Input config file */

    if (fcfg == NULL) {                     /* Check if the config file is exist or not */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) ERROR: Config file %s not exist!\n", out_time, getpid(), argv[1]);
        clean_up(1);
        exit(EXIT_FAILURE);
    }

    flog = fopen(argv[2], "w");             /* Output log file */

    if (flog == NULL) {                     /* Check if the log file can be open or not */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) ERROR: Log file %s can not open!\n", out_time, getpid(), argv[2]);
        clean_up(2);
        exit(EXIT_FAILURE);
    }

    if (getifaddrs(&ifaddr) == -1) {        /* Get the ip address of this machine and check if it failed */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) ERROR: Server can not get interface addresses of this machine!\n", out_time, getpid());
        clean_up(3);
        exit(EXIT_FAILURE);
    }

    addr_len = sizeof(struct sockaddr_in);

    for (ifa = ifaddr, i = 0; ifa != NULL; ifa = ifa->ifa_next, i++) {          /* Iterate each interface addresses and get the IP address */
        if (ifa->ifa_addr == NULL)
            continue;

        int family = ifa->ifa_addr->sa_family;

        if (family != AF_INET)                                                  /* If it is not the IPv4 interface */
            continue;

        if (getnameinfo(ifa->ifa_addr, addr_len, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0) {         /* If can not get the address information of interface address */
            get_time();
            fprintf(flog, "[%s] (Process ID #%d) ERROR: Server can not get the address information of interface address!\n", out_time, getpid());
            clean_up(4);
            exit(EXIT_FAILURE);
        }

        if (strcmp(host, "0.0.0.0") == 0 || strcmp(host, "127.0.0.1") == 0)     /* Filter the special IP address "0.0.0.0" and "127.0.0.1" */
            continue;

        break;                                                                  /* If found a avaliable Ip address, then we break */
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {                       /* Create socket in server */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) ERROR: Server can not create a new socket!\n", out_time, getpid());
        clean_up(4);
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, addr_len);                                            /* Initialize server address config */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(host);
    serv_addr.sin_port = htons(0);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, addr_len) < 0) {            /* Bind the server socket */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) ERROR: Server bind socket failed!\n", out_time, getpid());
        clean_up(5);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 8) < 0) {                                                /* Listen at the server socket */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) ERROR: Server listen at socket failed!\n", out_time, getpid());
        clean_up(5);
        exit(EXIT_FAILURE);
    }

    if (getsockname(sockfd, (struct sockaddr *)&serv_addr, &addr_len) < 0) {            /* Find out the port number that used for lyrebird in server machine */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) ERROR: Server get socket name failed!\n", out_time, getpid());
        clean_up(5);
        exit(EXIT_FAILURE);
    }

    get_time();                                                                         /* Print out the server information */
    fprintf(flog, "[%s] lyrebird.server: PID %d on host %s, port %d\n", out_time, getpid(), inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));

    if ((sockfd_cli = accept(sockfd, (struct sockaddr *)&cli_addr, &addr_len)) < 0) {           /* Accept new connections from client machine */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) ERROR: Server accept client socket failed!\n", out_time, getpid());
        clean_up(5);
        exit(EXIT_FAILURE);
    }

    enc_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);
    dec_txt = (char *)malloc(sizeof(char) * FILE_MAXLENGTH);

    while (fscanf(fcfg, "%s", enc_txt) != EOF) {                /* Read until end of file */
        fscanf(fcfg, "%s", dec_txt);
        //read(sockfd_cli, &)
    }

    clean_up(7);                                                /* Always remember to free all and close file pointer! */
    return main_flag;
}
