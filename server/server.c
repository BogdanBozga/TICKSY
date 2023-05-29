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
#include <dirent.h>

#define SOCK_PATH "/tmp/my_socket.sock"
#define UNIX_PATH_MAX 108
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

void receiveImage( )
{
    FILE *picture;
    int sizePic;
    char extension[] = ".jpg";
    char *image_name= generate_random_image_name(extension);  

    char *image_path_in = calloc(50,sizeof(char));
    strncat(image_path_in, "in/", 49);

    strncat(image_path_in, image_name, 49 - strlen(image_name));
    
    picture = fopen(image_path_in, "wb");
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
    printf("Image name file %s\n",image_name);
    image_global = vips_image_new_from_file(image_path_in,NULL);
    if (image_global == NULL){
            vips_error_exit(NULL);
    }
    image_name_global = calloc(50,sizeof(char));
    strcpy(image_name_global, image_name);
    fclose(picture);
    free(image_name);
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

void sendText(char* buffer) {
    pid_t pid = fork();  
    if (pid < 0) {
        printf("Error: fork failed\n");
        exit(1);
    }
    if (pid == 0) {  
        send(sockfd_global, buffer, strlen(buffer), 0);
        printf("Message sent to client.\n");
        _exit(0);  
    } else {  
        wait(NULL);  
    }
}


void* newfunc(void* conn)
{
    sockfd_global = *(int*)conn; 
    int choice = 7;
    do{
        sendText(choiceText);
        choice = receiveChoice();
        choice_maker(choice);
    }while(choice != 7);
    return NULL;
}

// void* func(void* conn)
// {
//     int connfd = *(int*)conn;
//     printf("readinf type of operation ");
//     int type;
//     recv(connfd, &type, sizeof(int), 0);
//     printf("%d \n", type);
//     if (type < 1 || type > 5)
//     {
//         // return 1;
//     }
//     printf("Reading Picture Size\n");
//     int size;
//     recv(connfd, &size, sizeof(int), 0);
//     printf("Received Picture Size: %d\n", size);

//     printf("Reading Picture Byte Array\n");
//     return NULL;
// }


long getNumber(int nrFiles, int inFiles){
    long total_bytes = 0;
    long file_count = 0;
    struct dirent *dir_entry;
    struct stat file_stats;
    DIR *dir;
    char inFilePath[] = "in/";
    char outFilePath[] = "out/";

    char *dir_path;
    if(inFiles == 1){
        dir_path = inFilePath;
    }else{
        dir_path = outFilePath;
        
    }
    dir = opendir(dir_path);
    if(dir == NULL) {
        perror("Unable to read directory");
        return 1;
    }

    while((dir_entry = readdir(dir)) != NULL) {
        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, dir_entry->d_name);
        if(stat(file_path, &file_stats) == 0) {
            if(S_ISREG(file_stats.st_mode)) {  // Check if it is a regular file
                total_bytes += file_stats.st_size;
                file_count++;
            }
        }
        else {
            perror("Cannot retrieve file stats");
            return 1;
        }
    }

    closedir(dir);

    // printf("Total files: %d\n", file_count);
    // printf("Total bytes: %ld\n", total_bytes);
    if(nrFiles ==1){
        return file_count;
    }else{
        return total_bytes;
    }

}



void adminHandler(int connfd)
{
    while (TRUE)
    {
        int option;
        recv(connfd, &option, sizeof(int), 0);
        long choice =0;

        if (option == 0)
        {
            // getNumber(int nrFiles, int inFiles)
            choice = getNumber(1, 0); // 1 - images, 0 - pachages /// 0 -  returned /// 1 - only recived

            send(connfd, &choice, sizeof(int), 0);
        }
        else if (option == 1)
        {
            choice = getNumber(1, 1);
            send(connfd, &choice, sizeof(int), 0);
        }
        else if (option == 2)
        {
            choice = getNumber(1, 0);// nr images returned  
            send(connfd, &choice, sizeof(int), 0);
        }
        else if (option == 3)
        {
            
            // long nrF =  getNumber(1, 1);
            long nrB =  getNumber(0, 1); // nr pachage received 
            choice = nrB/MAXLINE;
            send(connfd, &choice, sizeof(int), 0);
        }
        else if (option == 4)
        {
            long nrB =  getNumber(0, 0); // nr pachage returned 
            choice = nrB/MAXLINE;
            send(connfd, &choice, sizeof(int), 0);
        }
        else if (option == 5)
        {
            choice = getNumber(0, 1);
            send(connfd, &choice, sizeof(int), 0);
        }
        else if (option == 6)
        {
            choice =  getNumber(0, 0);
            send(connfd, &choice, sizeof(int), 0);
        }
        else
        {
            break;
        }
    }
}


void *adminClient()
{
    int sock1, sock2;
    struct sockaddr_un admin, remote;

    if ((sock1 = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket admin");
        exit(1);
    }
    printf("Socket create for admin\n");
    memset(&admin, 0, sizeof(struct sockaddr_un));
    admin.sun_family = AF_UNIX;

    strncpy(admin.sun_path, SOCK_PATH, UNIX_PATH_MAX - 1);
    unlink(SOCK_PATH);
    if (bind(sock1, (struct sockaddr *)&admin, sizeof(admin)) == -1)
    {
        perror("bind admin");
        exit(1);
    }
    printf("Bind realizat for admin\n");
    if (listen(sock1, 1) == -1)
    {
        perror("listen admin");
        exit(1);
    }
    printf("Server listening for admin..\n");
    while (TRUE)
    {
        printf("Waiting for admin client...\n");
        socklen_t size = sizeof(remote);
        if ((sock2 = accept(sock1, (struct sockaddr *)&remote, &size)) == -1)
        {
            perror("accept");
            exit(1);
        }
        printf("Admin client connected\n");
        adminHandler(sock2);
        close(sock2);
    }
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
        printf("Client connected on port %d\n",ntohs(client_addr.sin_port));
        pthread_t t;
        pthread_create(&t, NULL, newfunc, (void *)&connected_sock);
    }
    // After chatting close the socket
    close(socket_desc);
}


void sendImage()
{
    FILE *picture;
    char *image_path_modf = calloc(50,sizeof(char));
    char *image_name_modf = calloc(50,sizeof(char));
    strncat(image_name_modf, "m", 49);



    strncat(image_name_modf, image_name_global, 40 - strlen(image_name_modf)
    
    );
    printf("Image name before send (%s)...\n",image_name_modf);

    strncat(image_path_modf, "out/", 49);
    strncat(image_path_modf, image_name_modf, 49 - strlen(image_path_modf));

    printf("hmmm %s\n",image_path_modf);
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

    printf("Sending image name (%s)...\n",image_name_modf);
    sendText(image_name_modf);

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

    pthread_create(&thread_id[1], NULL, adminClient, NULL);
    pthread_create(&thread_id[0], NULL, inetClient, NULL);
    // pthread_join(thread_id[0], NULL);
    for (int i = 0; i < 2; i++)
        pthread_join(thread_id[i], NULL);
    
    printf("Byyy\n");
    return (0);
}

// gcc -g -Wall -o server.out server.c `pkg-config vips --cflags --libs`