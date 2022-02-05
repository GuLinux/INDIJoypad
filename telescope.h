#ifndef TELESCOPE_H
#define TELESCOPE_H
#include <QMap>
#include <memory>
#include <libindi/inditelescope.h>
#include "indidevice.h"

class Telescope : public INDIDevice
{
    Q_OBJECT
public:
    typedef std::shared_ptr<Telescope> ptr;
    Telescope(INDI::Telescope *telescope);
    ~Telescope();
    bool hasDevice(INDI::BaseDevice *other) const;
    static const QString AXIS_RA;
    static const QString AXIS_DEC;
    static const QString DIRECTION_N;
    static const QString DIRECTION_S;
    static const QString DIRECTION_E;
    static const QString DIRECTION_W;

private:
    void slew(const QString &axis, const QString &direction);
    void stopSlew(const QString &axis);
    void reloadSpeed();
    void abort();
    QString magnitudeToSpeed(double magnitude) const;
    void setSlewSpeed();

    INDI::Telescope *indiTelescope;

    QMap<QString, QString> axisSpeed;

    // INDIDevice interface
public slots:
    void onAxis(const Action<AxisPayload> &action);
    void onButton(const Action<ButtonPayload> &action);
    void onJoystick(const Action<JoystickPayload> &action);
};

#endif // TELESCOPE_H
