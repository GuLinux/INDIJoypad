#ifndef MAPPING_H
#define MAPPING_H
#include <QString>
#include <QMap>
#include <QVariant>
#include "indiclient.h"
#include "indidevice.h"
#include "joystickdriver.h"

class Mappings
{
public:
    Mappings(INDIClient &indiClient, JoyStickDriver &joystickDriver);
    void load(const QString &filename);

private:
    void loadJSON(const QString &filename);
    void loadYAML(const QString &filename);
    struct Mapping {
        QString action;
        QString deviceName;
        QString deviceType;
        double rotate;
        bool invert;
        QMap<QString, QVariant> parameters;
        bool valid() const { return !action.isEmpty() && !deviceName.isEmpty() && !deviceType.isEmpty(); }
    };
    typedef QMap<QString, Mapping> ConfiguredMappings;
    typedef QMap<QString, ConfiguredMappings> INDIMappings;
    typedef QMap<QString, INDIMappings> JoypadMappings;


    INDIClient &indiClient;
    JoyStickDriver &joystickDriver;
    JoypadMappings joypadsMappings;

    void joystickCallback(int joystickNumber, double magnitude, double angle);
    void axisCallback(int axisNumber, double value);
    void buttonCallback(int buttonNumber, int value);
    Mapping mapping(const QString &type, int number) const;
    INDIDevice::ptr deviceFor(const Mapping &mapping) const;
};

#endif // MAPPING_H
