// #include <vips/vips.h>
#include <stdio.h>
#include "myImageProcessing.h"

int path_size = 250;

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



void choise_maker(char *file_path)  
{
    int choice;
    VipsImage *image = NULL;
    do
    {
        printf("------------------------------------------------------\n");
        printf("Please choose an option from the list below:\n");
        printf("1. Edit a new image\n");
        printf("2. Save the edited image\n");
        printf("3. Apply gray scale to the image\n");
        printf("4. Resize the image\n");
        printf("5. Rotate the image\n");
        printf("6. Apply sobel to the image\n");
        printf("7. Exit\n");
        printf("------------------------------------------------------\n");

        printf("\nEnter your choice (1-7): ");
        scanf("%d", &choice);
        // Remove newline character from the input string

        if (choice == 1)
        {
            if (image != NULL)
                printf("\nOverwrite the image");
            get_file_path(file_path);
            
            image = vips_image_new_from_file(file_path, NULL);
            if (!image)
            {
                vips_error_exit("Failed to read image");
            }
            printf("\n The image was readed.\n\n ");
        }
        else if (choice == 7)
        {
            printf("Closing program...\n");
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
    } while (choice != 7);
}

// /home/bogdan-ubuntu-vm/pcd/TICKSY/test.jpg
// strcpy(saveLocation,"out_images/");
// strcat(saveLocation, generate_random_image_name(".jpg"));

// if (vips_image_write_to_file( out, saveLocation, NULL))
//     vips_error_exit(NULL);
// printf("Image proccesing ended\n");

int main(int argc, char **argv)
{
    char *file_path = malloc(path_size * sizeof(char));
    choise_maker(file_path);
    printf("Byyy");
}

// gcc -g -Wall -o server.out server.c `pkg-config vips --cflags --libs`