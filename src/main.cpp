#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include "headers/dmUpdateLoop.h"
#include "headers/AppSettings.h"


int main(int argc, char *argv[]) {
    AppSettings* p_appSettings;
    
    if (argc > 1)
        p_appSettings = new AppSettings(argv[1]);
    else
        p_appSettings = new AppSettings();
    
    p_appSettings->printSettings();

    

    delete p_appSettings;
}