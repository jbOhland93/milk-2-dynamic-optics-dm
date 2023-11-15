#ifndef IMAGESTREAMMANAGER_H
#define IMAGESTREAMMANAGER_H

#include <ImageStruct.h>
#include "AppSettings.h"

class ImageStreamManager {
public:
    ~ImageStreamManager();

    bool initialize(AppSettings* p_appSettings);

    // Waits a specified time for a new image
    // Returns true if a new image is available
    bool waitForNextImage(int timeout_us);
    float* getData();
    int getDataSize();

private:
    IMAGE* mp_image;
    uint_fast16_t m_semaphoreIndex = 0;
    timespec m_ISIOtimeout;
    int m_lastCnt0 = -1;
};

#endif // IMAGESTREAMMANAGER_H