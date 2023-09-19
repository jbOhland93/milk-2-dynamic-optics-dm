#ifndef DMCONTROLLER_H
#define DMCONTROLLER_H

class DMController {
public:
    DMController();
    ~DMController();

    bool initialize();

    bool checkDataSize(int size);
    bool setActuatorValues(double *values);
};

#endif // DMCONTROLLER_H