// #include <vips/vips.h>
#include <stdio.h>
#include "myImageProcessing.h"

void choise_maker(char * file_path){
    char * choice = malloc(10*sizeof(char));
    VipsImage* image = NULL;
    VipsImage* edited_image = NULL;
 do {
        printf("Please choose an option from the list below:\n");
        printf("1. Edit a new image\n");
        printf("2. Save the edited image\n");
        printf("3. Apply gray scale to the image\n");
        printf("4. Resize the image\n");
        printf("5. Rotate the image\n");
        printf("6. Apply sobel to the image\n");
        printf("7. Exit\n");


        printf("\nEnter your choice (1-7): ");
        fgets(choice, sizeof(choice), stdin);

        // Remove newline character from the input string
        choice[strcspn(choice, "\n")] = '\0';

        if (strcmp(choice, "1") == 0) {
            if( image != NULL)
                printf("\nOverwrite the image");
            printf("\nEnter the path to the image: ");
            fgets(file_path, sizeof(file_path), stdin);

            choice[strcspn(file_path, "\n")] = '\0';
            image = vips_image_new_from_file (file_path, NULL); 
            printf("\n The image was readed: ");
        } else if (strcmp(choice, "2") == 0) {
            if(edited_image == NULL){
                printf("\nEdit the image first");
            }else{
                if (vips_image_write_to_file( edited_image, "", NULL))
                    vips_error_exit(NULL);
                printf("\nImage savedd");
            }
        } else if (strcmp(choice, "3") == 0) {
            edited_image = grayscale(image);
            printf("Changes apply\n");
        } else if (strcmp(choice, "4") == 0) {
            double resize;
            printf("\nEnter scale to be resized: ");
            scanf("%le", &resize);
            edited_image = resize_image(image, resize);
            printf("Changes apply\n");
        } else if (strcmp(choice, "5") == 0) {
            int angle;
            printf("\nEnter the angle to rotate: ");
            scanf("%d", &angle);
            edited_image = rotate_image(image, angle);
        } else if (strcmp(choice, "6") == 0) {
            edited_image = apply_sobel(image);
            printf("Changes apply\n");
        } else if (strcmp(choice, "7") == 0) {
            printf("Closing program...\n");
        } else {
            printf("Invalid choice. Please choose a number between 1 and 7.\n");
        }

    } while (strcmp(choice, "7") != 0);
}
// strcpy(saveLocation,"out_images/");
// strcat(saveLocation, generate_random_image_name(".jpg"));

// if (vips_image_write_to_file( out, saveLocation, NULL))
//     vips_error_exit(NULL);
// printf("Image proccesing ended\n");

int main( int argc, char **argv ) {
    char *file_path = malloc(250*sizeof(char));
    choise_maker(file_path);
    printf("Byyy");
}


//gcc -g -Wall -o server.out server.c `pkg-config vips --cflags --libs` 