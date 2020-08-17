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

#define PORT "11037"
#define BUF_SIZE 1024

#define FILENAME_LEN 128
#define FILESIZE_LEN 10

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
char* recvFile(int sockfd, char* header, size_t headerSize, size_t fileSize);

/**
 * Once the connecion is made, this function listens to
 * the header and the file.
 */
int handle_server(int serverSocket);

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
char* recvHeader(int sockfd, size_t* headerSize);

/**
 * Waits for someone to connect through sockfd.
 */
void listen_server(int sockfd);

/**
 * Creates the program's socket.
 * 
 * @return the socket's file descriptor
 */
int create_socket();

/**
 * 
 * */
static void* get_in_addr(struct sockaddr* sa);

static void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int create_socket() {
    int sockfd;
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

void listen_server(int sockfd) {
    int new_fd;
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
    char* ip = malloc(INET_ADDRSTRLEN * sizeof(char));
    inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr*)&their_addr),
                ip,  INET_ADDRSTRLEN*sizeof(char));

    printf("Hello, %s.\n", ip);
    
    if (handle_server(new_fd) != EXIT_SUCCESS)
        fprintf(stderr, "An error has occured.\n");

}

char* recvHeader(int sockfd, size_t* headerSize) {
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
    while (recvBytesNb < HEADER_LEN) {
        size_t msgSize = recv(sockfd, buffer, BUF_SIZE, 0);
        header = realloc(header, recvBytesNb + msgSize);
        
        // Copy of the buffer into the header
        for(size_t i = recvBytesNb; i < recvBytesNb+msgSize; i++)
            header[i] = buffer[i-recvBytesNb];

        recvBytesNb += msgSize;
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

char* recvFile(int sockfd, char* header, size_t headerSize, size_t fileSize) {
    char buffer[BUF_SIZE] = {0};
    char* rawfile = NULL;
    size_t recvBytesNb = 0, msgSize = 0;

    /* *
     * If headerSize > FILENAME_LEN+FILESIZE_LEN, then it means
     * that a part of the beginning of the file is contained
     * inside of header, and has to be put into "rawfile".
     * */
    
    if (headerSize > FILENAME_LEN+FILESIZE_LEN) { 
        recvBytesNb = headerSize-(FILENAME_LEN+FILESIZE_LEN);
        rawfile = malloc(recvBytesNb);
        for(size_t i = 0; i < recvBytesNb; i++) {
            rawfile[i] = header[FILENAME_LEN+FILESIZE_LEN+i];
        }
    }

    while ((msgSize = recv(sockfd, buffer, BUF_SIZE, 0)) != 0) {
        if (msgSize != -1) {
            rawfile = realloc(rawfile, recvBytesNb + msgSize);
            for(size_t i = 0; i < msgSize; i++)
                rawfile[recvBytesNb+i] = buffer[i];
            recvBytesNb += msgSize;

            if (recvBytesNb >= fileSize)
                break;
        }
    }

    if (recvBytesNb < fileSize) {
        fprintf(stderr, "ERROR: Transfer is incomplete, only %lu Bytes out of %lu received.\n", recvBytesNb, fileSize);
        free(rawfile);
        return NULL;
    }
    
    return rawfile;
}

int handle_server(int serverSocket) {
    // HEADER
    size_t headerSize = 0;

    fprintf(stderr, "Awaiting header...");
    char* header = recvHeader(serverSocket, &headerSize);

    char fileName[FILENAME_LEN+1];
    size_t fileSize;
    decode_fileName(header, fileName);
    decode_fileSize(header, &fileSize);
    fprintf(stderr, "OK!\n");

    // FILE
    fprintf(stderr, "Awaiting file...");
    char* rawfile = recvFile(serverSocket, header, headerSize, fileSize);

    if (!rawfile) {
        free(header);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "OK!\n");

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
    int sockfd;

    printf("Creating the client socket...");
    if ((sockfd = create_socket()) == -1) {
        fprintf(stderr, "unable to create the client socket\n");
        exit(1);
    }
    printf("OK !\n");

    printf("Client ready.\n");
    listen_server(sockfd);

    printf("Transfer completed.\n");

    return 0;
}