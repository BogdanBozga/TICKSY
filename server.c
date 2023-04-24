#include <vips/vips.h>

VipsImage* myCopy(VipsImage *image){
    VipsImage *copyImage;
    if (vips_copy(image, &copyImage, NULL))
        vips_error_exit(NULL);
    return copyImage;
}

VipsImage* grayscale(VipsImage* image) {
    VipsImage *copyImage = myCopy(image);
    VipsImage *scRGB;
    VipsImage *grayImage;
    if (vips_sRGB2scRGB(copyImage, &scRGB, NULL))
        vips_error_exit("Fail to trasform RGB2 to RGB");
    if (vips_scRGB2BW(scRGB, &grayImage, NULL))
        vips_error_exit("Fail to trasform RGB to BW(gray)");
    free(copyImage);
    free(scRGB);

    return grayImage;
}

VipsImage* rotation270(VipsImage* image,int angle) {
    VipsImage *rotatedImage = NULL;
    VipsImage *copyImage = myCopy(image);
    if(vips_rotate(copy,&rotatedImage,angle, NULL)){
        vips_error_exit("Fail to ratate the image.");
    }
    free(copyImage);
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