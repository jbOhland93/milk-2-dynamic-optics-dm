#ifndef DMCONTROLLER_H
#define DMCONTROLLER_H

#include "AppSettings.h"

// DM library
#include "PztMultiInterface.h"
class DMController {
public:
    ~DMController();

    bool initialize(AppSettings* p_appSettings);

    bool checkDataSize(int size);
    bool setActuatorValues(double* values);
    bool setActuatorValues(float* values);
    bool relaxDM();

private:
    // Instance of the PztMulti API
    PztMulti* mp_driverInstance = nullptr;
    // Structure holding basic information about the API version
    PluginInfo m_pluginInfo;
    // Structure holding settings for the mirror driver
    ADPluginSettings m_dmSettings;
    // Buffer array for converting other types of input values
    double* mp_valBufferArr = nullptr;

    void forceInitialized(std::string actionName);
};

#endif // DMCONTROLLER_H