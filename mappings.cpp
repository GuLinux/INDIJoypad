#include "mappings.h"
#include <QFile>
#include <QCoreApplication>
#include <QJsonDocument>
#include "action.h"
#include <yaml-cpp/yaml.h>

using namespace std::placeholders;

Mappings::Mappings(INDIClient &indiClient, JoyStickDriver &joystickDriver) : indiClient{indiClient}, joystickDriver{joystickDriver}
{
    joystickDriver.setJoystickCallback(std::bind(&Mappings::joystickCallback, this, _1, _2, _3));
    joystickDriver.setAxisCallback(std::bind(&Mappings::axisCallback, this, _1, _2));
    joystickDriver.setButtonCallback(std::bind(&Mappings::buttonCallback, this, _1, _2));
}

void Mappings::load(const QString &filename)
{
    if(filename.endsWith(".json")) {
        loadJSON(filename);
    } else if(filename.endsWith(".yaml") || filename.endsWith(".yml")) {
        loadYAML(filename);
    }
}

void Mappings::loadJSON(const QString &filename)
{
    joypadsMappings.clear();
    QFile mappingsFile(filename);
    mappingsFile.open(QIODevice::ReadOnly);
    loadJSON(mappingsFile.readAll());
}

void Mappings::loadJSON(const QByteArray &json)
{
    auto jsonMappings = QJsonDocument::fromJson(json).toVariant().toMap();
    loadMappings(jsonMappings);
}


template<typename T>
QVariant node2variant(const YAML::Node &node, std::function<QVariant(const T&)> convertFunction = [](const T &t){ return QVariant::fromValue(t); }) {
    try {
        return convertFunction(node.as<T>());
    } catch (YAML::BadConversion) {
        return QVariant{};
    }
}

QVariant yamlNode2Variant(const YAML::Node &node) {

    if(node.IsScalar()) {
        QVariant value;
        value = node2variant<bool>(node);
        // }
        if(!value.isValid()) {
            value = node2variant<double>(node);
        }
        if(!value.isValid()) {
            value = node2variant<std::string>(node, [](const std::string &s) { return QVariant::fromValue(QString::fromStdString(s)); });
        }
        return value;
    }
    if(node.IsSequence()) {
        QVariantList value;
        std::for_each(node.begin(), node.end(), [&value](const YAML::iterator::value_type &v) {
            value.append(yamlNode2Variant(v));
        });
        return value;
    }
    if(node.IsMap()) {
        QVariantMap value;
        std::for_each(node.begin(), node.end(), [&value](const YAML::iterator::value_type &items) {
            QString key = yamlNode2Variant(items.first).toString();
            value[key] = yamlNode2Variant(items.second);
        });
        return value;
    }
    return {};
}


void Mappings::loadYAML(const QString &filename)
{
    YAML::Node mapping = YAML::LoadFile(filename.toStdString());
    loadMappings(yamlNode2Variant(mapping).toMap());
}

void Mappings::loadMappings(const QVariantMap &mappings)
{
    std::for_each(mappings.constKeyValueBegin(), mappings.constKeyValueEnd(), [=](const auto &joypad){
        QString joypadKey = joypad.first;
        std::for_each(joypad.second.toMap().constKeyValueBegin(), joypad.second.toMap().constKeyValueEnd(), [=](const auto &indiServer){
            QString indiServerKey = indiServer.first;
            std::for_each(indiServer.second.toMap().constKeyValueBegin(), indiServer.second.toMap().constKeyValueEnd(), [=](const auto &trigger){
                QString triggerKey = trigger.first;
                QVariantMap actionMap = trigger.second.toMap();
                Mapping mapping{
                    actionMap.take("action").toString(),
                    actionMap.take("deviceName").toString(),
                    actionMap.take("deviceType").toString(),
                    actionMap.contains("rotate") ? actionMap.take("rotate").toDouble() : 0,
                    actionMap.contains("invert") ? actionMap.take("invert").toBool() : false,
                    actionMap,
                };
                joypadsMappings[joypadKey][indiServerKey][triggerKey] = mapping;
            });
        });
    });
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
    if(mapping.valid() && device) {
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

