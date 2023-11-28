#include "../headers/dmController.h"
#include <stdio.h>
#include <iostream>
#include <thread>
#include <cmath>
#include <ImageStreamIO.h>

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

    return true;
}

bool DMController::checkDataSize(int size)
{
    return size == m_dmSettings.actuators;
}

bool DMController::setActuatorValues(double *values)
{
    forceInitialized("setActuatorValues(double*)");
    // Poll the device to check if it is busy with the operation
    while( isBusy(mp_driverInstance) ) {
        // Wait for 5 microseconds before polling again
        // Use busysleep to avoid the scheduler kicking in
        auto end = std::chrono::steady_clock::now()
                 + std::chrono::microseconds(100);
        while(std::chrono::steady_clock::now() < end);
    }
    int retval = setValues(mp_driverInstance, values, m_dmSettings.actuators);
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
