#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define PORT 11037
#define IP "127.0.0.1"
#define BUF_SIZE 1024

#define FILENAME "/home/g/Desktop/charlier.png"
#define OUTPUT_NAME "output.png"
#define FILENAME_LEN 128
#define FILESIZE_LEN 10
 
typedef struct{

    FILE* file;
    char size[FILESIZE_LEN];
    char name[FILENAME_LEN];
}File;

int main(void)
{
 
    SOCKET sock;
    SOCKADDR_IN sin;
    struct stat st;
    long unsigned size;
    char empty[FILENAME_LEN] = {0};

    printf("Opening file...");
    File* f = malloc(sizeof(File));
    if(f == NULL){
        perror("unable to create file");
        return EXIT_FAILURE;
    }

    f->file = fopen(FILENAME, "r");
    if(f->file == NULL){
        perror("unable to open file");
        return EXIT_FAILURE;
    }
    strcpy(f->name, empty);
    strcpy(f->name, OUTPUT_NAME);
    stat(FILENAME, &st);
    size = st.st_size;
    if(size > 999999999){
        perror("your file is too big!\n");
        return EXIT_FAILURE;
    }
    sprintf(f->size, "%10lu", size);
    printf("OK!\n");

    printf("Creating socket...");
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET){
        perror("Unable to create socket\n");
        return EXIT_FAILURE;
    }
    printf("OK!\n");

    sin.sin_addr.s_addr = inet_addr(IP);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);

    printf("Connection to %s through the port %d...", inet_ntoa(sin.sin_addr), htons(sin.sin_port));
    if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) != SOCKET_ERROR){

        printf("OK!\n");

        printf("Sending Header...");
        send(sock, f->name, 128, 0);
        send(sock, f->size, 10, 0);
        printf("OK!\n");

        printf("Sending message...");
        for(unsigned int i; i < size; i++){
            char c;
            fscanf(f->file, "%c", &c);
            send(sock, &c, 1, 0);
        }
        printf("OK!\n");

    }

    else
        printf("\nUnable to connect to %s\n", inet_ntoa(sin.sin_addr));


    printf("Closing connection...");
    fclose(f->file);
    free(f);
    close(sock);
    printf("OK!\n");
 
    return EXIT_SUCCESS;
}