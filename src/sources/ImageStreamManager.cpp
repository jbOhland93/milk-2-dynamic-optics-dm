#include "../headers/ImageStreamManager.h"
#include <ImageStreamIO.h>
#include <iostream>

ImageStreamManager::ImageStreamManager(const char* image_name)
    : mImageName(image_name)
{
    std::cout << "Image name: " << mImageName << std::endl;
}

ImageStreamManager::~ImageStreamManager()
{
    errno_t err = ImageStreamIO_closeIm(&mImage);
    if (err == 0)
        std::cout << "Image closed successfully." << std::endl;
    else
        std::cerr << "Error closing image: " << err << std::endl;
}

bool ImageStreamManager::initialize()
{
    errno_t ret;
    ret = ImageStreamIO_openIm(&mImage, mImageName);
    if (ret != IMAGESTREAMIO_SUCCESS) {
        printf("Failed to open image: %s\n", mImageName);
        return false;  // Print error message and exit
    }

    mSemaphoreIndex = ImageStreamIO_getsemwaitindex(&mImage, mSemaphoreIndex);
    
    return true;
}

void ImageStreamManager::waitForNextImage() {
    ImageStreamIO_semwait(&mImage, mSemaphoreIndex);
    printf("New image published in the stream.\n");
    ImageStreamIO_semflush(&mImage, mSemaphoreIndex);
}

double* ImageStreamManager::getData()
{
    return (double*) ImageStreamIO_get_image_d_ptr(&mImage);
}

int ImageStreamManager::getDataSize()
{
    return mImage.md->nelement;
}
