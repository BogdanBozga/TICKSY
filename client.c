#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 512
int PORT = 5005;

int sockfd_global;


void receiveText() {
    char buffer[MAXLINE];
    int len;
    len = recv(sockfd_global, buffer, MAXLINE, 0);
    buffer[len] = '\0';
    printf("Server: %s\n", buffer);
}

void sendText(char* buffer) {
    send(sockfd_global, buffer, strlen(buffer), 0);
    printf("Message sent to server.\n");
}


int sendImage(){
    return 0;
}






int main() {
    char buffer[MAXLINE];
    
    struct sockaddr_in servaddr;

    sockfd_global = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_global == -1) {
        printf("Socket creation failed...\n");
        exit(0);
    }
    printf("Socket successfully created..\n");

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd_global, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        printf("Connection with the server failed...\n");
        exit(0);
    }
    printf("Connected to the server..\n");

    while (1) {
        bzero(buffer, sizeof(buffer));        
        receiveText();
        fgets(buffer, sizeof(buffer), stdin);
        sendText(buffer);
        receiveText();
    }

    // close the socket
    close(sockfd_global);
}