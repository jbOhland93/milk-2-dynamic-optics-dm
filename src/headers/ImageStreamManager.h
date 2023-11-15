#ifndef IMAGESTREAMMANAGER_H
#define IMAGESTREAMMANAGER_H

#include <ImageStruct.h>
#include "AppSettings.h"

class ImageStreamManager {
public:
    ~ImageStreamManager();

    bool initialize(AppSettings* p_appSettings);

    void waitForNextImage();
    float* getData();
    int getDataSize();

private:
    IMAGE* mp_image;
    uint_fast16_t m_semaphoreIndex = 0;
};

#endif // IMAGESTREAMMANAGER_H