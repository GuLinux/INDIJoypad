#include "mapping.h"
#include <QFile>
#include <QCoreApplication>
#include <QJsonDocument>

using namespace std::placeholders;

QDebug operator<< (QDebug d, const Action &action) {
    d.nospace() << "{device=" << action.deviceName
                << ", type=" << action.deviceType
                <<", action=" << action.action
               << ", params=" << action.parameters << "}";
    return d.space();
}

Mapping::Mapping(INDIClient &indiClient, JoyStickDriver &joystickDriver) : indiClient{indiClient}, joystickDriver{joystickDriver}
{
    joystickDriver.setJoystickCallback(std::bind(&Mapping::joystickCallback, this, _1, _2, _3));
    joystickDriver.setAxisCallback(std::bind(&Mapping::axisCallback, this, _1, _2));
    joystickDriver.setButtonCallback(std::bind(&Mapping::buttonCallback, this, _1, _2));
}

void Mapping::load(const QString &filename)
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
                Action action{
                    actionMap.take("action").toString(),
                    actionMap.take("deviceName").toString(),
                    actionMap.take("deviceType").toString(),
                    actionMap.contains("invert") ? actionMap.take("invert").toBool() : false,
                    actionMap,
                };
                qDebug() << "Mapping action: " << action << "to indiServer" << indiServer << ", joypad: " << joypad;
                joypadsMappings[joypad][indiServer][trigger] = action;
            }
        }
    }
}

void Mapping::joystickCallback(int joystickNumber, double magnitude, double angle)
{

}

void Mapping::axisCallback(int axisNumber, double value)
{
    auto action = this->action("axis", axisNumber);
    auto device = this->deviceFor(action);
    if(action.invert) {
        value *= -1;
    }
    qDebug() << "action: " << action << ", device: " << (bool)device;
    if(action.valid() && device) {
        qDebug() << "Axis callback present: " << axisNumber << value << ": " << action;
        QMetaObject::invokeMethod(qApp, [=] {
            device->onAxis(action, value/32767);
        });
    }
}

void Mapping::buttonCallback(int buttonNumber, int value)
{

}

Action Mapping::action(const QString &type, int number) const
{
    auto key = QString("%1-%2").arg(type).arg(number);
    auto joypadName = QString(joystickDriver.getName()).trimmed();
    qDebug() << "searching for " << key << "in " << indiClient.server() << "/" << joypadName;
    auto joypadMappings = this->joypadsMappings[joypadName];
    auto indiServerMappings = joypadMappings.value(indiClient.server(), joypadMappings.value("*"));
    return indiServerMappings[key];
}

INDIDevice::ptr Mapping::deviceFor(const Action &action) const
{
    if(action.deviceType == "telescope") {
        return indiClient.telescopes().value(action.deviceName);
    }
    if(action.deviceType == "focuser") {
        return indiClient.focusers().value(action.deviceName);
    }
    return INDIDevice::ptr();
}




