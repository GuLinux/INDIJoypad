#include "telescope.h"
#include <QDebug>
#include <algorithm>

#include "mappings.h"

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

Telescope::~Telescope()
{
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

void Telescope::reloadSpeed()
{
    qDebug() << "Reloading slew speed";
    auto dec = indiTelescope->getSwitch(AXIS_DEC.toStdString().c_str());
    auto ra = indiTelescope->getSwitch(AXIS_RA.toStdString().c_str());
    auto decDirection = dec->findOnSwitch();
    auto raDirection = ra->findOnSwitch();
    dec->reset();
    ra->reset();
    client()->sendNewSwitch(dec);
    client()->sendNewSwitch(ra);
    setSlewSpeed();
    if(decDirection) {
        dec->findWidgetByName(decDirection->name)->setState(ISS_ON);
        client()->sendNewSwitch(dec);
    }
    if(raDirection) {
        ra->findWidgetByName(raDirection->name)->setState(ISS_ON);
        client()->sendNewSwitch(ra);
    }
}

void Telescope::abort()
{
    // TELESCOPE_ABORT_MOTION.ABORT
    auto abortProperty = indiTelescope->getSwitch("TELESCOPE_ABORT_MOTION");
    if(abortProperty) {
        auto abortSwitch = abortProperty->findWidgetByName("ABORT");
        if(abortSwitch) {
            abortSwitch->setState(ISS_ON);
            client()->sendNewSwitch(abortProperty);
        }
    }
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

void Telescope::onJoystick(const Action<JoystickPayload> &action)
{
}

void Telescope::onAxis(const Action<AxisPayload> &action)
{
    if(action.action == "slew") {
        auto axis = action.parameters.value("axis") == "NS" ? AXIS_DEC : AXIS_RA;
        if(action.value.magnitude == 0) {
            axisSpeed[axis] = QString();
            stopSlew(axis);
        } else {
            auto slewSpeed = action.parameters.value("speed", "keep");
            if(slewSpeed != "keep") {
                if(slewSpeed == "ramping") {
                    slewSpeed = magnitudeToSpeed(action.value.magnitude);
                }
                axisSpeed[axis] = slewSpeed.toString();
                setSlewSpeed();
            }

            static const QMap<QString, QMap<int8_t, QString>> directions {
                { AXIS_DEC, { { AxisPayload::FORWARD, DIRECTION_N}, { AxisPayload::BACKWARD, DIRECTION_S}}},
                { AXIS_RA, { { AxisPayload::FORWARD, DIRECTION_W}, { AxisPayload::BACKWARD, DIRECTION_E}}},
            };
            slew(axis, directions[axis][action.value.direction]);
        }
        return;
    }
}

void Telescope::onButton(const Action<ButtonPayload> &action)
{
    qDebug() << "onButton: " << action.action << action.value.pressed << action.parameters;
    if(action.action == "abort" && action.value.pressed) {
        abort();
        return;
    }
    if(action.action == "reload-speed" && action.value.pressed) {
        reloadSpeed();
        return;
    }
}
