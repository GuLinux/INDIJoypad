#include "focuser.h"
#include <libindi/baseclientqt.h>
#include "mappings.h"
#include <QTimer>

Focuser::Focuser(INDI::Focuser *focuser) : INDIDevice(focuser), indiFocuser(focuser)
{
}

Focuser::~Focuser()
{

}

void Focuser::onJoystick(const Action &action, double magnitude, double angle)
{
}

void Focuser::onAxis(const Action &action, double value)
{
    auto minSteps = action.parameters.value("steps-min", 1).toInt();
    auto maxSteps = action.parameters.value("steps-max", 10).toInt();
    auto repeat = action.parameters.value("repeat", 0.1).toDouble();

    newSteps = (maxSteps - minSteps) * abs(value) + minSteps;
    newDirection = value >= 0 ? OUTWARDS : INWARDS;

    qDebug() << name() << " onAxis: min=" << minSteps << ", max=" << maxSteps << ", repeat=" << repeat << ", steps=" << newSteps << ", direction=" << newDirection;

    if(value == 0) {
        qDebug() << "Focuser: stopping repeat motion";
        repeatTimer.reset();
    } else {
        if(repeat > 0 && !repeatTimer) {
            repeatTimer = std::make_unique<QTimer>();
            connect(repeatTimer.get(), &QTimer::timeout, this, &Focuser::moveFocuser);
            repeatTimer->start(repeat * 1000);

        }
        moveFocuser();
    }
}

void Focuser::onButton(const Action &action, int value)
{
}

void Focuser::moveFocuser()
{
    auto relativePositionProperty = indiFocuser->getNumber("REL_FOCUS_POSITION");
    auto absolutePositionProperty = indiFocuser->getNumber("ABS_FOCUS_POSITION");
    if(
        (relativePositionProperty && relativePositionProperty->getState() == IPS_BUSY) ||
        (absolutePositionProperty && absolutePositionProperty->getState() == IPS_BUSY)
       ) {
        qDebug() << "Focuser busy";
        return;
    }
    if(absolutePositionProperty) {
        qDebug() << "MoveFocuser::absolute" << newSteps << newDirection;

        auto positionControl = absolutePositionProperty->at(0);
        auto currentPosition = positionControl->getValue();
        auto newPosition = currentPosition + (newSteps * newDirection);
        qDebug() << "Moving focuser to abs position " << newPosition;
        positionControl->setValue(newPosition);
        client()->sendNewNumber(absolutePositionProperty);
    } else if(relativePositionProperty) {
        qDebug() << "MoveFocuser::relative" << newSteps << (newDirection == OUTWARDS ? "OUTWARDS" : "INWARDS");
        auto direction = indiFocuser->getSwitch("FOCUS_MOTION");
        direction->reset();
        direction->findWidgetByName(newDirection == OUTWARDS ? "FOCUS_OUTWARD" : "FOCUS_INWARD")->setState(ISS_ON);

        auto position = relativePositionProperty->at(0);
        position->setValue(abs(newSteps));

        client()->sendNewSwitch(direction);
        client()->sendNewNumber(relativePositionProperty);
    }
}

/*
FOCUS_MOTION.FOCUS_INWARD=On
FOCUS_MOTION.FOCUS_OUTWARD=Off
REL_FOCUS_POSITION.FOCUS_RELATIVE_POSITION=0
ABS_FOCUS_POSITION.FOCUS_ABSOLUTE_POSITION=50000
*/

