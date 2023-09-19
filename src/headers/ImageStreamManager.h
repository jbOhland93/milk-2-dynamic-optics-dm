#ifndef IMAGESTREAMMANAGER_H
#define IMAGESTREAMMANAGER_H

#include <ImageStruct.h>

class ImageStreamManager {
public:
    ImageStreamManager(const char* image_name);
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