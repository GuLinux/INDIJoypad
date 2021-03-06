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
        qDebug() << "MoveFocuser::absolute" << movingSteps << movingDirection;

        auto positionControl = absolutePositionProperty->at(0);
        auto currentPosition = positionControl->getValue();
        auto newPosition = currentPosition + (movingSteps * movingDirection);
        qDebug() << "Moving focuser to abs position " << newPosition;
        positionControl->setValue(newPosition);
        client()->sendNewNumber(absolutePositionProperty);
    } else if(relativePositionProperty) {
        qDebug() << "MoveFocuser::relative" << movingSteps << (movingDirection == OUTWARDS ? "OUTWARDS" : "INWARDS");
        auto direction = indiFocuser->getSwitch("FOCUS_MOTION");
        direction->reset();
        direction->findWidgetByName(movingDirection == OUTWARDS ? "FOCUS_OUTWARD" : "FOCUS_INWARD")->setState(ISS_ON);

        auto position = relativePositionProperty->at(0);
        position->setValue(movingSteps);

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
        movingSteps = (maxSteps - minSteps) * action.value.magnitude + minSteps;
    } else {
        movingSteps = action.parameters.value("steps", 1).toInt();
    }
    movingDirection = action.value.direction == AxisPayload::FORWARD ? OUTWARDS : INWARDS;

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
    qDebug() << "Focuser::onButton" << action.action << ", " << action.value.pressed << ", params=" << action.parameters;
    auto changeSteps = [this, &action] (bool increase) {
        QVariant steps = action.parameters.value("steps", 1);
        if(steps.type() == QVariant::List) {
            QVariantList sl = steps.toList();
            QList<uint16_t> stepsList;

            std::transform(sl.begin(), sl.end(), std::back_inserter(stepsList), [](const QVariant &v) { return v.toUInt(); });
            qDebug() << "changeSteps: configured steps=" << stepsList;
            stepsList.erase(std::remove_if(stepsList.begin(), stepsList.end(), [increase,this](uint16_t s){ return increase ? (s <= movingSteps) : (s >= movingSteps); }), stepsList.end());
            std::sort(stepsList.begin(), stepsList.end());

            if(stepsList.size() > 0) {
                movingSteps = increase ? stepsList.first() : stepsList.last();
                qDebug() << "changeSteps: result=" << movingSteps << "," << stepsList;
            }
        } else {
            int delta = steps.toInt() * (increase ? +1 : -1);
            int64_t newSteps = static_cast<int64_t>(movingSteps) + delta;
            movingSteps = std::max<int64_t>(std::min<int64_t>(newSteps, std::numeric_limits<uint16_t>::max()), 0);
            qDebug() << "changeSteps: result=" << newSteps << movingSteps;
        }
    };
    if(action.action == "increase-steps" && action.value.pressed) {
        changeSteps(true);
    } else if(action.action == "decrease-steps" && action.value.pressed) {
        changeSteps(false);
    } else if(action.action == "focus-in" && action.value.pressed) {
        movingDirection = INWARDS;
        if(action.parameters.contains("steps")) {
            movingSteps = action.parameters["steps"].toUInt();
        }
        moveFocuser();
    } else if(action.action == "focus-out" && action.value.pressed) {
        movingDirection = OUTWARDS;
        if(action.parameters.contains("steps")) {
            movingSteps = action.parameters["steps"].toUInt();
        }
        moveFocuser();
    }
}

/*
FOCUS_MOTION.FOCUS_INWARD=On
FOCUS_MOTION.FOCUS_OUTWARD=Off
REL_FOCUS_POSITION.FOCUS_RELATIVE_POSITION=0
ABS_FOCUS_POSITION.FOCUS_ABSOLUTE_POSITION=50000
*/

