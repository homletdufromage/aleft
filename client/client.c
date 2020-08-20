/**
 * ALEFT PROJECT
 * 
 * @author Alexandre E.
 * @author Lev M.
 * @date August 2020
 * 
 * @note This program is a part of the ALEFT Project.
 *       It's a naive file transfert program, which allows
 *       two users to transfer a file to each other.
 * 
 *       This program simply listens through the port 11037.
 * */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT "11037"
#define BUF_SIZE 1024

#define FILENAME_LEN 128
#define FILESIZE_LEN 10

typedef int SOCKET;

/*
MESSAGE STRUCTURE :
[HEADER]
    FILENAME_LEN Bytes of filename
        e.g. "h e l l o . t x t \0 ... \0 "
              1 2 3 4 5 6 7 8 9 10 ... 128
    
    FILESIZE_LEN Bytes of file size
        e.g. "      2048" = 2048 Bytes

[FILE CONTENT]
    Everything that follows the header is the sent file.
    The number of bytes from this section is known
    thanks to the size written in the HEADER.
    (e.g. 2048 Bytes according to the above example)
*/

/**
 * Listens to sockfd to receive the file
 * 
 * @arg sockfd: server's socket descriptor
 * @arg fileSize: size of the file
 */
void recvFile(SOCKET sockfd, char** rawfile, size_t recvBytesNb, size_t fileSize);

/**
 * Once the connecion is made, this function listens to
 * the header and the file.
 */
int handle_server(SOCKET serverSocket);

/**
 * Puts the file size from the header into size
 */
void decode_fileSize(char* header, size_t* size);

/**
 * Puts the filename from the header into fileName
 * 
 */
void decode_fileName(char* header, char* fileName);

/**
 * Checks the format of the header
 */
bool check_header(char* filename, char* fileSizeStr);

/**
 * Receives the file header.
 * 
 * STRUCTURE OF THE HEADER :
 * FILENAME_LEN Bytes of file name followed by FILESIZE_LEN of file size.
 * The filename uses a padding of '\0' after the end of the name
 * The file size is a string where the size is written in decimal form
 * after a series of whitespaces (e.g. "       128" = 128 Bytes)
 * 
 * @arg sockfd : socket to listen to
 * @arg headerSize : address of a variable in which to write the size
 * 
 * @return the received header
 */
char* recvHeader(SOCKET sockfd, size_t* headerSize);

/**
 * Waits for someone to connect through sockfd.
 */
int listen_server(SOCKET sockfd);

/**
 * Creates the program's socket.
 * 
 * @return the socket's file descriptor
 */
SOCKET create_socket();

/**
 * 
 * */
static void* get_in_addr(struct sockaddr* sa);

static void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

SOCKET create_socket() {
    SOCKET sockfd;
    struct addrinfo hints, *cliInfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &cliInfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = cliInfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(cliInfo);

    if (!p)
        return -1;

    return sockfd;
}

int listen_server(SOCKET sockfd) {
    SOCKET new_fd;
    struct sockaddr_storage their_addr;

    if (listen(sockfd, 1) == -1) {
        perror("listen");
        exit(1);
    }

    socklen_t sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
    if (new_fd == -1) {
        fprintf(stderr, "Connexion errors\n");
    }

    // Gets the client's IP address in string format
    char ip[INET_ADDRSTRLEN];
    inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr*)&their_addr),
                ip,  INET_ADDRSTRLEN*sizeof(char));

    printf("Hello, %s\n", ip);

    return handle_server(new_fd);
}

char* recvHeader(SOCKET sockfd, size_t* headerSize) {
    const size_t HEADER_LEN = FILENAME_LEN + FILESIZE_LEN;

    size_t recvBytesNb = 0;

    char* header = NULL;
    char buffer[BUF_SIZE];

    /*
     * The file header can arrive in several pieces smaller than HEADER_LEN
     * Bytes. So we have to fill a buffer with every part until
     * the whole header has arrived.
     * Part of the beginning of the file may also be received with the header,
     * and has to be taken into account.
     */
    bool disconnected = false;
    while (recvBytesNb < HEADER_LEN) {
        int msgSize = recv(sockfd, buffer, BUF_SIZE, 0);
        if (msgSize <= 0) {
            disconnected = true;
            break;
        }
        header = realloc(header, recvBytesNb + msgSize);
        
        // Copy of the buffer into the header
        for(size_t i = recvBytesNb; i < recvBytesNb+msgSize; i++)
            header[i] = buffer[i-recvBytesNb];

        recvBytesNb += msgSize;
    }
    if (disconnected) {
        if (header)
           free(header);
        return NULL;
    }
    *headerSize = recvBytesNb;

    return header;
}

void decode_fileName(char* header, char* fileName) {
    strcpy(fileName, header);
}

void decode_fileSize(char* header, size_t* size) {
    char fileSizeStr[FILESIZE_LEN+1];
    strncpy(fileSizeStr, header+FILENAME_LEN, FILESIZE_LEN);
    fileSizeStr[FILESIZE_LEN] = 0;

    *size = strtol(fileSizeStr, NULL, 10);
}

bool check_header(char* filename, char* fileSizeStr) {
    bool zeroEncountered = false;
    for(int i = 0; i < FILENAME_LEN; i++) {
        if (filename[i] == '\0')
            zeroEncountered = true;
        if (zeroEncountered)
            if (filename[i] != '\0')
                return false;
    }

    bool numberEncountered = false;
    for(int i = 0; i < FILESIZE_LEN; i++) {
        if (fileSizeStr[i] != ' ')
            numberEncountered = true;
        if (numberEncountered)
            if (fileSizeStr[i] < '0' || fileSizeStr[i] > '9')
                return false;
    }

    return true;
}

void recvFile(SOCKET sockfd, char** rawfile, size_t recvBytesNb, size_t fileSize) {
    char buffer[BUF_SIZE] = {0};
    int msgSize = 0;

    while ((msgSize = recv(sockfd, buffer, BUF_SIZE, 0)) != 0) {
        if (msgSize != -1) {
            *rawfile = realloc(*rawfile, recvBytesNb + msgSize);
            for(size_t i = 0; i < msgSize; i++)
                (*rawfile)[recvBytesNb+i] = buffer[i];
            recvBytesNb += msgSize;

            fprintf(stderr, "\r");
            fprintf(stderr, "Awaiting file...%0.f%%", ((float)recvBytesNb/(float)fileSize)*100.0);
            fflush(stderr);

            if (recvBytesNb >= fileSize)
                break;
        }
    }

    if (recvBytesNb < fileSize) {
        fprintf(stderr, "\nERROR: Transfer is incomplete, only %lu Bytes out of %lu received.\n", recvBytesNb, fileSize);
        if (*rawfile) {
            free(*rawfile);
            *rawfile = NULL;
        }
    }
}

int handle_server(SOCKET serverSocket) {
    // HEADER
    size_t headerSize = 0;

    fprintf(stderr, "Awaiting header...");
    char* header = recvHeader(serverSocket, &headerSize);
    if (!header) {
        fprintf(stderr, "\nERROR: an error has occured during the header transfer.\n");
        close(serverSocket);
        return EXIT_FAILURE;
    }

    // Checking header format
    if (!check_header(header, header+FILENAME_LEN)){
        fprintf(stderr, "\nERROR: wrong header format.\n");
        free(header);
        return EXIT_FAILURE;
    }

    char fileName[FILENAME_LEN+1];
    size_t fileSize;
    decode_fileName(header, fileName);
    decode_fileSize(header, &fileSize);
    fprintf(stderr, "OK!\n");

    /* *
     * If headerSize > FILENAME_LEN+FILESIZE_LEN, then it means
     * that a part of the beginning of the file is contained
     * inside of header, and has to be put into "rawfile".
     * */
    char* rawfile = NULL;
    size_t recvBytesNb = 0;
    if (headerSize > FILENAME_LEN+FILESIZE_LEN) { 
        recvBytesNb = headerSize-(FILENAME_LEN+FILESIZE_LEN);
        rawfile = malloc(recvBytesNb);
        for(size_t i = 0; i < recvBytesNb; i++) {
            rawfile[i] = header[FILENAME_LEN+FILESIZE_LEN+i];
        }
    }

    recvFile(serverSocket, &rawfile, recvBytesNb, fileSize);

    if (!rawfile) {
        free(header);
        return EXIT_FAILURE;
    }
    fprintf(stderr, " OK!\n");

    // SAVING THE FILE
    FILE* file = fopen(fileName, "w");
    for(size_t i = 0; i < fileSize; i++)
        fprintf(file, "%c", rawfile[i]);
    
    fclose(file);
    free(rawfile);
    free(header);
    close(serverSocket);
    
    return EXIT_SUCCESS;
}

int main(int argc, char const *argv[])
{
    SOCKET sockfd;

    printf("Creating the client socket...");
    if ((sockfd = create_socket()) == -1) {
        fprintf(stderr, "unable to create the client socket\n");
        exit(1);
    }
    printf("OK !\n");

    printf("Client ready.\n");
    if (listen_server(sockfd) == EXIT_SUCCESS)
        printf("Transfer completed.\n");
    else
        printf("Sorry, the transfer has not been completed.\n");

    return 0;
}