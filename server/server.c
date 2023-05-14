#include <stdio.h>
#include <vips/vips.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "myImageProcessing.h"
#include <sys/wait.h>

#define SA struct sockaddr
#define path_size 512
#define PORT 5006
#define MAXLINE 512

_Thread_local int sockfd_global;
_Thread_local VipsImage *image_global = NULL;
_Thread_local char *image_name_global;

char *choiceText = "------------------------------------------------------\n"
                    "Please choose an option from the list below:\n"
                        "\t1. Edit a new image\n"
                        "\t2. Save the edited image\n"
                        "\t3. Apply gray scale to the image\n"
                        "\t4. Resize the image\n"
                        "\t5. Rotate the image\n"
                        "\t6. Apply sobel to the image\n"
                        "\t7. Exit\n"
                    "------------------------------------------------------\n";


void choice_maker(int);  

int my_read_size(int nr1, int *remainSize){
    if (nr1<*remainSize){
        *remainSize -= nr1;
    }else{
        nr1 = *remainSize;
        *remainSize = 0;
    }
    return nr1;

}

int file_exists(const char *file_path) {
    struct stat buffer;
    return (stat(file_path, &buffer) == 0);
}

// void get_file_path(char *file_path){
//     strcpy(file_path,"/home/bogdan-ubuntu-vm/pcd/TICKSY/test.jpg");
//     printf("\n-> %s \n", file_path);
//     if (!file_exists(file_path)) {
//         printf("Error: The file does not exist or there is a problem with permissions.\n");
//         exit(1);
//     }else{
//         printf("File found at %s\n",file_path);
//     }
// }

void receiveImage( )
{
    FILE *picture;
    int sizePic;
    char extension[] = ".jpg";
    char *image_path = generate_random_image_name(extension);  

    picture = fopen(image_path, "wb");
    if (picture == NULL) {
        printf("Error opening the image file!\n");
        exit(1);
    }

    recv(sockfd_global, &sizePic, sizeof(sizePic), 0);
    printf("Received Picture Size: %d\n", sizePic);

    char recv_buffer[1024];
    int nb;
    int read_size = my_read_size(MAXLINE, &sizePic);
    int transationNr = 0;
    while(read_size > 0)
    {
        printf("Transaction %d , receiving %u \n",++transationNr,read_size);
        nb = recv(sockfd_global, recv_buffer, read_size, 0);
        fwrite(recv_buffer, 1, nb, picture);
        read_size = my_read_size(MAXLINE, &sizePic);
    }
    printf("Image name file %s\n",image_path);
    image_global = vips_image_new_from_file(image_path,NULL);

    if (image_global == NULL){
            vips_error_exit(NULL);
    }
    image_name_global = image_path;
    fclose(picture);
}




int receiveChoice() {
    char* buffer = malloc(5*sizeof(int));
    int len;
    printf("Receiving chose...\n");
    len = recv(sockfd_global, buffer, 5, 0);
    buffer[len] = '\0';
    printf("Choice received: %s\n", buffer);
    return atoi(buffer);
}
void sendTextChild(char* buffer){
    send(sockfd_global, buffer, strlen(buffer), 0);
    printf("Message sent to client.\n");
}

void sendText(char* buffer) {
    pid_t pid = fork();  
    if (pid < 0) {
        printf("Error: fork failed\n");
        exit(1);
    }
    if (pid == 0) {  
        // printf("%s with length %ld\n",buffer,strlen(buffer));
        sendTextChild(buffer);
        _exit(0);  
    } else {  
        wait(NULL);  
    }
}


void* newfunc(void* conn)
{
    sockfd_global = *(int*)conn; //separete globals for each thread
    int choice = 7;
    do{
        sendText(choiceText);
        choice = receiveChoice();
        choice_maker(choice);
    }while(choice != 7);
    return NULL;

}
void* func(void* conn)
{
    int connfd = *(int*)conn;
    printf("readinf type of operation ");
    int type;
    recv(connfd, &type, sizeof(int), 0);
    printf("%d \n", type);
    if (type < 1 || type > 5)
    {
        // return 1;
    }
    printf("Reading Picture Size\n");
    int size;
    recv(connfd, &size, sizeof(int), 0);
    printf("Received Picture Size: %d\n", size);

    printf("Reading Picture Byte Array\n");
    return NULL;
}

void *inetClient()
{   
    int socket_desc;
    struct sockaddr_in server;
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        perror("EROARE server: nu pot sa deschid stream socket");
    }
    printf("Socket creat\n");
    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(socket_desc, (struct sockaddr *)&server, sizeof(server))) != 0){
        perror("Bind failed. Error");
        // return 1;
    }
    printf("Bind realizat\n");
    if ((listen(socket_desc, 30)) != 0){
        printf("Listen failed...\n");
        exit(0);
    }
    printf("Server listening..\n");
    struct sockaddr_in client_addr;
    while (TRUE)
    {
        // Accept the data packet from client and verification
        socklen_t len = sizeof(client_addr);
        int connected_sock = accept(socket_desc, (SA *)&client_addr,&len );
        if (connected_sock < 0)
        {
            printf("server accept failed...\n");
            exit(0);
        }
        printf("server accept the client...\n");
        pthread_t t;
        // int *pconfd = malloc(sizeof(int));
        // *pconfd = connected_sock;
        pthread_create(&t, NULL, newfunc, (void *)&connected_sock);
    }
    // After chatting close the socket
    close(socket_desc);
}


void sendImage()
{
    FILE *picture;



// Don't fo
    char *image_path_modf = calloc(32,sizeof(char));
    strncat(image_path_modf, "m", 30);
    strncat(image_path_modf, image_name_global, 30 - strlen(image_path_modf));

    // strcat(image_path_modf,"m");
    // strcat(image_path_modf,image_name_global);


    printf("Image name before send (%s)...\n",image_path_modf);
    if (vips_image_write_to_file(image_global, image_path_modf, NULL))
            vips_error_exit(NULL);
    picture = fopen(image_path_modf, "rb");
    if (picture == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    int sizePic;
    fseek(picture, 0, SEEK_END);
    sizePic = ftell(picture);
    fseek(picture, 0, SEEK_SET);

    printf("Sending image name (%s)...\n",image_path_modf);
    sendText(image_path_modf);

    send(sockfd_global, &sizePic, sizeof(sizePic), 0);
    printf("Sended Picture Size: %d\n", sizePic);

    char send_buffer[MAXLINE+1];
    size_t bytesRead;
    // int transactionNr = 0;
    while ((bytesRead = fread(send_buffer, sizeof(char), MAXLINE, picture)) > 0) {
        // printf("Transaction  %d sending %zu,\n",++transactionNr, bytesRead);
        send(sockfd_global, send_buffer, bytesRead, 0);
    }

    fclose(picture);
}


void choice_maker(int choice)  
{
    if (choice == 1)
        {
            if (image_global != NULL)
                sendText("Overwrite the image, Give me you image.\0");
            else
                sendText("Give me you image \n");
            receiveImage();
        }else if (choice == 7){
            sendText("Closing program...\n");
        }else{
            if (image_global == NULL)
            {
                sendText("No image was found load one first!\n\0");
            }
            else
            {
                if (choice == 2)
                {
                    sendText("Returning image ...\n");
                    printf("Sending image...\n");
                    sendImage();
                    printf("Image sended.\n");
                }
                else if (choice == 3)
                {
                    image_global = grayscale(image_global);
                    sendText("Changes apply\n");
                }
                else if (choice == 4)
                {
                    char* buffer_size = malloc(5*sizeof(int));
                    int len;
                    printf("Receiving scale to be resized:...\n");
                    sendText("Please enter a double for resize scale :");
                    len = recv(sockfd_global, buffer_size, 10, 0);
                    buffer_size[len] = '\0';
                    double resize= atof(buffer_size);
                    printf("resize get %f\n",resize);
                    image_global = resize_image(image_global, resize);
                    sendText("Changes apply\n");
                    free(buffer_size);
                }
                else if (choice == 5)
                {
                    char* buffer_angle = malloc(5*sizeof(int));
                    int len;
                    printf("Receiving the angle to rotate:...\n");
                    sendText("Enter the angle to rotate: ");
                    len = recv(sockfd_global, buffer_angle, 10, 0);
                    buffer_angle[len] = '\0';
                    double angle= atoi(buffer_angle);
                    image_global = rotate_image(image_global, angle);
                    sendText("Changes apply\n");
                    free(buffer_angle);
                }
                else if (choice == 6)
                {
                    image_global = apply_sobel(image_global);
                    sendText("Changes apply\n");
                }
                else
                {
                    sendText("Invalid choice. Please choose a number between 1 and 7.\n");
                }
            }
        }
}


int main(int argc, char **argv)
{
    if(VIPS_INIT(argv[0]))
        vips_error_exit( NULL );
    pthread_t thread_id[2];

    // pthread_create(&thread_id[0], NULL, adminClient, NULL);
    pthread_create(&thread_id[0], NULL, inetClient, NULL);
    pthread_join(thread_id[0], NULL);
    // for (int i = 0; i < 1; i++)
    //     pthread_join(thread_id[i], NULL);
    
    printf("Byyy\n");
    return (0);
}

// gcc -g -Wall -o server.out server.c `pkg-config vips --cflags --libs`