#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vips/vips.h>


VipsImage* make_copy(VipsImage *image){
    VipsImage *copyImage;
    if (vips_copy(image, &copyImage, NULL))
        vips_error_exit(NULL);
    return copyImage;
}

VipsImage* grayscale(VipsImage* image) {
    VipsImage *copyImage = make_copy(image);
    VipsImage *scRGB;
    VipsImage *grayImage;
    if (vips_sRGB2scRGB(copyImage, &scRGB, NULL))
        vips_error_exit("Fail to trasform RGB2 to RGB");
    if (vips_scRGB2BW(scRGB, &grayImage, NULL))
        vips_error_exit("Fail to trasform RGB to BW(gray)");
    g_object_unref(copyImage);
    g_object_unref(scRGB);

    return grayImage;
}

VipsImage* rotate_image(VipsImage* image,double angle) {
    VipsImage *rotatedImage = NULL;
    VipsImage *copy = make_copy(image);
    if(vips_rotate(copy,&rotatedImage,angle, NULL)){
        vips_error_exit("F;uck");
    }
    g_object_unref(copy);
    return rotatedImage;
}

char* generate_random_image_name(const char *extension) {
    printf(" extension %s\n",extension);
    const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int alphabet_size = sizeof(alphabet) - 1;
    int random_length = 20;
    char *buffer=NULL;
    printf(" new buffer %s\n",buffer);
    buffer = malloc(30*sizeof(char)+1);
    strcat(buffer, "m");
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(buffer, 17, "%Y:%m:%d_%H:%M_", t);
    char s = '-';
    buffer[strlen(buffer)] = s;
    time_t t2; 
    srand((unsigned)time(&t2));
    for (int i = strlen(buffer); i < random_length; ++i) {
        srand(time(NULL));
        buffer[i] = alphabet[rand()%alphabet_size];
    }
    // printf("without extension %s\n",buffer);
    strcat(buffer, extension);
    // printf("with extension %s\n",buffer);


    char *copy = malloc(30*sizeof(char)+1);
    strcpy(copy,buffer);
    free(buffer);
    return copy;
}


VipsImage* resize_image(VipsImage* image, double scale) {
    VipsImage *copy = make_copy(image);
    VipsImage *resized_image; 
    if (vips_resize(copy, &resized_image, scale, NULL)) {
        vips_error_exit("Failed to resize image");
    }

    g_object_unref(copy);
    return resized_image;
}


VipsImage *apply_sobel(VipsImage *image) {
    VipsImage *copy = make_copy(image);
    VipsImage *sobel_image = NULL;

    if (vips_sobel(copy, &sobel_image, NULL)) {
        vips_error_exit("Failed to apply Sobel operator");
        return NULL;
    }

    g_object_unref(copy);
    return sobel_image;
}


