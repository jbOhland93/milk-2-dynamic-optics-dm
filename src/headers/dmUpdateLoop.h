#ifndef DMUPDATELOOP_H
#define DMUPDATELOOP_H

#include "../headers/ImageStreamManager.h"
#include "../headers/dmController.h"

class DMUpdateLoop {
public:
    DMUpdateLoop(const char* image_name);
    ~DMUpdateLoop();

    void run();
private:
    ImageStreamManager mISManager;
    DMController mDMController;

    bool mInitSuccess = true;
};

#endif // DMUPDATELOOP_H