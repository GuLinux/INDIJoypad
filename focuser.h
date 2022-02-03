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
private:
    void moveFocuser();
    INDI::Focuser *indiFocuser;
    std::unique_ptr<QTimer> repeatTimer;
    int newSteps;
    enum { INWARDS = -1, OUTWARDS = 1 } newDirection;

    // INDIDevice interface
public slots:
    void onJoystick(const Action<JoystickPayload> &action);
    void onAxis(const Action<AxisPayload> &action);
    void onButton(const Action<ButtonPayload> &action);
};

#endif // FOCUSER_H
