#include "../headers/dmController.h"
#include <stdio.h>
#include <iostream>
#include <thread>
#include <cmath>
#include <ImageStreamIO.h>

using namespace std::chrono;

#define passOrReturn(check, errmsg, retval) \
    if (check)                              \
    {                                       \
        std::cerr << errmsg << std::endl;   \
        return retval;                      \
    }

DMController::~DMController()
{
    // Safely destroy the PztMulti instance and disconnect from the device
    if (mp_driverInstance != nullptr)
    {
        closePztMulti(mp_driverInstance);
        destroyPztMultiInstance(mp_driverInstance);
        std::cout << "DMController: disconnected from DM!" << std::endl; 

        delete[] mp_valBufferArr;
    }

    if (mp_outputImage != nullptr)
    {
        ImageStreamIO_closeIm(mp_outputImage);
        ImageStreamIO_destroyIm(mp_outputImage);
        delete mp_outputImage;
    }
}

bool DMController::initialize(
    AppSettings* p_appSettings,
    bool postSetValues,
    const char* postImName,
    int postImWidth)
{
    passOrReturn(mp_driverInstance != nullptr,
        "DMController: already initialized.", false);

    std::cout   << "DMController: device ip: "
                << p_appSettings->getDMaddress() << std::endl;
    std::cout << "DMController: Creating PztMulti instance..." << std::endl; 
    mp_driverInstance = createPztMultiInstance(p_appSettings->getDMaddress());

    std::cout << "DMController: getting plugin info..." << std::endl; 
    passOrReturn(getInfo(mp_driverInstance, m_pluginInfo),
        "DMController: failed to get plugin info.", false);
    std::cout << "\tName: " << m_pluginInfo.name << std::endl;
    std::cout << "\tFile Name: " << m_pluginInfo.filename << std::endl;
    std::cout << "\tVersion: " << m_pluginInfo.version << std::endl;
    std::cout << "\tType: " << m_pluginInfo.pluginType << std::endl;

    // Initialize the plugin settings
    // Number of actuators
    m_dmSettings.actuators = p_appSettings->getDmActuatorCount();
    m_dmSettings.center    = p_appSettings->getDmVoltageCenter();
    m_dmSettings.lower     = p_appSettings->getDmVoltageLower();
    m_dmSettings.upper     = p_appSettings->getDmVoltageUpper();

    mp_valBufferArr = new double[m_dmSettings.actuators];

    // Attempt to open the PztMulti instance with the given settings
    std::cout << "DMController: opening PztMulti instance..." << std::endl; 
    passOrReturn(openPztMulti(mp_driverInstance, m_dmSettings),
        "DMController: failed to open PztMulti instance.", false);
    
    // Retrieve the settings from the device to ensure they are correct
    ADPluginSettings hardwareSettings;
    passOrReturn(getSettings(mp_driverInstance, hardwareSettings),
        "DMController: failed to read settings from the device.", false);

    // Check the equality between the original and the retrieved settings
    bool equal = true;
    equal &= hardwareSettings.actuators == m_dmSettings.actuators;
    equal &= hardwareSettings.center == m_dmSettings.center;
    equal &= hardwareSettings.lower == m_dmSettings.lower;
    equal &= hardwareSettings.upper == m_dmSettings.upper;
    passOrReturn(!equal,
        "DMController: settings from device do not equal sent settings.", false);

    // Set up debugging image, if demanded
    if (postSetValues)
        setUpDebuggingImage(postImName, postImWidth);

    // Set framerate cap
    setFrameRateCap(p_appSettings->getDmFrameRateCap_Hz());

    return true;
}

void DMController::setFrameRateCap(float fps)
{
    m_framerateCap_Hz = fps;
    m_frameDtCap_us = (int64_t) (1e6 / fps);
}

bool DMController::checkDataSize(int size)
{
    return size == m_dmSettings.actuators;
}

bool DMController::setActuatorValues(double *values)
{
    forceInitialized("setActuatorValues(double*)");
    
    if (m_framerateCap_Hz <= 0)
    {   // Poll the device to check if it is busy with the operation
        while( isBusy(mp_driverInstance) ) {}
    }
    else
    {   // Wait until the framerate cap is respected
        // Prapare for precise busy sleep...
        _V2::system_clock::time_point currentTime;
        nanoseconds sinceLast;
        int64_t sinceLast_us;
        // ... but inprecisely wait for the majority of time in the regular way.
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
        // Do the busy sleep for the rest of the time for precision.
        do
        {
            currentTime = high_resolution_clock::now();
            sinceLast = currentTime - m_lastFrame;
            sinceLast_us = duration_cast<microseconds>(sinceLast).count();
        }
        while (m_frameDtCap_us > sinceLast_us);
        m_lastFrame = currentTime;
    }

    // Set the values to the DM
    int retval = setValues(mp_driverInstance, values, m_dmSettings.actuators);

    // Copy the values to the output image
    if (mp_outputImage != nullptr)
    {
        double* dst = (double*) ImageStreamIO_get_image_d_ptr(mp_outputImage);
        for (int i = 0; i < m_dmSettings.actuators; i++)
            dst[i] = values[i];
        ImageStreamIO_UpdateIm(mp_outputImage);
    }
    return retval == 0;
}

bool DMController::setActuatorValues(float *values)
{
    forceInitialized("setActuatorValues(float*)");

    for (int i = 0; i < m_dmSettings.actuators; i++)
        mp_valBufferArr[i] = (double) values[i];

    return setActuatorValues(mp_valBufferArr);
}

bool DMController::relaxDM()
{
    forceInitialized("relaxDM()");

    // Start the relaxation process
    relax(mp_driverInstance);

    // Poll the device to check if it is busy with the operation
    while( isBusy(mp_driverInstance) ) {
        // Wait for 100 milliseconds before polling again
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return true;
}

bool DMController::stressTest(
    int numPokes,
    int64_t* duration_us_out,
    int* missedPokes_out)
{
    // Initialize poke values
    float* poke1 = new float[m_dmSettings.actuators];
    float* poke2 = new float[m_dmSettings.actuators];
    for (int i = 0; i < m_dmSettings.actuators; i++)
    {
        poke1[i] = m_dmSettings.lower * 0.1;
        poke2[i] = m_dmSettings.upper * 0.1;
    }

    // Record start time
    auto start = std::chrono::high_resolution_clock::now();
    int errCnt = 0;

    // Perform pokes
    for (int i = 0; i < numPokes; i++)
        if (i%2)
            if (!setActuatorValues(poke1))
                errCnt++;
        else
            if (!setActuatorValues(poke2))
                errCnt++;

    // Record end time
    auto end = std::chrono::high_resolution_clock::now();

    // Write telemetry to given ptrs
    if (duration_us_out != nullptr)
        *duration_us_out = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    if (missedPokes_out != nullptr)
        *missedPokes_out = errCnt;
    
    // Clean up
    delete[] poke1;
    delete[] poke2;

    // Return true if no pokes were missed
    return errCnt == 0;
}

void DMController::setUpDebuggingImage(
        const char* postImName,
        int postImWidth)
{
    std::cout
        << "DMController: attempting to open image containing set values with name \""
        << postImName << "\"..." << std::endl;
    mp_outputImage = new IMAGE;
    errno_t ret = ImageStreamIO_openIm(mp_outputImage,
                postImName);
    if (ret == IMAGESTREAMIO_SUCCESS)
    {   // Success. Check compatibility.
        if (mp_outputImage->md->nelement >= m_dmSettings.actuators
            && mp_outputImage->md->datatype == _DATATYPE_DOUBLE)
        {
            std::cout
                << "DMController: output image successfully opened."
                << std::endl;
            return;
        }
        else
        {
            std::cout
                << "DMController: output image opened, but has insufficient size or wrong datatype."
                << std::endl;
            std::cout
                << "DMController: replacing image..."
                << std::endl;
            ImageStreamIO_closeIm(mp_outputImage);
            ImageStreamIO_destroyIm(mp_outputImage);
        }
    }
    else
        std::cout << "DMController: could not open output image." << std::endl;
    
    // Image could not be opened
    std::cout << "DMController: creating image containing set values with name \""
        << postImName << "\"..." << std::endl;
    int naxis = 2;
    uint32_t * imsize = new uint32_t[naxis]();
    imsize[0] = postImWidth;
    imsize[1] = (int) std::ceil(m_dmSettings.actuators / postImWidth);
    uint8_t atype = _DATATYPE_DOUBLE;
    int shared = 1; // image will be in shared memory
    int NBkw = 0; // No keywords
    int circBufSize = 0;
    ret = ImageStreamIO_createIm_gpu(mp_outputImage,
                            postImName,
                            naxis,
                            imsize,
                            atype,
                            -1, // -1 = Host, >=0 = device
                            shared,
                            10,
                            NBkw,
                            MATH_DATA,
                            circBufSize);
    delete[] imsize;
    
    if (ret == IMAGESTREAMIO_SUCCESS)
    {
        std::cout << "DMController: output image successfully created." << std::endl;
        return;
    }
    
    std::cout << "DMController: failed to create output image." << std::endl;
    delete mp_outputImage;
    mp_outputImage = nullptr;
}

void DMController::forceInitialized(std::string actionName)
{
    if(mp_driverInstance == nullptr)
    {
        std::string msg("DMController: action '");
        msg.append(actionName);
        msg.append("' not permitted on uninitialized driver.");
        throw std::runtime_error(msg);
    }
}
