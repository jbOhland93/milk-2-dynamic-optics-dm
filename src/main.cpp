#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include "headers/AppSettings.h"
#include "headers/dmController.h"
#include "headers/ImageStreamManager.h"
#include "headers/UserInputHandler.h"

using namespace std;

int main(int argc, char *argv[]) {
    // Initialize settings
    AppSettings* p_appSettings;
    if (argc > 1)
        p_appSettings = new AppSettings(argv[1]);
    else
        p_appSettings = new AppSettings();
    p_appSettings->printSettings();

    // Initialize DM Controller
    std::shared_ptr<DMController> p_DMC
        = std::shared_ptr<DMController>(
            new DMController() );
    if (!p_DMC->initialize(p_appSettings))
    {
        cout << "Error initializing driver. Exiting programm." << endl;
        return 1;
    }

    // Initialize Image Stream Handler
    std::shared_ptr<ImageStreamManager> p_ISM
        = std::shared_ptr<ImageStreamManager>(
            new ImageStreamManager() );
    if (!p_ISM->initialize(p_appSettings))
    {
        cout << "Error initializing image manager. Exiting programm." << endl;
        return 1;
    }
    
    // Start the CLI
    UserInputHandler ui(p_DMC, p_ISM);
    std::thread uiThread = thread(&UserInputHandler::core, &ui);
    uiThread.join();

    delete p_appSettings;
}
