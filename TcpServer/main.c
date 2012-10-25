/* 
 * File:   main.c
 * Author: burgc5
 *
 * Created on 24 October 2012, 15:20
 */

#include "common.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT "20911"
#define BUFFER_SIZE 128

void *handlerThread(void *tcpSocket) {
    char hello[] = "Thank you for registering\n";
    int returnCode;
    char buffer[BUFFER_SIZE];
    
    SOCKET socket = *(SOCKET *) tcpSocket;
    
    send(socket, hello, strlen(hello), 0);
    while (1) {
        returnCode = recv(socket, buffer, sizeof (buffer), 0);
        if (returnCode <= 0 || buffer[0] == '\n') {
            break;
        }
        send(socket, hello, strlen(hello), 0);
    }
    
    return NULL;
}


/*
 * 
 */
int main(int argc, char** argv) {

    struct sockaddr address;
    int addressLength = sizeof (address);
    pthread_t workerThread;

    SOCKET s, ss;
    WSAinit();
    ss = passiveTCP(PORT, 5);

    s = accept(ss, &address, &addressLength);
    
    pthread_create(&workerThread, NULL, handlerThread, (void *) &s);
    
    pthread_join(workerThread, NULL);
    
//    send(s, hello, strlen(hello), 0);
//    while (1) {
//        returnCode = recv(s, buffer, sizeof (buffer), 0);
//        if (returnCode <= 0 || buffer[0] == '\n') {
//            break;
//        }
//        send(s, buffer, returnCode, 0);
//    }
    closesocket(ss);

    return (EXIT_SUCCESS);
}

