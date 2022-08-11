
/*
Ahmet Tunahan Özkan - 20120205041
Derlemek için:
gcc client.c -lpthread
./a.out 5001 OXXN HCXU 

Server'a bağlananları görmek için:
Server'a bağlandıktan sonra ":get_clients" yazınız 
*/

#include <sys/stat.h>
#include"client.h"

int main(int argc , char *argv[]) {
    if (argc <= 3) {
        printf("ERROR!"
                 "(Şu formatta yazınız: ./client 5001 user1 xxxx)\n");
        return 1;
    }
    if (argc > 4) {
        printf("ERROR!"
                 "(Şu formatta yazınız: ./client 5001 user1 xxxx)\n");
        return 1;
    }

    port_number = atoi(argv[1]); //port number 1024'ten küçükse hata verir
    if (port_number < 1024) {
        printf("ERROR: Geçersiz port numarası\n");
        return 1;
    }

    if (strlen(argv[2]) > 20) {
        printf("ERROR: Kullanıcı adı 20 karakterden fazla olamaz\n");
        return 1;
    }

    if (strlen(argv[3]) > 20) {
        printf("ERROR: Şifre 20 karakterden fazla olamaz\n");
        return 1;
    }
    strcpy(username, argv[2]); 
    strcpy(password, argv[3]);
    char ch = username[0];
    int i = 0;

    FILE *file_pointer = fopen("users.txt","r"); //users.txt dosyasını 2 kere açıyoruz
    FILE *file_pointer2 = fopen("users.txt","r"); //isim ve şifre kontrolü için
    if(!file_pointer)
    {
        perror("fopen");
        return 5;
    }

    char last[20];  
    while (fscanf(file_pointer2,"%s",last) != EOF){} //dosyadaki son veriyi kaydediyor 

    char name[20];
    char pass[20];

    while (fscanf(file_pointer,"%s %s",name,pass) != EOF)   //kelimesi kelimesine name ve pass'i dosya sonuna kadar tara
    {                                                       // 2 column olduğu için 2 tane %s
        if(strcmp(name,username) == 0 && strcmp(pass,password) == 0)    //verilen argümanlar users.txt'dekilerle uyuşuyorsa
        {                                                               //Giriş yapılır
            printf("login success\n");
            break;                          //username ve password uyuşuyorsa break
        }
        else if(strcmp(last,pass) == 0)    //eğer dosyanın sonuna kadar gelmişse ad veya şifre yanlıştır
        {                                  //(dosyanın sonundaki kullanıcı verileri doğru yazmışsa break sayesinde girer)
            printf("LOGIN ERROR: Kullanıcı adı veya şifre yanlış\n");
            return 1;      
        }

    }
    //açılan dosyaları kapatma
    fclose(file_pointer); 
    fclose(file_pointer2);

    //socket oluşturma
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1) {
        printf("ERROR: Socket oluşturulamıyor");
    }

    //ip address ve port number veriliyor
    server.sin_addr.s_addr = inet_addr(ip_address);
    server.sin_family = AF_INET;
    server.sin_port = htons(port_number);

    //Server'a Bağlanma
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0) {
        puts("CONNECT ERROR");
        return 1;
    }

    status = 1;
    printf("======== You have entered The Chat Room ========\n*** token:12 ***\n"); //token Sayısını önceden 12 diye belirledim
                                                                            //12'ye göre şifrelencek server'a gönderilmeden önce
    pthread_t receiving_thread;
    int new_socket = socket_desc;
    int *new_sock;
    new_sock = malloc(1);
    *new_sock = new_socket;
    //thread oluşturma her bir socket için
    pthread_create( &receiving_thread , NULL ,  receiver_handler , (void*) new_sock);

    //username şifreleme (encrypt işlemi)
    for(int i=0;i<strlen(username);i++)
    {
        username[i] = username[i] + 12;
    }
 
    strcpy(user_token, username);
    strcat(user_token, "\n");

    //Kullanıcı adını şifrelenmeiş şekilde server'a gönderme
    if( send(socket_desc , user_token , strlen(user_token) , 0) < 0) {
        puts("ERROR: Log in failed");
        return 1;
    }

    while (status) {
        char str[message_limit];
        char strEnc[message_limit];
        fgets(str, message_limit+2, stdin);

        for(int i=0;i<strlen(str);i++)  //Client'in yazdığı mesajları şifreleme (Encrypt)
        {
            strEnc[i] = str[i] + 12;
        }

        if(!status) {
            return 0;
        }
        if (str[0] == '\0' || str[0] == '\n') {     
            printf("ERROR: Boş mesaj gönderilemez!\n");
            fflush(stdin);

        } else {
            fflush(stdin);
            if (strlen(str) > message_limit) {
                printf("ERROR: your message cannot exceed 1024 characters\n");

            } else {    
                if( send(socket_desc , strEnc , strlen(str) , 0) < 0) {     //Client'in mesajı Şifrelenmiş biçimde Server'a gönderildi
                    puts("ERROR: Send failed");
                    return 1;
                }
            }
        }
    }
    return 0;
}


void *receiver_handler(void *socket) {
    //socket desc'i al
    int sock = *(int*)socket;
    int read_size;
    char server_message[2000];
    char processed_server_message[2000];

    //Client'ten mesajı receive
    while( (read_size = recv(sock , server_message , 2000 , 0)) > 0 ) {
        int ie = 0;
        for (int i = 0; i <= strlen(server_message); i++) {
            if (server_message[i] == '\n') {
                server_message[i] = '\0';
            }
            processed_server_message[i] = server_message[i];
            ie = i;
        }
        printf(processed_server_message);
        printf("\n");
        processed_server_message[ie] = '\n';
    }

    if(read_size == 0) {
        printf("Server disconnected\n");
        exit(0);
    } else if(read_size == -1) {
        perror("ERROR: recv failed");
    }

    status = 0;

    //Socket pointer free etme
    free(socket);

    return 0;
}

