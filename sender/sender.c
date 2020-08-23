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
 * */

#include "sender.h"

int main(int argc, char* argv[])
{

    SOCKADDR_IN sin;
    File* f;
    char ip[16];
    char port[16];

    int args = parse_arguments(argc, argv, &f, ip, port);
    if(args == ERROR){
        return EXIT_FAILURE;
    }

    SOCKET sock = create_socket(AF_INET, SOCK_STREAM, 0, &sin, ip, port);
    if(sock == ERROR){
        fprintf(stderr, "an error occurred while creating the socket!\n");
        return EXIT_FAILURE;
    }

    int connection = start_connection(sock, sin);
    if(connection == ERROR){
        fprintf(stderr, "an error occurred while starting the connection!\n");
        return EXIT_FAILURE;
    }

    int header = send_header(sock, f);
    if(header == ERROR){
        fprintf(stderr, "an error occurred while sending the header!\n");
        return EXIT_FAILURE;
    }
    int message = send_message(sock, f);
    if(message == ERROR){
        fprintf(stderr, "an error occurred while sending the message!\n");
        return EXIT_FAILURE;
    }

    stop_connection(sock);
    free_file(f);
    
    return EXIT_SUCCESS;
}

int parse_arguments(int argc, char** argv, File** f, char* ip, char* port){

    assert(f != NULL);

    if(argc < NB_ARGS-1){
        fprintf(stderr, "usage : ./server -i [filename.extension] -a [ip] -p [port]\n");
        return ERROR;
    }

    const char *optstring = ":i:a:p:";
    int value;

    while((value = getopt(argc, argv, optstring)) != EOF){

        switch(value){

            case 'i':
                (*f) = open_file(argv[optind-1]);
                if(f == NULL) return ERROR;
            break;

            case 'a':
                strcpy((ip), argv[optind-1]);
            break;

            case 'p':
                strcpy((port), argv[optind-1]);
            break;

            default:
                fprintf(stderr, "usage : ./server -i [filename.extension] -a [ip] -p [port]\n");
                return ERROR;

        }
    }

    return SUCCESS;
}


unsigned long get_file_size(char* filename){

    assert(filename != NULL);

    struct stat st;

    stat(filename, &st);
    return st.st_size;
}

File* create_file(){

    File* f = malloc(sizeof(File));
    if(f == NULL) return NULL;
    return f;
}

File* open_file(char* filename){

    assert(filename != NULL);

    fprintf(stderr, "Opening file...");

    File* f = create_file();

    if(f == NULL){
        fprintf(stderr, "error: unable to create the file!\n");
        return NULL;
    }

    f->file = fopen(filename, "r");

    if(f->file == NULL){
        fprintf(stderr, "error: unable to open \"%s\"\n", filename);
        return NULL;
    }

    if(get_file_size(filename) > MAX_SIZE){
        fprintf(stderr, "error: file is too big!\n");
        return NULL;
    }

    sprintf(f->size, "%10lu", get_file_size(filename));

    if(strchr(filename, '/') != NULL)
        fix_name(filename);

    strcpy(f->name, filename);

    printf("OK!\n");

    return f;
}

SOCKET create_socket(int domain, int type, int protocol, SOCKADDR_IN* sin, char* ip, char* port){

    fprintf(stderr, "Creating socket...");

    assert(sin != NULL && ip != NULL && port != NULL);

    SOCKET sock = socket(domain, type, protocol); 
    if(sock == ERROR) return ERROR; 

    sin->sin_addr.s_addr = inet_addr(ip);
    sin->sin_family = domain;
    fprintf(stderr, "VOTRE PORT : %s", port);
    sin->sin_port = htons(atoi(port));

    printf("OK!\n");

    return sock;
}


int send_header(SOCKET sock, File* file){

    fprintf(stderr, "Sending Header...");

    if(send(sock, file->name, FILENAME_LEN, 0) != FILENAME_LEN)
       return ERROR;

    if(send(sock, file->size, FILESIZE_LEN, 0) != FILESIZE_LEN)
        return ERROR;

    printf("OK!\n");

    return SUCCESS;

}

int send_message(SOCKET sock, File* f){

    char buffer[BUF_SIZE], format[MAX_FORMAT_SIZE];

    double size = atol(f->size);

    unsigned rest = (int)size % BUF_SIZE;
    double totalSent = size - rest;

    for(double nbSent = 0; nbSent != totalSent; nbSent += BUF_SIZE){

        fprintf(stderr, "\rSending message...%d%%", (int)(((nbSent / size) * 100))+1);
        fflush(stdout);

        fscanf(f->file, "%1024c", buffer);
        if(send(sock, buffer, BUF_SIZE, 0) != BUF_SIZE)
            return ERROR;
    }

    sprintf(format, "%%%dc", rest);
    fscanf(f->file, format, buffer);
    if(send(sock, buffer, rest, 0) != rest)
        return ERROR;

    printf(" OK!\n");

    return SUCCESS;

}

int start_connection(SOCKET sock, SOCKADDR_IN sin){

    fprintf(stderr, "Connection to %s through the port %d...", inet_ntoa(sin.sin_addr), htons(sin.sin_port));
    
    int connectionState = connect(sock, (SOCKADDR*)&sin, sizeof(sin));
    if(connectionState == ERROR)
        return ERROR;

    printf("OK!\n");

    return SUCCESS;
}

void stop_connection(SOCKET sock){

    printf("Closing connection...");
    close(sock);
    printf("OK!\n");

}

void free_file(File* file){

    assert(file != NULL);

    fclose(file->file);
    free(file);
}

void fix_name(char* name){

    assert(name != NULL);

    const size_t stringSize = strlen(name);
    unsigned nbChars = 0;

    for(unsigned i = stringSize - 2; name[i] != '/'; i--){

        nbChars++;        
    }

    strcpy(name, name+(stringSize-nbChars-1));

}