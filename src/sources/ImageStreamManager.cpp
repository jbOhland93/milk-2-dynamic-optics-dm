#include "../headers/ImageStreamManager.h"
#include <ImageStreamIO.h>
#include <iostream>
#include <unistd.h>

ImageStreamManager::~ImageStreamManager()
{
    if (mp_image != nullptr)
    {
        errno_t err = ImageStreamIO_closeIm(mp_image);
        if (err == 0)
            std::cout << "ImageStreamMamager: image closed successfully." << std::endl;
        else
            std::cerr << "ImageStreamMamager: error closing image: " << err << std::endl;
    }
}

bool ImageStreamManager::initialize(AppSettings* p_appSettings)
{
    // Info for user
    std::cout   << "ImageStreamMamager: image name: "
                << p_appSettings->getISIOdmImName() << std::endl;

    // Close image if it previously existed
    if (mp_image != nullptr)
        ImageStreamIO_closeIm(mp_image);
    else
        mp_image = new IMAGE;

    // Try to open the image
    bool success = false;
    while(!success)
    {
        errno_t ret = ImageStreamIO_openIm(mp_image,
                    p_appSettings->getISIOdmImName().c_str());
        if (ret == 0)
            success = true;
        else
        {
            std::cout << "Opening image failed, retry in a second ...\n";
            sleep(1);
        }
    }
    // Check compatibility.
    if (mp_image->md->nelement == p_appSettings->getDmActuatorCount()
        && mp_image->md->datatype == _DATATYPE_FLOAT)
    {
        std::cout
            << "ImageStreamMamager: image successfully opened."
            << std::endl;
        return true;
    }
    else
    {
        std::cout
            << "ImageStreamMamager: image opened, but has wrong size or datatype:"
            << std::endl;
        std::cout << "\t nelement is " << mp_image->md->nelement
            << " and should be " << p_appSettings->getDmActuatorCount()
            << "." << std::endl;
        std::cout << "\t datatype is " << (int) mp_image->md->datatype
            << "and should be " << (int) _DATATYPE_FLOAT
            << " (float)." << std::endl;
        return false;
    }
}

bool ImageStreamManager::waitForNextImage(int timeout_us) {
    clock_gettime(CLOCK_REALTIME, &m_ISIOtimeout);
    m_ISIOtimeout.tv_nsec += (__time_t) timeout_us*1000;

    ImageStreamIO_semtimedwait(mp_image, m_semaphoreIndex, &m_ISIOtimeout);
    if (mp_image->md->cnt0 != m_lastCnt0)
    {
        m_lastCnt0 = mp_image->md->cnt0;
        ImageStreamIO_semflush(mp_image, m_semaphoreIndex);
        return true;
    }
    else
        return false;
}

float* ImageStreamManager::getData()
{
    return (float*) ImageStreamIO_get_image_d_ptr(mp_image);
}

int ImageStreamManager::getDataSize()
{
    return mp_image->md->nelement;
}
