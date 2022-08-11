
/*
Ahmet Tunahan Özkan - 20120205041
Derlemek için:
gcc server.c -lpthread
./a.out 5001

Server'a bağlananları görmek için:
Server'a bağlandıktan sonra ":get_clients" yazınız 

Broadcast kullanılmıştır (UDP)
*/

#include"server.h"
int flag = 0;
char online_list[200];

int main(int argc , char *argv[]) {
    if (argc <= 1) {
        printf("ERROR: Şu formatta yazınız: ./server 5001\n");
        return 1;
    }
    if (argc > 2) {
        printf("ERROR: Şu formatta yazınız: ./server 5001)\n");
        return 1;
    }

    port_number = atoi(argv[1]);
    if (port_number < 1024) {
        printf("ERROR: Geçersiz port numarası\n");
        return 1;
    }

    int socket_desc , new_socket , c , *new_sock;
    struct sockaddr_in server , client;

    //Socket oluşturma
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( port_number );

    //Bind İşlemi
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
        puts("bind failed");
        return 1;
    }


    printf("======== You have entered The Chat Room ========\n*** token:12 ***\n");

    //Listen (Hazırlık)
    listen(socket_desc , 5);

    //Gelen Bağlantıları Kabul etme
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) {

        struct connection_t *new_connection = (struct connection_t *) (struct connection *) malloc(
                sizeof(struct connection_t));
        new_connection->p_sock = new_socket;
        new_connection->status = 1;
        new_connection->user = 0;
        add_socket(new_connection);

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;

        pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_connection);

    }

    if (new_socket<0) {
        perror("accept failed");
        return 1;
    }

    return 0;
}

//Her client için bağlantıları ayarlama
void *connection_handler(void *p_connection) {
    struct connection_t *connection = (struct connection_t*)p_connection;
    int dummy = 0;

    pthread_t receiving_thread;

    if( pthread_create( &receiving_thread , NULL ,  receiver_handler , (void*) p_connection) > 0) {
        perror("receving thread");
        return 0;
    }

    while(connection->status) {
        dummy++;
    }

    //Socket pointer free etme
    broadcast(p_connection, "", 2);
    delete_socket(p_connection);
    free(p_connection);

    return 0;
}


void *receiver_handler(void *p_connection) {
    struct connection_t *connection = (struct connection_t*)p_connection;
    //socket descriptor için
    int sock = connection->p_sock;
    char username[20];
    char token[41];

    int read_size;
    char client_message[2000];
    char processed_client_message[2000];

    //Client'den mesajı alma
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 ) {
        if (!connection->user) {
            strcpy(token, client_message);

            char ch = token[0];
            int i = 0;
            while (ch != '\n' && ch != '\0') {
                username[i] = ch;
                i++;
                ch = token[i];
            }
            username[i] = '\0';  //kullanıcı adını ayarlama        

             for(int i=0;i<strlen(username);i++)    //Şifreli gelen username'i çözüyor (Decrypt)
            {
               username[i] = username[i] - 12;
            } 

            strcpy(connection->username, username); //Çözülen isim


            for(int i = 0; i<=strlen(username);i++) //Server'a Bağlanan Clientlerin Listesi için
            {
                online_list[flag] = username[i];
                
                if(username[i] == '\0')
                {
                    online_list[flag] = ',';
                    flag++;
                    break;
                }
                flag++;
            }
            

            printf(connection->username);
            printf(" has joined\n");

            broadcast(connection, "", 1);

            connection->user = 1;

        } else {
            int ie = 0;  
            for(int i = 0;i<strlen(client_message);i++)     //Client'in attığı şifreli mesajı çözmek (Decrypt)
            {
                client_message[i] = client_message[i] - 12;
            }                      
            for (int i = 0; i <= strlen(client_message); i++) {
                if (client_message[i] == '\n') {
                    client_message[i] = '\0';
                }
                
                processed_client_message[i] = client_message[i] ;   //Çözülen Mesaj
                ie = i;
            }
            
            if(strcmp(":get_clients",processed_client_message) == 0)    // Server'a Bağlananların listesi için
            {                                                          // :get_clients yazılmalı
                printf("%s: %s\n", connection->username, processed_client_message);
                printf("Clients = %s\n", online_list);
            }
            else
            {   //:get_clients haricinde yazılanlar serverda normal mesaj olarak gösteriliyor 
                printf("%s: %s\n", connection->username, processed_client_message); 
            }
            

            processed_client_message[ie] = '\n';
            processed_client_message[ie+1] = '\0';

            broadcast(connection, processed_client_message, 0);
        }
    }

    if(read_size == 0) {
        printf(connection->username);
        printf(" has left\n");

        fflush(stdout);

    } else if(read_size == -1) {
        perror("recv failed");
    }

    connection->status = 0;

    return 0;
}

//socket ekleme
void add_socket(struct connection_t *socket) {
    if (open_connections == 0) {
        socket->next = NULL;
        head = socket;
        tail = socket;
        open_connections++;

    } else if (open_connections > 0) {
        tail->next = socket;
        socket->next = NULL;
        tail = socket;
        open_connections++;
    }
}
//socket silme
void delete_socket(struct connection_t *socket) {
    if (open_connections == 1 && socket == head) {
        head = NULL;
        open_connections--;

    } else if (socket == head) {
        head = head->next;
        open_connections--;

    } else if (open_connections > 1) {

        struct connection_t *currentSocket = head;
        while (currentSocket->next != socket && currentSocket->next != NULL) {
            currentSocket = currentSocket->next;
        }

        if (tail == socket) {
            tail = currentSocket;
            currentSocket->next = NULL;

        } else {
            currentSocket->next = currentSocket->next->next;

        }
        open_connections--;
    }
}

//broadcast: serverda'ki her client mesajı alıyor, aynı broadcast'daki (UDP)
void broadcast(struct connection_t *socket, char *message, int type) {
    char signed_message[strlen(socket->username) + 2 + strlen(message)];
    char signed_message2[strlen(socket->username) + 2 + strlen(message)];
    int size = strlen(socket->username) + 2;
    strcpy(signed_message, socket->username);
    strcpy(signed_message2, signed_message);
    strcat(signed_message2, message);

    if (type == 0) {        //message type

        char get_clients[] = ":get_clients\n";

         if (strcmp(get_clients, message) == 0) {   //online list istendiğinde
            strcat(signed_message, "'s Request Replied by Server\n");
            size += strlen("'s Request Replied by Server\n"); 
        }
        else {                                    //online list harici
            strcat(signed_message, ": ");
            strcat(signed_message, message);
            size += strlen(message);
        }
        strcat(signed_message, "\0");

    } else if (type == 1) {         //Log in type
        strcat(signed_message, " has joined\n");
        size += strlen(" has joined\n");

    } else if (type == 2) {         //Log out type
        strcat(signed_message, " has left\n");
        size += strlen(" has left\n");
    }

    struct connection_t *currentSocket = head;
    int i = 0;
    while (i < open_connections) {

        if(socket != currentSocket) {
            write(currentSocket->p_sock, signed_message , size);
        }
        currentSocket = currentSocket->next;
        i++;
    }
}
