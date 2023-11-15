#ifndef DMUPDATELOOP_H
#define DMUPDATELOOP_H

#include "../headers/ImageStreamManager.h"
#include "../headers/dmController.h"
#include "../headers/AppSettings.h"

class DMUpdateLoop {
public:
    DMUpdateLoop(AppSettings* p_appSettings);
    ~DMUpdateLoop();

    void run();
private:
    ImageStreamManager mISManager;
    DMController mDMController;

    bool mInitSuccess = true;
};

#endif // DMUPDATELOOP_H