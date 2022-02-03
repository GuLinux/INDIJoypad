#include "telescope.h"
#include <QDebug>
#include <algorithm>

#include "mapping.h"

const QString Telescope::DIRECTION_N = "MOTION_NORTH";
const QString Telescope::DIRECTION_E = "MOTION_EAST";
const QString Telescope::DIRECTION_W = "MOTION_WEST";
const QString Telescope::DIRECTION_S = "MOTION_SOUTH";
const QString Telescope::AXIS_DEC = "TELESCOPE_MOTION_NS";
const QString Telescope::AXIS_RA = "TELESCOPE_MOTION_WE";


Telescope::Telescope(INDI::Telescope *telescope)
    : INDIDevice(telescope), indiTelescope(telescope)
{
    qDebug() << "Telescope: " << name();
}

bool Telescope::hasDevice(INDI::BaseDevice *other) const
{
    return other == this->indiTelescope;
}

void Telescope::slew(const QString &axis, const QString &direction)
{
    auto axisProperty = indiTelescope->getSwitch(axis.toStdString().c_str());
    axisProperty->reset();
    axisProperty->findWidgetByName(direction.toStdString().c_str())->setState(ISS_ON);
    client()->sendNewSwitch(axisProperty);
}

void Telescope::stopSlew(const QString &axis)
{
    auto axisProperty = indiTelescope->getSwitch(axis.toStdString().c_str());
    axisProperty->reset();
    client()->sendNewSwitch(axisProperty);
}

QString Telescope::magnitudeToSpeed(double magnitude) const
{
    auto slewRate = indiTelescope->getSwitch("TELESCOPE_SLEW_RATE");
    int rateIndex = (slewRate->nsp * abs(magnitude))-1;
    if(rateIndex > -1) {
        return slewRate->at(rateIndex)->name;
    }
    return slewRate->at(0)->name;
}

void Telescope::setSlewSpeed()
{
    auto slewRate = indiTelescope->getSwitch("TELESCOPE_SLEW_RATE");
    slewRate->reset();
    QString speed;
    std::for_each(slewRate->begin(), slewRate->end(), [&speed, this] (auto &s) {
        if(axisSpeed.values().contains(s.name)) {
            speed = s.name;
        }
    });
    if(!speed.isEmpty()) {
        slewRate->findWidgetByName(speed.toStdString().c_str())->setState(ISS_ON);
        client()->sendNewSwitch(slewRate);
    }
}

void Telescope::onJoystick(const Action &action, double magnitude, double angle)
{

}

void Telescope::onAxis(const Action &action, double value)
{
    if(action.action == "slew") {
        auto axis = action.parameters.value("axis") == "NS" ? AXIS_DEC : AXIS_RA;
        if(value == 0) {
            axisSpeed[axis] = QString();
            stopSlew(axis);
        } else {
            auto slewSpeed = action.parameters.value("speed", magnitudeToSpeed(std::abs(value))).toString();
            axisSpeed[axis] = slewSpeed;
            setSlewSpeed();
            int8_t valueSign = value < 0 ? -1 : 1;
            static const QMap<QString, QMap<int8_t, QString>> directions {
                { AXIS_DEC, { { 1, DIRECTION_N}, { -1, DIRECTION_S}}},
                { AXIS_RA, { { 1, DIRECTION_W}, { -1, DIRECTION_E}}},
            };
            slew(axis, directions[axis][valueSign]);
        }
    }
}

void Telescope::onButton(const Action &action, int value)
{

}
