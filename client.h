//
// Ahmet Tunahan Özkan - client.c için Header dosyası

#ifndef client_h
#define client_h

#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<ctype.h>
#include<signal.h>

char ip_address[] = "127.0.0.1";
int port_number = 80;
int status = 0;
int socket_desc;
struct sockaddr_in server;
char username[20];
char password[20];
char user_token[41];
int token = 12;
int message_limit = 1024+1;

void *receiver_handler(void *socket);


#endif