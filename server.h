//
// Ahmet Tunahan Özkan - server.c için Header dosyası

#ifndef server_h
#define server_h

#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<signal.h>

int port_number = 5001;
struct connection_t
{
    int p_sock;
    int status;
    int user;
    char username[50];
    struct connection_t *next;
};

void *connection_handler(void *);
void *receiver_handler(void *);
struct connection_t *head;
struct connection_t *tail;
int open_connections = 0;
void add_socket(struct connection_t *socket);
void delete_socket(struct connection_t *socket);
void broadcast(struct connection_t *socket, char *message, int type);


#endif