#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCK_PATH "/tmp/my_socket.sock"


int main(){

	printf("Starting ");
    int sockfd, size, len;
    struct sockaddr_un remote;
    char string[1024];
    int choice, noOfImg;
    double uptime;

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);

    len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    if (connect(sockfd, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(2);
    }

    printf("Admin client connected\n");

    while (1) {
        printf("Choose operation typing the number:\n");
        printf("0 - Total number of images processed;\n");
        printf("1 - Total number of images received;\n");
        printf("2 - Total number of images sent;\n");
        printf("3 - Total number of pachets received\n");
        printf("4 - Total number of pachets sent\n");
        printf("5 - Total number of bytes received\n");
        printf("6 - Total number of bytes sent\n");
        printf("Any key - to exit.\n");
        printf("Choice: ");
		scanf("%d", &choice);

        if (choice < 0 || choice > 6) {
            printf("Admin client disconnected\n");
            exit(0);
        }
        else {
            send(sockfd, &choice, sizeof(int), 0);
            recv(sockfd, &noOfImg, sizeof(int), 0);
            printf("%d\n", noOfImg);
        }
    }

    close(sockfd);
    return 0;
}
