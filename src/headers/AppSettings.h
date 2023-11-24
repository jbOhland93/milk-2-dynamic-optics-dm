#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

class AppSettings {
public:
    AppSettings(const std::string& filepath = "../dm-settings-default");

    std::string getDMaddress() const { return m_dmAddress; }
    int getDmActuatorCount() const { return m_dmActuatorCount; }
    double getDmVoltageUpper() const { return m_dmVUpper; }
    double getDmVoltageLower() const { return m_dmVLower; }
    double getDmVoltageCenter() const { return m_dmVCenter; }
    std::string getISIOdmImName() const { return m_dmImName; }

    void printSettings();

private:
    std::string m_filepath;
    std::string m_dmAddress = "unset";
    int m_dmActuatorCount = 0;
    double m_dmVUpper = 1.0;
    double m_dmVLower = -1.0;
    double m_dmVCenter = 0;
    std::string m_dmImName = "unset";
};

#endif // APPSETTINGS_H