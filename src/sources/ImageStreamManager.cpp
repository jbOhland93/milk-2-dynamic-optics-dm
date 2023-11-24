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
    errno_t ret = ImageStreamIO_openIm(mp_image,
                    p_appSettings->getISIOdmImName().c_str());
    if (ret == IMAGESTREAMIO_SUCCESS)
    {   // Success. Check compatibility.
        if (mp_image->md->naxis == 1
            && mp_image->md->size[0] == p_appSettings->getDmActuatorCount()
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
                << "ImageStreamMamager: image opened, but has wrong dimension, size or datatype."
                << std::endl;
            return false;
        }
    }
    else
    {
        std::cout
                << "ImageStreamMamager: failed to open image. "
                << "Creating new image."
                << std::endl;
        int naxis = 1;
        uint32_t * imsize = new uint32_t[naxis]();
        imsize[0] = p_appSettings->getDmActuatorCount();
        uint8_t atype = _DATATYPE_FLOAT;
        int shared = 1; // image will be in shared memory
        int NBkw = 0; // No keywords
        int circBufSize = 0;
        ret = ImageStreamIO_createIm_gpu(mp_image,
                                p_appSettings->getISIOdmImName().c_str(),
                                naxis,
                                imsize,
                                atype,
                                -1, // -1 = Host, >=0 = device
                                shared,
                                10,
                                NBkw,
                                MATH_DATA,
                                circBufSize);
        delete[] imsize;
        if (ret != IMAGESTREAMIO_SUCCESS)
        {
            std::cout
                << "ImageStreamMamager: failed to create image."
                << std::endl;
            delete mp_image;
            return false;
        }
        else
        {
            std::cout
                << "ImageStreamMamager: image successfully created."
                << std::endl;
            m_semaphoreIndex =
                ImageStreamIO_getsemwaitindex(mp_image, m_semaphoreIndex);
            return true;
        }
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
