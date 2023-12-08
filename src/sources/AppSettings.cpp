#include "../headers/AppSettings.h"
#include <iostream>

#include <filesystem>

#include <unistd.h>
#include <limits>

AppSettings::AppSettings(const std::string& filepath)
    : m_filepath(filepath)
{
    char buffer[256];
    if (getcwd(buffer, sizeof(buffer)) != NULL)
        std::cout << "Current Working Directory: " << buffer << "\n";

    std::ifstream file(filepath);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '=')) {
            std::string value;
            if (std::getline(iss, value)) {
                if (key == "DM-IPAddress") m_dmAddress = value;
                else if (key == "actuatorCount") m_dmActuatorCount = std::stoi(value);
                else if (key == "actuatorVoltageUpper") m_dmVUpper = std::stod(value);
                else if (key == "actuatorVoltageLower") m_dmVLower = std::stod(value);
                else if (key == "actuatorVoltageCenter") m_dmVCenter = std::stod(value);
                else if (key == "framerateCap_Hz") m_dmFPScap_Hz = std::stof(value);
                else if (key == "DMcombStream") m_dmImName = value;
                else if (key == "CPUcore") m_cpuCore = std::stoi(value);
            }
        }
    }
}

void AppSettings::printSettings()
{
    std::cout   << "Application settings read from '"
                << m_filepath
                << "':" << "\n";
    std::cout   << "\t CPU core: " << m_cpuCore << "\n";
    std::cout   << "\t DM IP address: " << m_dmAddress << "\n";
    std::cout   << "\t DM actuator count: " << m_dmActuatorCount << "\n";
    std::cout   << "\t DM voltate upper limit: " << m_dmVUpper << "\n";
    std::cout   << "\t DM voltage center value: " << m_dmVCenter << "\n";
    std::cout   << "\t DM voltate lower limit: " << m_dmVLower << "\n";
    std::cout   << "\t DM frame rate cap: " << m_dmFPScap_Hz << " Hz" << "\n";
    std::cout   << "\t DM comb stream: " << m_dmImName << "\n";
}
