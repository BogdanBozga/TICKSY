#include <vips/vips.h>


VipsImage* grayscale(VipsImage* image) {
    VipsImage *copyImage;
    VipsImage *scRGB;
    VipsImage *grayImage;
    if (vips_copy(image, &copyImage, NULL))
        vips_error_exit(NULL);
    if (vips_sRGB2scRGB(copyImage, &scRGB, NULL))
        vips_error_exit(NULL);
    if (vips_scRGB2BW(scRGB, &grayImage, NULL))
        vips_error_exit(NULL);
    free(copyImage);
    free(scRGB);

    return grayImage;
}

VipsImage* rotation270(VipsImage* image,int angle) {
    VipsImage *rotatedImage = NULL;
    VipsImage *copy;
    if (vips_copy(image, &copy, NULL))
        vips_error_exit(NULL);

    if(vips_rotate(copy,&rotatedImage,angle, NULL)){
        vips_error_exit("F;uck");
    }
    // copy->rotate(90,VipsInterpolate);

    // if (vips_rot270(copy, &rotated270, NULL))
    //     vips_error_exit(NULL);

    return rotatedImage;
}





int main( int argc, char **argv ) {
const char *filename = "test.jpg";
VipsImage *in = vips_image_new_from_file (filename, NULL); 
printf("Image readed\n");
VipsImage *out = rotation270(in,180);
if (vips_image_write_to_file( out, "out_images/newmodf.jpg", NULL))
        vips_error_exit(NULL);

    printf("Image proccesing ended\n");

}