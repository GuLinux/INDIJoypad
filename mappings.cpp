#include "mappings.h"
#include <QFile>
#include <QCoreApplication>
#include <QJsonDocument>
#include "action.h"

using namespace std::placeholders;

Mappings::Mappings(INDIClient &indiClient, JoyStickDriver &joystickDriver) : indiClient{indiClient}, joystickDriver{joystickDriver}
{
    joystickDriver.setJoystickCallback(std::bind(&Mappings::joystickCallback, this, _1, _2, _3));
    joystickDriver.setAxisCallback(std::bind(&Mappings::axisCallback, this, _1, _2));
    joystickDriver.setButtonCallback(std::bind(&Mappings::buttonCallback, this, _1, _2));
}

void Mappings::load(const QString &filename)
{
    joypadsMappings.clear();
    QFile mappingsFile(filename);
    mappingsFile.open(QIODevice::ReadOnly);
    auto jsonMappings = QJsonDocument::fromJson(mappingsFile.readAll()).toVariant().toMap();
    for(auto joypad: jsonMappings.keys()) {
        auto joypadMap = jsonMappings[joypad].toMap();
        for(auto indiServer: joypadMap.keys()) {
            auto indiServerMap = joypadMap[indiServer].toMap();
            for(auto trigger: indiServerMap.keys()) {
                auto actionMap = indiServerMap[trigger].toMap();
                Mapping mapping{
                    actionMap.take("action").toString(),
                    actionMap.take("deviceName").toString(),
                    actionMap.take("deviceType").toString(),
                    actionMap.contains("rotate") ? actionMap.take("rotate").toDouble() : 0,
                    actionMap.contains("invert") ? actionMap.take("invert").toBool() : false,
                    actionMap,
                };
                joypadsMappings[joypad][indiServer][trigger] = mapping;
            }
        }
    }
}

void Mappings::joystickCallback(int joystickNumber, double magnitude, double angle)
{
    auto mapping = this->mapping("joystick", joystickNumber);
    auto device = this->deviceFor(mapping);

    angle += mapping.rotate;
    while(angle < 0) angle += 360.0;
    while(angle > 360) angle -= 360.0;

    if(mapping.valid() && device) {
        Action<JoystickPayload> action{mapping.action, JoystickPayload{magnitude, angle}, mapping.parameters};
        QMetaObject::invokeMethod(qApp, [=] { device->onJoystick(action); });
    }
}

void Mappings::axisCallback(int axisNumber, double value)
{
    auto mapping = this->mapping("axis", axisNumber);
    auto device = this->deviceFor(mapping);

    if(mapping.valid() && device) {
        value = value/32767 * (mapping.invert ? -1 : +1);
        Action<AxisPayload> action{mapping.action, AxisPayload{abs(value), value<0 ? AxisPayload::BACKWARD : AxisPayload::FORWARD}, mapping.parameters};
        QMetaObject::invokeMethod(qApp, [=] { device->onAxis(action); });
    }
}

void Mappings::buttonCallback(int buttonNumber, int value)
{
    auto mapping = this->mapping("button", buttonNumber);
    auto device = this->deviceFor(mapping);
    if(mapping.valid()) {
        bool pressed = value != 0;
        if(mapping.invert) {
            pressed = !pressed;
        }
        Action<ButtonPayload> action{mapping.action, ButtonPayload{pressed}, mapping.parameters};
        QMetaObject::invokeMethod(qApp, [=] { device->onButton(action); });
    }
}

Mappings::Mapping Mappings::mapping(const QString &type, int number) const
{
    auto key = QString("%1-%2").arg(type).arg(number);
    auto joypadName = QString(joystickDriver.getName()).trimmed();
    auto joypadMappings = this->joypadsMappings[joypadName];
    auto indiServerMappings = joypadMappings.value(indiClient.server(), joypadMappings.value("*"));
    return indiServerMappings[key];
}

INDIDevice::ptr Mappings::deviceFor(const Mapping &mapping) const
{
    if(mapping.deviceType == "telescope") {
        return indiClient.telescopes().value(mapping.deviceName);
    }
    if(mapping.deviceType == "focuser") {
        return indiClient.focusers().value(mapping.deviceName);
    }
    return INDIDevice::ptr();
}

