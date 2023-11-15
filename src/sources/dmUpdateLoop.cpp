#include "../headers/dmUpdateLoop.h"
#include <iostream>

DMUpdateLoop::DMUpdateLoop(AppSettings* p_appSettings)
    : mISManager(p_appSettings) {
    if (!mDMController.initialize(p_appSettings) || !mISManager.initialize()) {
        std::cerr << "ERROR: Initialization failed." << std::endl;
        mInitSuccess = false;
    }
    else
    {
        int dataSize = mISManager.getDataSize();
        if (!mDMController.checkDataSize(dataSize)) {
            std::cerr << "ERROR: Data size does not meet expectations." << std::endl;
            mInitSuccess = false;
        }
    }
}

DMUpdateLoop::~DMUpdateLoop() {
    // Cleanup logic here...
}

void DMUpdateLoop::run() {
    if (!mInitSuccess)
        return; // An error occured - return!

    // No error occured - run the loop!
    while(true)
    {
        mISManager.waitForNextImage();
        mDMController.setActuatorValues(mISManager.getData());
    }
}
