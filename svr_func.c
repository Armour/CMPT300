/*------------------------------------------------------
 *
 *  svr_func.c
 *
 *  this file contains many functions that used in lyrebird server side
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

#include "svr_func.h"
#include "time.h"
#include "memwatch.h"

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
 * Function: Send_socket_msg
 * -------------------
 *   This function is used to send a message through socket with its length send first
 *
 *   Parameters:
 *      socket: the socket file descriptor
 *      msg: the message that need to send
 *
 *   Returns:
 *      void
 */

void send_socket_msg(int socket, char *msg) {
    msg_len = htonl(strlen(msg));
    send(socket, &msg_len, sizeof(uint32_t), 0);
    send(socket, msg, ntohl(msg_len), 0);
    //printf("Send msg len: !%u!\n", ntohl(msg_len));
    printf("Send msg content: !%s!\n", msg);
}

/*
 * Function: Recv_socket_msg
 * -------------------
 *   This function is used to receive a message through socket with its length at first
 *
 *   Parameters:
 *      socket: the socket file descriptor
 *      msg: the message that need to receive
 *
 *   Returns:
 *      void
 */

void recv_socket_msg(int socket, char *msg) {
    recv(socket, &msg_len, sizeof(uint32_t), 0);
    recv(socket, msg, ntohl(msg_len), 0);
    msg[ntohl(msg_len)] = '\0';
    //printf("Recv msg len: !%u!\n", ntohl(msg_len));
    printf("Recv msg content: !%s!\n", msg);
}

/*
 * Function: Check_par
 * -------------------
 *   This function is used to check the parameters in command
 *
 *   Parameters:
 *      argc: the count of arguments
 *      argv: the command arguments
 *
 *   Returns:
 *      void
 */

void check_par(int argc, char *argv[]) {
    if (argc != 3) {                        /* Check if the arguments number is right or not */
        get_time();
        printf("[%s] (Process ID #%d) Arguments number is not right! Usage: %s <config_file> <log_file>\n", out_time, getpid(), argv[0]);
        clean_up(CLEAN_TO_TIME);
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: Open_config
 * -------------------
 *   This function is used to open config file and check if it failed
 *
 *   Parameters:
 *      argv: the command arguments
 *
 *   Returns:
 *      void
 */

void open_config(char *argv[]) {
    fcfg = fopen(argv[1], "r+");            /* Input config file */
    if (fcfg == NULL) {                     /* Check if the config file is exist or not */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Config file %s not exist!\n", out_time, getpid(), argv[1]);
        clean_up(CLEAN_TO_TIME);
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: Open_log
 * -------------------
 *   This function is used to open log file and check if it failed
 *
 *   Parameters:
 *      argv: the command arguments
 *
 *   Returns:
 *      void
 */

void open_log(char *argv[]) {
    flog = fopen(argv[2], "w");             /* Output log file */
    if (flog == NULL) {                     /* Check if the log file can be open or not */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Log file %s can not open!\n", out_time, getpid(), argv[2]);
        clean_up(CLEAN_TO_CONFIG);
        exit(EXIT_FAILURE);
    }

    fuck = fopen("fuck.txt", "w");             /* Output log file */
}

/*
 * Function: Get_host_by_sockfd
 * -------------------
 *   This function is used to get host ip address through socket file descriptor
 *
 *   Parameters:
 *      sockfd: a file descriptor
 *
 *   Returns:
 *      a pointer to a host ip address
 */

char *get_host_by_sockfd(int sockfd) {
    int i;
    for (i = 0; i < CLIENT_MAXNUM; ++i) {           /* Iterate each sockfd_cli to see if it matches sockfd */
        if (sockfd_cli[i] == sockfd)
            return ipaddr_cli[i];
    }
    return NULL;
}

/*
 * Function: Get_ipaddr
 * -------------------
 *   This function is used to get the ip address of server machine
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void get_ipaddr(void) {
    int i;

    if (getifaddrs(&ifaddr) == -1) {                /* Get the ip address of this machine and check if it failed */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Server can not get the interface address of this machine!\n", out_time, getpid());
        clean_up(CLEAN_TO_LOG);
        exit(EXIT_FAILURE);
    }

    addr_len = sizeof(struct sockaddr_in);

    for (ifa = ifaddr, i = 0; ifa != NULL; ifa = ifa->ifa_next, i++) {          /* Iterate each interface addresses and get the IP address */
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family != AF_INET)                                /* If it is not the IPv4 interface */
            continue;

        if (getnameinfo(ifa->ifa_addr, addr_len, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0) {     /* If can not get the address information of interface address */
            get_time();
            printf("[%s] (Process ID #%d) ERROR: Server can not get the address information of interface address!\n", out_time, getpid());
            clean_up(CLEAN_TO_IFADDR);
            exit(EXIT_FAILURE);
        }

        if (strcmp(host, "0.0.0.0") == 0 || strcmp(host, "127.0.0.1") == 0)     /* Filter the special ip address "0.0.0.0" and "127.0.0.1" */
            continue;

        break;                                                                  /* If found a avaliable ip address, then we break */
    }

    if (strcmp(host, "0.0.0.0") == 0)                                           /* 0.0.0.0 is not what we want */
        strcpy(host, "ERROR!");
    if (strcmp(host, "127.0.0.1") == 0)                                         /* 127.0.0.1 is not what we want */
        strcpy(host, "ERROR!");
}

/*
 * Function: Create_socket
 * -------------------
 *   This function is used to create a socket in server side
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void create_socket(void) {
    int opt = TRUE;                                 /* Used when set socket options */

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {                       /* Create socket in server */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Server can not create a new socket!\n", out_time, getpid());
        clean_up(CLEAN_TO_IFADDR);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {      /* Set socket option to reuseable address (not necessary but good) */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Server can not set socket option!\n", out_time, getpid());
        clean_up(CLEAN_TO_SOCKET);
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: Bind_socket
 * -------------------
 *   This function is used to bind a socket in serve side
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void bind_socket(void) {
    memset(&serv_addr, 0, addr_len);                                            /* Initialize server address config */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(0);

    if (inet_pton(AF_INET, host, &(serv_addr.sin_addr)) <= 0) {                 /* Check IP address */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Host in inet_pton function is not a valid IP address!\n", out_time, getpid());
        clean_up(CLEAN_TO_SOCKET);
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)&serv_addr, addr_len) < 0) {            /* Bind the server socket */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Server bind socket failed!\n", out_time, getpid());
        clean_up(CLEAN_TO_SOCKET);
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: Listen_socket
 * -------------------
 *   This function is used to listen a socket in server side
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void listen_socket(void) {
    if (listen(sockfd, CLIENT_MAXNUM) < 0) {                                            /* Listen at the server socket */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Server listen at socket failed!\n", out_time, getpid());
        clean_up(CLEAN_TO_SOCKET);
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: Print_server_info
 * -------------------
 *   This function is used to find out and print the info of server side
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void print_server_info(void) {
    if (getsockname(sockfd, (struct sockaddr *)&serv_addr, &addr_len) < 0) {            /* Find out the port number that used for lyrebird in server machine */
        get_time();
        printf("[%s] (Process ID #%d) ERROR: Server get socket name failed!\n", out_time, getpid());
        clean_up(CLEAN_TO_SOCKET);
        exit(EXIT_FAILURE);
    }
    get_time();                                                                         /* Print out the server information */
    printf("[%s] lyrebird.server: PID %d on host %s, port %d\n", out_time, getpid(), host, ntohs(serv_addr.sin_port));
}

/*
 * Function: Init_cli_sock
 * -------------------
 *   This function is used to init socket array for client side
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void init_cli_sock(void) {
    int i;
    for (i = 0; i < CLIENT_MAXNUM; ++i) {
        sockfd_cli[i] = 0;
    }
}

/*
 * Function: Init_select
 * -------------------
 *   This function is used to init FD before each time's select
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void init_select(void) {
    int i;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);                      /* The server socket should also be include */
    max_fds = sockfd;
    for (i = 0; i < CLIENT_MAXNUM; i++) {
        if (sockfd_cli[i] > 0)                  /* If this client socket exist */
            FD_SET(sockfd_cli[i], &rfds);
        if (sockfd_cli[i] > max_fds)
            max_fds = sockfd_cli[i];
    }
}

/*
 * Function: Select_func
 * -------------------
 *   This function is a wrapper for select function
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      0 for success, -1 for failed
 */

int select_func(void) {
    if (select(max_fds + 1, &rfds, NULL, NULL, NULL) == -1) {               /* Check if select function failed */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) ERROR: Server failed when run select function!\n", out_time, getpid());
        main_flag = EXIT_FAILURE;
        return -1;
    }
    return 0;
}

/*
 * Function: Accept_new_cli
 * -------------------
 *   This function is used to accept new client
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      0 for success, -1 for failed
 */

int accept_new_cli(void) {
    if ((sockfd_new = accept(sockfd, (struct sockaddr *)&cli_addr, &addr_len)) < 0) {       /* Accept new connections from client machine */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) ERROR: Server accept client socket failed!\n", out_time, getpid());
        main_flag = EXIT_FAILURE;
        return -1;
    }
    return 0;
}

/*
 * Function: Check_connect
 * -------------------
 *   This function is used to check if a client connect successfully with a message
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

int check_connect(void) {
    printf("Recv connect msg!\n");
    recv_socket_msg(sockfd_new, recv_mark);
    if (strcmp(recv_mark, CONNECT_MSG) != 0) {                              /* If is not the connect message */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) ERROR: Server accept client socket failed!\n", out_time, getpid());
        main_flag = EXIT_FAILURE;
        return -1;
    }
    return 0;
}

/*
 * Function: Print_client_info
 * -------------------
 *   This function is used to find out and print the info of client side
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      0 for success, -1 for failed
 */

int print_client_info(void) {
    if (inet_ntop(AF_INET, &cli_addr.sin_addr, ip_buffer, sizeof(ip_buffer)) == NULL) {         /* Check if get client ip address failed */
        get_time();
        fprintf(flog, "[%s] (Process ID #%d) ERROR: Server get client ip address by inet_ntop failed!\n", out_time, getpid());
        main_flag = EXIT_FAILURE;
        return -1;
    }
    get_time();
    fprintf(flog, "[%s] Successfully connected to lyrebird client %s.\n", out_time, ip_buffer);
    return 0;
}

/*
 * Function: Store_client_ip
 * -------------------
 *   This function is used to store the client ip address into sockfd_cli array
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void store_client_ip(void) {
    int i;
    for (i = 0; i < CLIENT_MAXNUM; ++i) {
        if (sockfd_cli[i] == 0) {                               /* If find a empty place for storing client socket */
            sockfd_cli[i] = sockfd_new;
            printf("new client in sokcet %d array %d\n", sockfd_new, i);
            memset(ipaddr_cli[i], 0, sizeof(ipaddr_cli[i]));
            strcpy(ipaddr_cli[i], ip_buffer);
            break;
        }
    }
}

/*
 * Function: Handle_success
 * -------------------
 *   This function is uead to handle success message
 *
 *   Parameters:
 *      sock_num: the socket number that we need to handle with
 *
 *   Returns:
 *      void
 */

void handle_success(int sock_num) {
    char recv_pid[PID_MAXLENGTH];                   /* This is the pid number that read from one socket to client side */
    char recv_buffer[FILE_MAXLENGTH];               /* This is the buffer that we used to read message from socket */
    printf("Recv handle success!\n");
    recv_socket_msg(sock_num, recv_buffer);
    recv_socket_msg(sock_num, recv_pid);
    get_time();
    fprintf(flog, "[%s] The lyrebird client %s has successfully decrypted %s in process %s.\n", out_time, client_ip, recv_buffer, recv_pid);
    cnt_task++;
    printf("%u cnt: %d max: %d\n", read_type, cnt_task, max_task);
}

/*
 * Function: Handle_dispatch
 * -------------------
 *   This function is uead to handle dispatch message
 *
 *   Parameters:
 *      sock_num: the socket number that we need to handle with
 *
 *   Returns:
 *      void
 */

void handle_dispatch(int sock_num) {
    printf("Send dispatch msg!\n");
    strcpy(send_mark, CLIENT_WORK_MSG);                        /* Assign new task to client */
    send_socket_msg(sock_num, send_mark);
    send_socket_msg(sock_num, enc_txt);
    send_socket_msg(sock_num, dec_txt);

    get_time();
    fprintf(flog, "[%s] The lyrebird client %s has been given the task of decrypting %s.\n", out_time, client_ip, enc_txt);
    max_task++;
    printf("%u cnt: %d max: %d\n", read_type, cnt_task, max_task);
}

/*
 * Function: Handle_failure
 * -------------------
 *   This function is uead to handle failure message
 *
 *   Parameters:
 *      sock_num: the socket number that we need to handle with
 *
 *   Returns:
 *      void
 */

void handle_failure(int sock_num) {
    char recv_buffer[ERROR_MAXLENGTH];                  /* This is the buffer that we used to read message from socket */
    printf("Recv handle failure!\n");
    recv_socket_msg(sock_num, recv_buffer);
    get_time();
    fprintf(flog, "[%s] The lyrebird client %s has encountered an error: %s", out_time, client_ip, recv_buffer);
    cnt_task++;
    printf("%u cnt: %d max: %d\n", read_type, cnt_task, max_task);
}

/*
 * Function: Handle_client_msg
 * -------------------
 *   This function is used to handle different kinds of client message
 *
 *   Parameters:
 *      sock_num: the socket number that we need to handle with
 *
 *   Returns:
 *      void
 */

void handle_client_msg(int sock_num) {
    int need_dispatch = 0;                              /* If set to 1, then means this time server need to dispatch new task to client */
    if (strcmp(recv_mark, SUCCESS_MSG) == 0) {                  /* If it is a success message */
        handle_success(sock_num);
        need_dispatch = 1;
    } else if (strcmp(recv_mark, FAILURE_MSG) == 0) {           /* If it is a failure message */
        handle_failure(sock_num);
        need_dispatch = 1;
    }
    if (strcmp(recv_mark, DISPATCH_MSG) == 0 || need_dispatch == 1) {       /* If it is a dispatch message or need dispatch */
        if (finish_flag) return;
        handle_dispatch(sock_num);
        read_flag = 1;
        return;
    }
    get_time();
    fprintf(flog, "[%s] (Process ID #%d) ERROR: Server failed when checking message type from client machine!\n", out_time, getpid());
    sleep(100);
    main_flag = EXIT_FAILURE;
}

/*
 * Function: Ask_clients_quit
 * -------------------
 *   This function is used to ask all clients quit
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      the number of the remained clients
 */

int ask_clients_quit(void) {
    int i;
    int remained_cli = 0;
    for (i = 0; i < CLIENT_MAXNUM; ++i) {
        if (sockfd_cli[i] > 0) {                        /* If this client still connected */
            printf("Send client exit msg!\n");
            strcpy(send_mark, CLIENT_EXIT_MSG);
            send_socket_msg(sockfd_cli[i], send_mark);
            remained_cli++;
        }
    }
    return remained_cli;
}

/*
 * Function: Wait_clients_quit
 * -------------------
 *   This function is used to wait all clients quit
 *
 *   Parameters:
 *     remained_cli: the number of remained clients
 *
 *   Returns:
 *      void
 */

void wait_clients_quit(int remained_cli) {
    int i, j;
    while (remained_cli) {
        init_select();
        if (select_func() == -1) break;                 /* Select */
        for (i = 0; i < max_fds + 1; i++) {
            if (FD_ISSET(i, &rfds)) {
                client_ip = get_host_by_sockfd(i);              /* Get the client's ip address */
                printf("Wait for %s socket %d\n", client_ip, i);
                printf("Recv remained mark!\n");
                recv_socket_msg(i, recv_mark);                  /* Get the response message from client side */
                if (strcmp(recv_mark, DISCONNECT_SUCC_MSG) == 0 || strcmp(recv_mark, DISCONNECT_FAIL_MSG) == 0) {
                    get_time();
                    if (strcmp(recv_mark, DISCONNECT_SUCC_MSG) == 0)
                        fprintf(flog, "[%s] lyrebird client %s has disconnected expectedly.\n", out_time, client_ip);
                    if (strcmp(recv_mark, DISCONNECT_FAIL_MSG) == 0)
                        fprintf(flog, "[%s] lyrebird client %s has disconnected unexpectedly.\n", out_time, client_ip);
                    for (j = 0; j < CLIENT_MAXNUM; ++j) {               /* Close socket of this client */
                        if (sockfd_cli[j] == i) {
                            close(i);
                            sockfd_cli[j] = 0;
                            memset(ipaddr_cli[j], 0, sizeof(ipaddr_cli[j]));
                            break;
                        }
                    }
                    remained_cli--;
                    printf("Remained: %d\n", remained_cli);
                } else {
                    get_time();
                    fprintf(flog, "[%s] (Process ID #%d) ERROR: Server failed when checking message type from client machine!\n", out_time, getpid());
                    main_flag = EXIT_FAILURE;
                }
                break;
            }
        }
    }
}

/*
 * Function: Quit_server
 * -------------------
 *   This function is used to clean up everything and quit the server
 *
 *   Parameters:
 *      no parameters
 *
 *   Returns:
 *      void
 */

void quit_server(void) {
    get_time();                                             /* Print out the server information */
    printf("[%s] lyrebird.server: PID %d completed its tasks and is exiting successfully.\n", out_time, getpid());
    clean_up(CLEAN_ALL);                                    /* Always remember to free all and close file pointer! */
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

void clean_up(int step) {
    if (step >= CLEAN_TO_TXT) free(enc_txt);
    if (step >= CLEAN_TO_TXT) free(dec_txt);
    if (step >= CLEAN_TO_TIME) free(out_time);
    if (step >= CLEAN_TO_CONFIG) fclose(fcfg);
    if (step >= CLEAN_TO_LOG) fclose(flog);
    if (step >= CLEAN_TO_IFADDR) freeifaddrs(ifaddr);
    if (step >= CLEAN_TO_SOCKET) close(sockfd);
}

