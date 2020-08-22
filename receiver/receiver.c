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

#include "receiver.h"

void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

SOCKET create_socket(const char* PORT) {
    SOCKET sockfd;
    struct addrinfo hints, *rcvInfo, *p;
    int rv;

    // Initialize the socket
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // SOCK_STREAM (TCP) or SOCK_DGRAM (UDP)
    hints.ai_flags = AI_PASSIVE; // use my IP

    /**
     * NULL because we don't connect anywhere, we let the sender connect to us
     * PORT is the used port
     * &hints address of the socket initialization parameters
     * &rcvInfo pointer to the address of the addrinfo struct to initialize with a linked list.
     * 
     * rcvInfo will contain a linked list of structs containing
     * an Internet address that can be specified in a call to bind() or connect().
     * */
    if ((rv = getaddrinfo(NULL, PORT, &hints, &rcvInfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for (p = rcvInfo; p != NULL; p = p->ai_next) {
        /**
         * Creates the socket file descriptor thanks to the current node
         * */
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket()");
            continue;
        }

        /**
         * Allows to lose the boring "Address already in use" error message
         * by specifying the OS that this program is allowed to reuse the port
         * */
        int yaaas = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yaaas, sizeof(int)) == -1) {
            perror("setsockopt()");
            return -1;
        }

        /**
         * Binds the socket to one of the computer's ports, so the kernel can match an incoming
         * packet to a certain process' socket descriptor.
         * */
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("bind()");
            continue;
        }

        break;
    }

    freeaddrinfo(rcvInfo);

    if (!p)
        return -1;

    return sockfd;
}

SOCKET listen_sender(SOCKET sockfd) {
    SOCKET new_fd;
    struct sockaddr_storage their_addr;

    if (listen(sockfd, 1) == -1) {
        perror("listen");
        return -1;
    }

    socklen_t sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
    if (new_fd == -1) {
        fprintf(stderr, "Could not accept()\n");
        return -1;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr*)&their_addr),
                ip,  INET_ADDRSTRLEN*sizeof(char));
    printf("New connection from %s\n", ip);

    return new_fd;
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

    printf("Awaiting file...0%% (0/0 B received)");
    fflush(stdout);
    while ((msgSize = recv(sockfd, buffer, BUF_SIZE, 0)) != 0) {
        if (msgSize != -1) {
            *rawfile = realloc(*rawfile, recvBytesNb + msgSize);
            for(size_t i = 0; i < msgSize; i++)
                (*rawfile)[recvBytesNb+i] = buffer[i];
            recvBytesNb += msgSize;

            fprintf(stderr, "\r");
            float recvStr = (float) recvBytesNb, sizeStr = (float) fileSize;
            char unit[3];
            strcpy(unit, "B\0");
            if (recvBytesNb > 1000 && recvBytesNb < 1000000) {
                recvStr /= 1000.0;
                sizeStr /= 1000.0;
                strcpy(unit, "KB\0");
            } else if (recvBytesNb >= 1000000 && recvBytesNb < 1000000000) {
                recvStr /= 1000000.0;
                sizeStr /= 1000000.0;
                strcpy(unit, "MB\0");
            } else if (recvBytesNb >= 1000000000) {
                recvStr /= 1000000000.0;
                sizeStr /= 1000000000.0;
                strcpy(unit, "GB\0");
            }
            printf("Awaiting file...%0.f%% (%.2f/%.2f %s received)", ((float)recvStr/(float)sizeStr)*100.0, recvStr, sizeStr, unit);
            fflush(stdout);

            if (recvBytesNb >= fileSize)
                break;
        }
    }

    if (recvBytesNb < fileSize) {
        fprintf(stderr, RED "\nError: " RESET "Transfer is incomplete, only %lu Bytes out of %lu received.\n", recvBytesNb, fileSize);
        if (*rawfile) {
            free(*rawfile);
            *rawfile = NULL;
        }
    }
}

int copy_str_to_file(char* rawfile, char* fileName, size_t fileSize) {
    // SAVING THE FILE
    FILE* file = fopen(fileName, "w");
    if (!file)
        return EXIT_FAILURE;

    for(size_t i = 0; i < fileSize; i++)
        if (fprintf(file, "%c", rawfile[i]) < 0) {
            fclose(file);
            fprintf(stderr, RED "Error: " RESET "an error has occured during the saving of the file.\n");
            return EXIT_FAILURE;
        }
    
    fclose(file);
    return EXIT_SUCCESS;
}

int start_transfer(SOCKET senderSocket) {
    // Receiving the header first
    size_t headerSize = 0;

    printf("Awaiting header...");
    char* header = recvHeader(senderSocket, &headerSize);
    if (!header) {
        fprintf(stderr, RED"\nError: "RESET"an error has occured during the header transfer.\n");
        close(senderSocket);
        return EXIT_FAILURE;
    }

    // Checking the header format
    if (!check_header(header, header+FILENAME_LEN)){
        fprintf(stderr, RED"\nError: "RESET"wrong header format.\n");
        free(header);
        return EXIT_FAILURE;
    }

    // Decoding header
    char fileName[FILENAME_LEN+1];
    size_t fileSize;
    decode_fileName(header, fileName);
    decode_fileSize(header, &fileSize);
    printf(GRN"OK!\n"RESET);

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

    // Receiving the file
    recvFile(senderSocket, &rawfile, recvBytesNb, fileSize);
    close(senderSocket);
    free(header);

    if (!rawfile) {
        return EXIT_FAILURE;
    }
    printf(GRN" OK!\n"RESET);

    int status = copy_str_to_file(rawfile, fileName, fileSize);
    free(rawfile);

    if (status != EXIT_SUCCESS)
        fprintf(stderr, "The file couldn't be saved.\n");

    return status;
}