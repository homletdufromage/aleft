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
#include <string.h>
#include "receiver.h"

int parse_arguments(int argc, char** argv, char* port){;

    if(argc < 3){
        fprintf(stderr, "usage : %s -p [PORT NUMBER]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *optstring = ":p";
    int value;

    while((value = getopt(argc, argv, optstring)) != EOF){

        switch(value){

            case 'p':
                strcpy(port, argv[optind]);
                break;

            default:
                fprintf(stderr, "usage : %s -p [PORT NUMBER]\n", argv[0]);
                return EXIT_FAILURE;

        }
    }

    return EXIT_SUCCESS;
}


int main(int argc, char const *argv[])
{
    char PORT[6] = {0};

    if(parse_arguments(argc, (char**) argv, PORT) == EXIT_FAILURE)
        return EXIT_FAILURE;
    SOCKET sockfd;

    printf("Creating the receiver socket...");
    if ((sockfd = create_socket(PORT)) == -1) {
        fprintf(stderr, RED"Error:"RESET" Unable to create the receiver socket\n");
        exit(1);
    }
    printf(GRN "OK !\n");

    printf("Listening...\n"RESET);
    SOCKET new_sockfd;
    if ((new_sockfd = listen_sender(sockfd)) == -1)
        fprintf(stderr, RED "Error: " RESET "the connection couldn't be made.\n");
    
    if (start_transfer(new_sockfd) == EXIT_SUCCESS)
        printf(GRN "Transfer completed successfully.\n" RESET);
    else
        printf(RED "\nError: " RESET "File not received.\n");

    return 0;
}