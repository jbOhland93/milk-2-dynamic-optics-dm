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
    // Parse arguments, initialize settings
    AppSettings* p_appSettings = nullptr;
    bool provideDebugOutput = false;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-d" || arg == "--debug")
            provideDebugOutput = true;
        else
            p_appSettings = new AppSettings(arg);
    }
    if (p_appSettings == nullptr)
        p_appSettings = new AppSettings();
    p_appSettings->printSettings();

    if (p_appSettings->getCPUcore() < 0)
        std::cout << "Execution CPU core not specified in settings. Continue ...\n";
    else if (p_appSettings->getCPUcore() >= std::thread::hardware_concurrency())
    {
        std::cout   << "Execution CPU core specified in settings exceeds number of physical cores. "
                    << "\nAborting ...\n";
        return 1;
    }
    else
    {
        std::cout   << "Waiting to be scheduled to execution CPU core("
                    << p_appSettings->getCPUcore() << ")...\n";
        while (sched_getcpu() != p_appSettings->getCPUcore())
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout   << "done!\n";
    }

    // Initialize DM Controller
    std::shared_ptr<DMController> p_DMC
        = std::shared_ptr<DMController>(
            new DMController() );
    if (!p_DMC->initialize(p_appSettings, provideDebugOutput))
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
