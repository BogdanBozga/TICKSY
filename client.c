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
#include <sys/stat.h>

#define MAXLINE 512
#define MAX_THREADS 100	
int PORT = 5005;
int sockfd_global;



char* receiveText()
{
    char *buffer = malloc(MAXLINE*sizeof(char));
    int len;
    len = recv(sockfd_global, buffer, MAXLINE, 0);
    buffer[len] = '\0';
    printf("Server: %s\n", buffer);
    return buffer;
}

void sendText(char *buffer)
{
    send(sockfd_global, buffer, strlen(buffer), 0);
    printf("Message sent to server.\n");
}

int file_exists(const char *image_path)
{
    struct stat buffer;
    if (stat(image_path, &buffer) != 0)
    {
        return 1;
    }
    const char *extension = strrchr(image_path, '.');
    if (extension)
    {
        if (strcmp(extension, ".png") == 0 || strcmp(extension, ".jpg") == 0)
        {
            return 0;
        }
    }
    return 1;
}




void sendImage()
{
    FILE *picture;
    char image_path[256];
    printf("Enter a image path: ");
    fgets(image_path, sizeof(image_path), stdin);
    while (file_exists(image_path) == 1){
        printf("Wrong path or extension (not .jpg or .png)\nTry again ...\n");
        fgets(image_path, sizeof(image_path), stdin);
    }
    picture = fopen(image_path, "rb");

    int sizePic;
    fseek(picture, 0, SEEK_END);
    sizePic = ftell(picture);
    fseek(picture, 0, SEEK_SET);

    send(sockfd_global, &sizePic, sizeof(sizePic), 0);
    printf("Sended Picture Size: %d\n", sizePic);

    char send_buffer[1024];
    do
    {
        int nb = fread(send_buffer, 1, sizeof(send_buffer), picture);
        send(sockfd_global, send_buffer, nb, 0);
    } while (!feof(picture));
    fclose(picture);
}

int main()
{
    char buffer[MAXLINE];

    struct sockaddr_in servaddr;

    sockfd_global = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_global == -1)
    {
        printf("Socket creation failed...\n");
        exit(0);
    }
    printf("Socket successfully created..\n");

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd_global, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("Connection with the server failed...\n");
        exit(0);
    }
    printf("Connected to the server..\n");

    while (1)
    {
        bzero(buffer, sizeof(buffer));
        receiveText(); // choice  list
        fgets(buffer, sizeof(buffer), stdin);
        sendText(buffer); // send choice
        char *text = receiveText();
        if (strstr(text, "Give me you image") != NULL)
        {
            printf("Sending image option selected.\n");
            sendImage();
        }
        else
        {
            printf("Other option.\n");
        }
    }

    // close the socket
    close(sockfd_global);
}