#ifndef DMCONTROLLER_H
#define DMCONTROLLER_H

#include "AppSettings.h"
#include "ImageStruct.h"

// DM library
#include "PztMultiInterface.h"
class DMController {
public:
    ~DMController();

    bool initialize(
        AppSettings* p_appSettings,                 // Settings for the DM
        bool postSetValues = true,                  // Write set values into output image for debugging
        const char* postImName = "milk-2-dm-output",// Name of output image
        int postImWidth = 12);                      // Width of output image - multiple lines if too small

    bool checkDataSize(int size);
    bool setActuatorValues(double* values);
    bool setActuatorValues(float* values);
    bool relaxDM();
    // Performs a given number of pokes in quick succession and returns telemetry
    bool stressTest(int numPokes,
        int64_t* duration_us_out = nullptr,
        int* missedPokes_out = nullptr);

private:
    // Instance of the PztMulti API
    PztMulti* mp_driverInstance = nullptr;
    // Structure holding basic information about the API version
    PluginInfo m_pluginInfo;
    // Structure holding settings for the mirror driver
    ADPluginSettings m_dmSettings;
    // Buffer array for converting other types of input values
    double* mp_valBufferArr = nullptr;

    // == debugging ==
    IMAGE* mp_outputImage = nullptr;
    void setUpDebuggingImage(
        const char* postImName,// Name of output image
        int postImWidth);      // Width of output image - multiple lines if too small

    void forceInitialized(std::string actionName);
};

#endif // DMCONTROLLER_H