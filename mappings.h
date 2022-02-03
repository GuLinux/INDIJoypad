#ifndef MAPPING_H
#define MAPPING_H
#include <QString>
#include <QMap>
#include <QVariant>
#include "indiclient.h"
#include "indidevice.h"
#include "joystickdriver.h"

struct JoystickPayload {
    double magnitude;
    double angle;
};

struct AxisPayload {
    double magnitude;
    enum { FORWARD = +1, BACKWARD = -1 } direction;
};

struct ButtonPayload {
    bool pressed;
};

struct Action {
    QString action;
    QString deviceName;
    QString deviceType;
    bool invert;
    QMap<QString, QVariant> parameters;
    bool valid() const { return !action.isEmpty() && !deviceName.isEmpty() && !deviceType.isEmpty(); }
};

class Mappings
{
public:
    Mappings(INDIClient &indiClient, JoyStickDriver &joystickDriver);
    void load(const QString &filename);
    typedef QMap<QString, Action> ActionMappings;
    typedef QMap<QString, ActionMappings> INDIMappings;
    typedef QMap<QString, INDIMappings> JoypadMappings;

private:
    INDIClient &indiClient;
    JoyStickDriver &joystickDriver;
    JoypadMappings joypadsMappings;

    void joystickCallback(int joystickNumber, double magnitude, double angle);
    void axisCallback(int axisNumber, double value);
    void buttonCallback(int buttonNumber, int value);
    Action action(const QString &type, int number) const;
    INDIDevice::ptr deviceFor(const Action &action) const;
};

#endif // MAPPING_H
