/* 
 * File:   main.c
 * Author: burgc5
 *
 * Created on 10 October 2012, 14:49
 */

#include <stdio.h>
#include <stdlib.h>

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

struct tm *getLocalTime() {
    time_t now;

    now = time(NULL);
    return localtime(&now);
}

/*
 * 
 */
int main(int argc, char** argv) {

    FILE *logFile;
    struct tm *time;

    char buffer[128];
    char *message = "Thank you for registering\n";
    char *peerName;
    int peerPort;

    int numberOfReceivedBytes;
    unsigned char tcp = 1;
    struct sockaddr_in socketAddress;
    int addressLength = sizeof (socketAddress);
    const int bufferSize = sizeof (buffer);

    memset(buffer, 0, bufferSize);

    time = getLocalTime();

    //printf ( "The time is %d:%d\n", lcltime->tm_hour, lcltime->tm_min );
    logFile = fopen("/var/log/inetdserver/server.log", "a");

    if (*argv[1] == 'u') {
        tcp = 0;
    }
    if (tcp) {

        getpeername(0, (struct sockaddr *) &socketAddress, &addressLength);
        peerName = inet_ntoa(socketAddress.sin_addr);
        peerPort = socketAddress.sin_port;

        fprintf(logFile, "%02d.%02d.%04d %02d:%02d:%02d %s:%d TCP %s", time->tm_mday,
                time->tm_mon, time->tm_year + 1900,
                time->tm_hour, time->tm_min, time->tm_sec,
                peerName, peerPort, "Client connected\n");
        fflush(logFile);
        while (1) {
            numberOfReceivedBytes = read(0, buffer, bufferSize);

            time = getLocalTime();
            if (numberOfReceivedBytes < 0) {
                fprintf(logFile, "%02d.%02d.%04d %02d:%02d:%02d %s:%d TCP %s", time->tm_mday,
                        time->tm_mon, time->tm_year + 1900,
                        time->tm_hour, time->tm_min, time->tm_sec,
                        peerName, peerPort, "Socket error\n");
                fclose(logFile);
                return EXIT_FAILURE;
            } else if (numberOfReceivedBytes <= 1) {
                fprintf(logFile, "%02d.%02d.%04d %02d:%02d:%02d %s:%d TCP %s", time->tm_mday,
                        time->tm_mon, time->tm_year + 1900,
                        time->tm_hour, time->tm_min, time->tm_sec,
                        peerName, peerPort, "Client disconnected\n");
                break;
            }

            buffer[numberOfReceivedBytes] = 0;
            fprintf(logFile, "%02d.%02d.%04d %02d:%02d:%02d %s:%d TCP -> %s", time->tm_mday,
                    time->tm_mon, time->tm_year + 1900,
                    time->tm_hour, time->tm_min, time->tm_sec,
                    peerName, peerPort, buffer);
            fflush(logFile);
            write(1, message, (int) strlen(message));
        }
    } else {
        numberOfReceivedBytes = recvfrom(0, buffer, bufferSize, 0,
                (struct sockaddr *) &socketAddress, &addressLength);

        if (numberOfReceivedBytes < 0) {
            fprintf(logFile, "%02d.%02d.%04d %02d:%02d:%02d %s:%d TCP %s", time->tm_mday,
                    time->tm_mon, time->tm_year + 1900,
                    time->tm_hour, time->tm_min, time->tm_sec,
                    peerName, peerPort, "Socket error\n");
            fclose(logFile);
            return EXIT_FAILURE;
        }

        peerName = inet_ntoa(socketAddress.sin_addr);
        peerPort = socketAddress.sin_port;

        time = getLocalTime();
        fprintf(logFile, "%02d.%02d.%04d %02d:%02d:%02d %s:%d UDP -> %s", time->tm_mday,
                time->tm_mon, time->tm_year + 1900,
                time->tm_hour, time->tm_min, time->tm_sec,
                peerName, peerPort, buffer);

        sendto(1, message, (int) strlen(message), 0, (struct sockaddr *) &socketAddress, addressLength);
    }

    fclose(logFile);
    return (EXIT_SUCCESS);
}

