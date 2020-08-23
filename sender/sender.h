#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef int SOCKET;

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define BUF_SIZE 1024

#define MAX_FORMAT_SIZE 7
#define NB_ARGS 8
#define MAX_SIZE 9999999999

#define FILENAME_LEN 128
#define FILESIZE_LEN 10

#define ERROR -1
#define SUCCESS 0


typedef struct{

    FILE* file; // the file itself
    char size[FILESIZE_LEN]; // the file size
    char name[FILENAME_LEN]; // the filename

}File;


/*
* return the total size of a file in bytes
*
* @return the size of the file called "filename" if success
* @return 0 else
*/
unsigned long get_file_size(char* filename);


/*
* allocate memory for a new file
*
* @return a pointer to the new file if success
* @return NULL else
*/
File* create_file();

/*
*
* opens and initialises the file called "filename"
*/
File* open_file(char* filename);


/*
* closes the file itself and frees the structure
*/
void free_file(File* file);


/*
* creates a new socket and configure it
*/
SOCKET create_socket(int domain, int type, int protocol, SOCKADDR_IN* sin, char* ip, char* port);


/*
* sends f's header using sock
*
* @return  0 if everyting went well
* @return -1 else
*/
int send_header(SOCKET sock, File* file);


/*
* sends f using sock
*
* @return  0 if everyting went well
* @return -1 else
*/
int send_message(SOCKET sock, File* f);


/*
* starts the connection
*
* @return  0 if everyting went well
* @return -1 else
*/
int start_connection(SOCKET sock, SOCKADDR_IN sin);


/*
* stops the connection
*/
void stop_connection(SOCKET sock);

/*
* parses command line arguments given to the program
*/
int parse_arguments(int argc, char** argv, File** f, char* ip, char* port);

/*
*
* fixes a file name that is actually a path (example : /home/user/test.txt becomes test.txt)
*
*/
void fix_name(char* name);