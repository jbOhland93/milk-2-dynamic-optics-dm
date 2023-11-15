#ifndef IMAGESTREAMMANAGER_H
#define IMAGESTREAMMANAGER_H

#include <ImageStruct.h>

#include "AppSettings.h"

class ImageStreamManager {
public:
    ImageStreamManager(AppSettings* p_appSettings);
    ~ImageStreamManager();

    bool initialize();

    void waitForNextImage();
    double* getData();
    int getDataSize();

private:
    const char* mImageName;
    IMAGE mImage;
    uint_fast16_t mSemaphoreIndex = 0;
};

#endif // IMAGESTREAMMANAGER_H