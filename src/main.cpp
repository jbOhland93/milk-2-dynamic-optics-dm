#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "headers/dmUpdateLoop.h"

DMUpdateLoop* loop = nullptr;

void terminate()
{
    if (loop != nullptr)
        delete loop;
    exit(1);
}

// Signal handler
void sigint_handler(int signo) {
    if (signo == SIGINT) {
        std::cout << "\nExiting due to manual termination (Ctrl+C)." << std::endl;
        terminate();
    }
}

int main(int argc, char *argv[]) {
    // Check argument validity
    if (argc < 3) {
        std::cerr
            << "Usage: "
            << argv[0]
            << " <image_name>"
            << " <device_ip>"
            << std::endl;
        return 1;
    }
    // Register signal handler for SIGINT
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        std::cout << "Cannot catch SIGINT" << std::endl;
    }

    loop = new DMUpdateLoop(argv[1], argv[2]);
    loop->run();

    terminate();
}