#include "../headers/dmController.h"
#include <stdio.h>
#include <iostream>
#include <thread>

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
}

bool DMController::initialize(AppSettings* p_appSettings)
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
    m_dmSettings.center    = 0.0;   // Center position
    m_dmSettings.lower     = -1.0;  // Lower limit for the actuator position
    m_dmSettings.upper     = 1.0;   // Upper limit for the actuator position

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
                 + std::chrono::microseconds(5);
        while(std::chrono::steady_clock::now() < end);
    }
    return setValues(mp_driverInstance, values, m_dmSettings.actuators) == 0;
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
    std::cout << "DMController: relaxing DM...";

    // Poll the device to check if it is busy with the operation
    while( isBusy(mp_driverInstance) ) {
        // Wait for 100 milliseconds before polling again
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << std::endl
        << "DMController: relaxation routine complete." << std::endl; 

    return true;
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
