/* 
 * File:   main.c
 * Author: burgc5
 *
 * Created on 26 September 2012, 15:41
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>


#define HOST_ADDRESS (((((147 << 8) | 87) << 8) | 117) << 8) | 163
#define PORT 20901
#define TCP_SOCKET_FLAGS 0

/*
 * 
 */
int main(int argc, char** argv) {

    printf("Select method to connect\n");
    printf("UDP: 1\n");
    printf("TCP: 2\n");

    char buf[64];
    fgets(buf, 63, stdin);
    buf[1] = 0;

    if (buf[0] == '1') {
        printf("Connecting with UDP\n");
        return connectWithUdp();
    } else if (buf[0] == '2') {
        printf("Connecting with TCP\n");
        return connectWithTcp();
    }

    printf("No valid method selected. Exiting..\n");
    return (EXIT_FAILURE);
}

int connectWithUdp() {
    char buf[64];

    struct sockaddr_in serverSocket;
    int socketAddress;
    int numReceivedBytes, addressLength = sizeof (struct sockaddr_in);

    socketAddress = socket(PF_INET, SOCK_DGRAM, 0);
    if (socketAddress == -1) {
        return EXIT_FAILURE;
    }

    serverSocket.sin_family = AF_INET;
    serverSocket.sin_port = htons(PORT);
    serverSocket.sin_addr.s_addr = htonl(HOST_ADDRESS);
    memset(&serverSocket.sin_zero, 0, sizeof (serverSocket.sin_zero));

    printf("Socket opened. Enter message:\n");

    while (1) {

        fgets(buf, 63, stdin);
        buf[63] = 0;

        if (buf[0] == '\n') {
            break;
        }

        sendto(socketAddress, buf, (int) strlen(buf), 0,
                (struct sockaddr *) &serverSocket, addressLength);

        numReceivedBytes = recvfrom(socketAddress, buf, sizeof (buf), 0,
                (struct sockaddr *) &serverSocket, &addressLength);
        if (numReceivedBytes < 2) {
            return EXIT_FAILURE;
        }
        buf[numReceivedBytes - 2] = 0;
        printf("%s\n", buf);

    }

    close(socketAddress);

    return (EXIT_SUCCESS);
}

int connectWithTcp() {
    char buf[64];

    struct sockaddr_in serverSocket;
    int socketAddress;
    int addressLength = sizeof (struct sockaddr_in);

    socketAddress = socket(PF_INET, SOCK_STREAM, 0);
    if (socketAddress == -1) {
        return EXIT_FAILURE;
    }

    serverSocket.sin_family = AF_INET;
    serverSocket.sin_port = htons(PORT);
    serverSocket.sin_addr.s_addr = htonl(HOST_ADDRESS);
    memset(&serverSocket.sin_zero, 0, sizeof (serverSocket.sin_zero));

    int success = connect(socketAddress, (struct sockaddr *) &serverSocket, addressLength);
    if (success == EXIT_FAILURE) {
        close(socketAddress);
        return EXIT_FAILURE;
    }

    int welcomeMessageLines = 3;
    receiveAndPrintUdpMessage(socketAddress, welcomeMessageLines);

    while (1) {

        fgets(buf, 63, stdin);
        buf[63] = 0;

        if (buf[0] == '\n') {
            break;
        }

        send(socketAddress, buf, (int) strlen(buf), 0);

        int messageLines = 1;
        receiveAndPrintUdpMessage(socketAddress, messageLines);

    }

    return (close(socketAddress));
}

int receiveAndPrintUdpMessage(int socket, int numberOfLines) {
    char incomingData[64];
    int receiveAgain = 1;
    int numberOfLineFeeds = 0;
    int maxReceiveLength = (int) sizeof (incomingData) - 1;
    int numberOfReceivedBytes;
    do {
        numberOfReceivedBytes = recv(socket, incomingData, maxReceiveLength, TCP_SOCKET_FLAGS);
        if (numberOfReceivedBytes == EXIT_FAILURE) {
            close(socket);
            return EXIT_FAILURE;
        }
        int i;
        for (i = 0; i < numberOfReceivedBytes; i++) {
            printf("%.*s", 1, incomingData + i);
            if (incomingData[i] == '\n') {
                numberOfLineFeeds++;
                if (numberOfLineFeeds >= numberOfLines) {
                    receiveAgain = 0;
                    break;
                }
            }
        }

    } while (receiveAgain);
    
    return EXIT_SUCCESS;
}

