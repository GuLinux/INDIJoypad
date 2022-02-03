#ifndef FOCUSER_H
#define FOCUSER_H
#include "indidevice.h"
#include <libindi/indifocuser.h>
class QTimer;
class Focuser : public INDIDevice
{
    Q_OBJECT
public:
    Focuser(INDI::Focuser *focuser);
    ~Focuser();
    typedef std::shared_ptr<Focuser> ptr;
    // INDIDevice interface
public slots:
    void onJoystick(const Action &action, double magnitude, double angle);
    void onAxis(const Action &action, double value);
    void onButton(const Action &action, int value);
private:
    void moveFocuser();
    INDI::Focuser *indiFocuser;
    std::unique_ptr<QTimer> repeatTimer;
    int newSteps;
    enum { INWARDS = -1, OUTWARDS = 1 } newDirection;
};

#endif // FOCUSER_H
