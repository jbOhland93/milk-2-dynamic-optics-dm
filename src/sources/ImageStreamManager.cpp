#include "../headers/ImageStreamManager.h"
#include <ImageStreamIO.h>
#include <iostream>
#include <unistd.h>

ImageStreamManager::ImageStreamManager(AppSettings* p_appSettings)
    : mImageName(p_appSettings->getISIOdmImName().c_str())
{
    std::cout << "ImageStreamMamager: Image name: " << mImageName << std::endl;
}

ImageStreamManager::~ImageStreamManager()
{
    errno_t err = ImageStreamIO_closeIm(&mImage);
    if (err == 0)
        std::cout << "ImageStreamMamager: Image closed successfully." << std::endl;
    else
        std::cerr << "ImageStreamMamager: Error closing image: " << err << std::endl;
}

bool ImageStreamManager::initialize()
{
    errno_t ret = IMAGESTREAMIO_FAILURE;
    int attempts = 0;
    while (ret != IMAGESTREAMIO_SUCCESS)
    {
        ret = ImageStreamIO_openIm(&mImage, mImageName);
        if (ret != IMAGESTREAMIO_SUCCESS) {
            std::cout
                << "ImageStreamMamager: Attempt "
                << attempts
                << " Failed to open image "
                << mImageName
                << std::endl;
            attempts++;
            sleep(1);
        }
        else
            std::cout
                << "ImageStreamMamager: Successfully opened "
                << mImageName
                << " on attempt "
                << attempts
                << "!" << std::endl;
    }

    mSemaphoreIndex = ImageStreamIO_getsemwaitindex(&mImage, mSemaphoreIndex);
    
    return true;
}

void ImageStreamManager::waitForNextImage() {
    ImageStreamIO_semwait(&mImage, mSemaphoreIndex);
    std::cout << "ImageStreamMamager: New image published in the stream." << std::endl;
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
