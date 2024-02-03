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
#include <stdio.h>								// perror
#include <stdlib.h>
#include <string.h>								// used for strlen
#include <pthread.h>								// threads
#include <unistd.h>								// read, write and close, usleep
#include <fcntl.h>								// open and permissions
#include <dirent.h>								// dirent
#include <sys/stat.h>								// stats
#include <limits.h>	


//so_reuse_ADDR
#define MAXLINE 512
#define MAX_THREADS 100	
#define PORT 5006
int sockfd_global;

					
char *directoryPath;	


void getCurrentDir(){
    directoryPath = malloc(1024);  // allocate memory for the path
    if (getcwd(directoryPath, 1024) != NULL) {
        printf("Current working directory: %s\n", directoryPath);
    } else {
        perror("getcwd() error");
    }
}

void *fileThread(struct dirent * entry) {

    	struct stat fileStat;
    	// using dynamic memory, the code would not find the correct path for some reason and would perror for each thread
    	// solution: the one below
	    char fullPath[PATH_MAX];						// PATH_MAX Max nr bytes in pathname o
    	snprintf(fullPath, sizeof(fullPath), "%s/%s", directoryPath, entry->d_name);
    										// glue together the path and the file name 
										// to make the true path
	// printf would trigger perror, sprintf would trigger memory leaks, snprintf doesn't trigger memory leaks and works
    	if (stat(fullPath, &fileStat) < 0) {
        	perror("stat");
        	exit(EXIT_FAILURE);
    	}
    	char perms[10] = "---------"; 						// simulate ls -l command permissions
    	mode_t fileMode = fileStat.st_mode;
    	if (fileMode & S_IRUSR) perms[0] = 'r';					// add letter for each permission that holds true
    	if (fileMode & S_IWUSR) perms[1] = 'w';
    	if (fileMode & S_IXUSR) perms[2] = 'x';
    	if (fileMode & S_IRGRP) perms[3] = 'r';
    	if (fileMode & S_IWGRP) perms[4] = 'w';
    	if (fileMode & S_IXGRP) perms[5] = 'x';
    	if (fileMode & S_IROTH) perms[6] = 'r';
    	if (fileMode & S_IWOTH) perms[7] = 'w';
    	if (fileMode & S_IXOTH) perms[8] = 'x';
    	printf("\tFile: %s, Size: %zu  bytes, Permissions: %s\n", entry->d_name, fileStat.st_size, perms);
    										// print file name, size and permissions
    	pthread_exit(NULL);							// close thread to free memory 
										// and prevent bullshittery from happening
}

void chech_directory(){
    getCurrentDir();
        printf("\nReading from: %s\n", directoryPath);
    	DIR *directory = opendir(directoryPath);
    	if (directory == NULL) {						// check if directory exists
        	perror("Error opening directory");
        	exit(EXIT_FAILURE);
    	}
    	struct dirent *dirEntry;
    	pthread_t *threadfiles = malloc(MAX_THREADS * sizeof(pthread_t));	// alloc max 100 threads
    	int threadCount = 0;							// variable for counting nr threads
    	while ((dirEntry = readdir(directory)) != NULL) {			
        	if (dirEntry->d_type == DT_REG) {				// check for regular files
            		threadfiles = realloc(threadfiles, (threadCount + 1) * sizeof(pthread_t));	
										// realloc to make space for new thread
            		pthread_create(&threadfiles[threadCount], NULL, (void *) fileThread, (void *) dirEntry);
										// create said thread
            		threadCount++;						// increase counter
        	}
    	}
    	for (int i = 0; i < threadCount; i++) {					// loop to achieve parallelism
        	pthread_join(threadfiles[i], NULL);
    	}
    	closedir(directory);							// close dir
    	free(threadfiles);	
}

char* receiveText()
{
    char *buffer = malloc(MAXLINE*sizeof(char));
    int len;
    len = recv(sockfd_global, buffer, MAXLINE, 0);
    buffer[len] = '\0';
    printf("Server: %s\n", buffer);
    return buffer;
}

int my_read_size(int nr1, int *remainSize){
    if (nr1<*remainSize){
        *remainSize -= nr1;
    }else{
        nr1 = *remainSize;
        *remainSize = 0;
    }
    return nr1;
}

char* receiveImage( )
{
    FILE *picture;
    int sizePic;
    char *image_name = receiveText();
    printf("Image name recived is %s\n",image_name);
    picture = fopen(image_name, "wb");
    if (picture == NULL) {
        printf("Failed to open the file for writing.\n");
        exit(1);
    }

    recv(sockfd_global, &sizePic, sizeof(sizePic), 0);
    printf("Received Picture Size: %d\n", sizePic);


    char recv_buffer[1024];
    int nb;
    int read_size = my_read_size(MAXLINE, &sizePic);
    // int transationNr = 0;
    while(read_size > 0)
    {
        nb = recv(sockfd_global, recv_buffer, read_size, 0);
        fwrite(recv_buffer, 1, nb, picture);
        read_size = my_read_size(MAXLINE, &sizePic);
    }

    fclose(picture);
    return image_name;
}



void sendText(char *buffer)
{
    send(sockfd_global, buffer, strlen(buffer), 0);
    printf("Message sent to server.\n");
}


int file_exists(const char *image_path)
{
    if (access(image_path, F_OK) != -1) {
        printf("File '%s' exists.\n", image_path);
    } else {
        printf("File '%s' does not exist.\n", image_path);
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
    memset(image_path, 0, sizeof(image_path));
    printf("Enter a image path: ");
    fgets(image_path, sizeof(image_path), stdin);
    image_path[strcspn(image_path, "\n")] = '\0';
    printf("%s\n",image_path);
    while (file_exists(image_path) == 1){
        chech_directory();
        printf("Wrong path or extension (not .jpg or .png)\nTry again ...\n");
        memset(image_path, 0, sizeof(image_path));
        fgets(image_path, sizeof(image_path), stdin);
        image_path[strcspn(image_path, "\n")] = '\0';
    }
    picture = fopen(image_path, "rb");
    if (picture == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    int sizePic;
    fseek(picture, 0, SEEK_END);
    sizePic = ftell(picture);
    fseek(picture, 0, SEEK_SET);

    send(sockfd_global, &sizePic, sizeof(sizePic), 0);
    printf("Sended Picture Size: %d\n", sizePic);

    char send_buffer[MAXLINE+1];
    size_t bytesRead;
    // int transactionNr = 0;
    while ((bytesRead = fread(send_buffer, sizeof(char), MAXLINE, picture)) > 0) {
        // printf("Transaction  %d sending %zu,\n",++transactionNr, bytesRead);
        send(sockfd_global, send_buffer, bytesRead, 0);
        // fseek(picture, bytesRead, SEEK_CUR);
    }
    fclose(picture);
}

int main()
{
    

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
        char *text = receiveText(); // choice  list
        if (strstr(text, "Please choose an option") != NULL){
            char *buffer = malloc(MAXLINE*sizeof(char));
            int choice;
            printf("Your choice:");
            scanf("%d", &choice);
            printf("Choice selected : %d\n",choice);
            char str[12];
            sprintf(str, "%d", choice);
            sendText(str); // send choice
            
            char *text = receiveText();
            if (strstr(text, "Give me you image") != NULL) //1
            {
                printf("Sending image option selected.\n");
                sendImage();
            }
            else if (strstr(text, "Returning image") != NULL) //2
            {
                printf("Reciving image ...\n");
                receiveImage();

            }else if (strstr(text, "Closing program") != NULL) //7
            {
                printf("Finishing program ...\n");
                break;

            }else if (strstr(text, "resize") != NULL) //4
            {
                double resize;
                while (scanf("%lf", &resize) != 1) {
                    printf("Invalid input. Please enter a number.\n");
                }
                char *str = malloc(10 * sizeof(char));
                
                sprintf(str, "%lf", resize);
                sendText(str);
                free(str); 
                receiveText();
            }else if (strstr(text, "rotate") != NULL) //5
            {
                double angle;
                while (scanf("%lf", &angle) != 1) {
                    printf("Invalid input. Please enter a number.\n");
                }
                char *str = malloc(10 * sizeof(char));
                
                sprintf(str, "%lf", angle);
                sendText(str);
                free(str); 
                receiveText();

            }else if(strstr(text, "Changes apply") != NULL){
                printf("good\n");
            }else{
                printf("Nothing done.\n");
            }
        }
    }
    // close the socket
    close(sockfd_global);
}