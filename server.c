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
#define SA struct sockaddr
int path_size = 512;
int PORT = 5005;
int sockfd_global;
VipsImage *image = NULL;


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



int file_exists(const char *file_path) {
    struct stat buffer;
    return (stat(file_path, &buffer) == 0);
}

void get_file_path(char *file_path){
    strcpy(file_path,"/home/bogdan-ubuntu-vm/pcd/TICKSY/test.jpg");
    printf("\n-> %s \n", file_path);
    if (!file_exists(file_path)) {
        printf("Error: The file does not exist or there is a problem with permissions.\n");
        exit(1);
    }else{
        printf("File found at %s\n",file_path);
    }
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
    send(sockfd_global, buffer, strlen(buffer), 0);
    printf("Message sent to client.\n");
}


void* newfunc(void* conn)
{
    sockfd_global = *(int*)conn; //separete globals for each thread


    sendText(choiceText);
    int choice = receiveChoice();
    choice_maker(choice);
    // printf("readinf type of operation ");
    // int type;
    // recv(connfd, &type, sizeof(int), 0);

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

    // Read Picture Byte Array
    printf("Reading Picture Byte Array\n");
    // char p_array[1024];
    // char *filename;
    // filename = generate_random_image_name("png");

    // char folder[255] = "serverIn/";
    // strcat(folder, filename);

    // FILE *image = fopen(folder, "wb");
    // int nb;
    // while (size > 0)
    // {

    //     nb = recv(connfd, p_array, 1024, 0);
    //     if (nb < 0)
    //         continue;
    //     size = size - nb;

    //     fwrite(p_array, 1, nb, image);
    // }

    // fclose(image);

    // VipsImage *in;

    // printf("Image proccesing started\n");
    // if (!(in = vips_image_new_from_file(folder, NULL)))
    //     vips_error_exit(NULL);

    // printf("image loaded\n");

    // printf("image processed\n");

    // char folder1[255] = "serverOut/";
    // strcat(folder1, filename);

    // if (vips_image_write_to_file(out, folder1, NULL))
    //     vips_error_exit(NULL);

    // printf("Image proccesing ended\n");

    // g_object_unref(in);
    // g_object_unref(out);

    // sendImg(connfd, filename);

    // return 0;
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



void choice_maker(int choice)  
{



    if (choice == 1)
        {
            if (image != NULL)
                sendText("\nOverwrite the image");
            sendText("\nGive me you image\n");
            // get_file_path(file_path);
            
            // image = vips_image_new_from_file(file_path, NULL);
            // if (!image)
            // {
            //     sendText("ERROR: fail rad")
            //     vips_error_exit("Failed to read image");
            // }
            // printf("\n The image was readed.\n\n ");
        }
        else if (choice == 7)
        {
            sendText("Closing program...\n");
        }
        else
        {
            if (image == NULL)
            {
                printf("\n No image was foud load one first !!!\n\n");
            }
            else
            {
                if (choice == 2)
                {
                    if (vips_image_write_to_file(image, generate_random_image_name(".jpg"), NULL))
                        vips_error_exit(NULL);
                    printf("\nImage savedd");
                }
                else if (choice == 3)
                {
                    image = grayscale(image);
                    printf("Changes apply\n");
                }
                else if (choice == 4)
                {
                    double resize;
                    printf("\nEnter scale to be resized: ");
                    scanf("%le", &resize);
                    image = resize_image(image, resize);
                    printf("Changes apply\n");
                }
                else if (choice == 5)
                {
                    double angle;
                    printf("\nEnter the angle to rotate: ");
                    scanf("%le", &angle);
                    image = rotate_image(image, angle);
                }
                else if (choice == 6)
                {
                    image = apply_sobel(image);
                    printf("Changes apply\n");
                }
                else
                {
                    printf("Invalid choice. Please choose a number between 1 and 7.\n");
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