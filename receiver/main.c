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
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "receiver.h"

int main(int argc, char const *argv[])
{
    SOCKET sockfd;

    printf("Creating the receiver socket...");
    if ((sockfd = create_socket()) == -1) {
        fprintf(stderr, "unable to create the receiver socket\n");
        exit(1);
    }
    printf(GRN "OK !\n" RESET);

    printf("Listening...\n");
    SOCKET new_sockfd;
    if ((new_sockfd = listen_sender(sockfd)) == -1)
        fprintf(stderr, RED "Error: " RESET "the connection couldn't be made.\n");
    
    if (start_transfer(new_sockfd) == EXIT_SUCCESS)
        printf(GRN "Transfer completed successfully.\n" RESET);
    else
        printf(RED "Error: " RESET "The transfer could not be completed.\n");

    return 0;
}