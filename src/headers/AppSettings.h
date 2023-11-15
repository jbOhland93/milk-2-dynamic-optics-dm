#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

class AppSettings {
public:
    AppSettings(const std::string& filepath = "../settings.txt");

    std::string getDMaddress() const { return m_dmAddress; }
    int getDmActuatorCount() const { return m_dmActuatorCount; }
    std::string getISIOdmImName() const { return m_dmImName; }

    void printSettings();

private:
    std::string m_filepath;
    std::string m_dmAddress = "unset";
    int m_dmActuatorCount = -1;
    std::string m_dmImName = "unset";
};

#endif // APPSETTINGS_H