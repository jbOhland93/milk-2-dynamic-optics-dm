#include "../headers/dmController.h"
#include <stdio.h>

//#include "plugin.h"
//#include "ActuatorsDriverPlugin.h"

DMController::DMController()
{
}

DMController::~DMController()
{
    printf("Destroying dmController... Call pd_close() here.\n");
}

bool DMController::initialize()
{
    printf("Initializing dmController... Call pd_open(), pd_setSettings(), etc. here.\n");
    printf("\t=> Read settings from a file?\n");
    return true;
}


bool DMController::checkDataSize(int size) {
    printf("Checking data size ...\n");
    return size == size/* Expected size here */;
}

bool DMController::setActuatorValues(double *values) {
    printf("=> Setting values... Call pd_setValues(values, size) here.\n");
    return true;
}
