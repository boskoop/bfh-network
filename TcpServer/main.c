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

#define SERVER_PORT "20911"
#define SHUTDOWN_PORT "20912"
#define BUFFER_SIZE 128

static int runFlag = 1;

void *shutdownHookThread(void *p) {
    struct sockaddr address;
    int addressLength = sizeof (address);
    char shutdown[] = "shutdown\n";
    char okMessage[] = "initializing shutdown\n";
    char nokMessage[] = "invalid command, use 'shutdown' to terminate\n";
    SOCKET socket = passiveUDP(SHUTDOWN_PORT);
    int returnCode;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof (buffer));

    while (1) {
        returnCode = recvfrom(socket, buffer, sizeof (buffer), 0, &address, &addressLength);
        if (strncmp(shutdown, buffer, strlen(shutdown)) == 0) {
            sendto(socket, &okMessage, strlen(okMessage), 0, (struct sockaddr *) &address, (socklen_t) addressLength);
            break;
        }
        sendto(socket, &nokMessage, strlen(nokMessage), 0, (struct sockaddr *) &address, (socklen_t) addressLength);
    }
}

void *workerThread(void *tcpSocket) {
    char hello[] = "Thank you for registering\n";
    char shutdown[] = "Server shutting down..\n";
    int returnCode;
    char buffer[BUFFER_SIZE];
    
    SOCKET socket = *(SOCKET *) tcpSocket;

    memset(buffer, 0, sizeof (buffer));

    send(socket, hello, strlen(hello), 0);
    while (runFlag) {
        returnCode = recv(socket, buffer, sizeof (buffer), 0);
        if (returnCode <= 0 || buffer[0] == '\n') {
            break;
        }
        send(socket, hello, strlen(hello), 0);
    }
    if (runFlag == 0) {
        send(socket, shutdown, strlen(shutdown), 0);
    }

    close(socket);
    
    return NULL;
}

/*
 * 
 */
int main(int argc, char** argv) {

    struct sockaddr address;
    int addressLength = sizeof (address);
    pthread_t worker, shutdownHook;

    SOCKET s, ss;
    WSAinit();
    ss = passiveTCP(SERVER_PORT, 5);

    s = accept(ss, &address, &addressLength);

    pthread_create(&worker, NULL, workerThread, (void *) &s);

    pthread_create(&shutdownHook, NULL, shutdownHookThread, (void *) NULL);

    pthread_join(shutdownHook, NULL);
    
    runFlag = 0;

    pthread_join(worker, NULL);

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

