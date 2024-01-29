#ifndef DMCONTROLLER_H
#define DMCONTROLLER_H

#include "AppSettings.h"
#include "ImageStruct.h"
#include <chrono>

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

    void setFrameRateCap(float fps);
    bool checkDataSize(int size);
    bool setActuatorValues(double* values);
    bool setActuatorValues(float* values);
    bool relaxDM();
    bool setCDoffset(double dcOffset);
    // Performs a given number of pokes in quick succession and the duration
    int64_t stressTest(int numPokes);

private:
    // Instance of the PztMulti API
    PztMulti* mp_driverInstance = nullptr;
    // Structure holding basic information about the API version
    PluginInfo m_pluginInfo;
    // Structure holding settings for the mirror driver
    ADPluginSettings m_dmSettings;
    // Buffer array for converting other types of input values
    double* mp_valBufferArr = nullptr;
    // Buffer array for applying a DC offset
    double* mp_offsetBufferArr = nullptr;
    // Framerate cap in Hz.
    // If <= 0, the isBusy() command will be used (=freerunning)
    float m_framerateCap_Hz = 500.;
    int64_t m_frameDtCap_us = (int64_t) (1e6 / m_framerateCap_Hz);
    std::chrono::_V2::system_clock::time_point m_lastFrame;

    // Offset for all actuators
    bool m_applyDCoffset = false;
    double m_dcOffset = 0;

    // == debugging ==
    IMAGE* mp_outputImage = nullptr;
    void setUpDebuggingImage(
        const char* postImName,// Name of output image
        int postImWidth);      // Width of output image - multiple lines if too small

    void forceInitialized(std::string actionName);
};

#endif // DMCONTROLLER_H