#ifndef ACTION_H
#define ACTION_H
#include <QMap>
#include <QString>
#include <QVariant>

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

template<typename ValueType>
struct Action {
    QString action;
    ValueType value;
    QMap<QString, QVariant> parameters;
};


#endif // ACTION_H
