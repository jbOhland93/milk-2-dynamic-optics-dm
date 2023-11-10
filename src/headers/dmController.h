#ifndef DMCONTROLLER_H
#define DMCONTROLLER_H

#include "PztMultiInterface.h"
class DMController {
public:
    DMController();
    ~DMController();

    bool initialize(std::string deviceIP);

    bool checkDataSize(int size);
    bool setActuatorValues(double *values);
    bool relaxDM();

private:
    // Instance of the PztMulti API
    PztMulti* mp_driverInstance = nullptr;
    // Structure holding basic information about the API version
    PluginInfo m_pluginInfo;
    // Structure holding settings for the mirror driver
    ADPluginSettings m_dmSettings;

    void forceInitialized(std::string actionName);
};

#endif // DMCONTROLLER_H