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

void Focuser::onJoystick(const Action<JoystickPayload> &action)
{
}

void Focuser::onAxis(const Action<AxisPayload> &action)
{
    auto repeat = action.parameters.value("repeat", 0.1).toDouble();
    if(action.parameters.contains("steps-min") && action.parameters.contains("steps-max")) {
        auto minSteps = action.parameters.value("steps-min", 1).toInt();
        auto maxSteps = action.parameters.value("steps-max", 10).toInt();
        newSteps = (maxSteps - minSteps) * action.value.magnitude + minSteps;
    } else {
        newSteps = action.parameters.value("steps", 1).toInt();
    }
    newDirection = action.value.direction == AxisPayload::FORWARD ? OUTWARDS : INWARDS;

    if(action.value.magnitude == 0) {
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

void Focuser::onButton(const Action<ButtonPayload> &action)
{
    auto changeSteps = [this, &action] (bool increase) {
        QVariant steps = action.parameters.value("steps", 1);
        qDebug() << steps.type();
        if(steps.type() == QVariant::List) {
            qDebug() << "[list]changeSteps: " << steps.toList();
        } else {
            qDebug() << "[int]changeSteps: " << steps.toInt();
        }
    };
    if(action.action == "increase-steps") {
        changeSteps(true);
    } else if(action.action == "decrease-steps") {
        changeSteps(false);
    } else if(action.action == "focus-in") {
    } else if(action.action == "focus-out") {
    }
}

/*
FOCUS_MOTION.FOCUS_INWARD=On
FOCUS_MOTION.FOCUS_OUTWARD=Off
REL_FOCUS_POSITION.FOCUS_RELATIVE_POSITION=0
ABS_FOCUS_POSITION.FOCUS_ABSOLUTE_POSITION=50000
*/

