#include "../headers/AppSettings.h"
#include <iostream>

AppSettings::AppSettings(const std::string& filepath)
    : m_filepath(filepath)
 {
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
                else if (key == "DMcombStream") m_dmImName = value;
            }
        }
    }
}

void AppSettings::printSettings()
{
    std::cout   << "Application settings read from '"
                << m_filepath
                << "':" << std::endl;
    std::cout   << "\t DM IP address: " << m_dmAddress << std::endl;
    std::cout   << "\t DM actuator count: " << m_dmActuatorCount << std::endl;
    std::cout   << "\t DM comb stream: " << m_dmImName << std::endl;
}
