/* 
 * File:   main.c
 * Author: burgc5
 *
 * Created on 24 October 2012, 15:20
 */

#include "common.h"
#include <stdio.h>
#include <stdlib.h>

#define PORT "20911"
#define BUFFER_SIZE 128

/*
 * 
 */
int main(int argc, char** argv) {

    struct sockaddr address;
    int returnCode, addressLength = sizeof(address);
    char hello[] = "Hello dear client, I am an echo server\n";
    char buffer[BUFFER_SIZE];
    
    SOCKET s, ss;
    WSAinit();
    ss = passiveTCP(PORT, 5);
    while (1) {
        s = accept(ss, &address, &addressLength);
        send(s, hello, strlen(hello), 0);
        while (1) {
            returnCode = recv(s, buffer, sizeof(buffer), 0);
            if (returnCode <= 0) {
                break;
            }
            send(s, buffer, returnCode, 0);
        }
        closesocket(s);
    }
    
    return (EXIT_SUCCESS);
}

