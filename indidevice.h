#ifndef INDIDEVICE_H
#define INDIDEVICE_H
#include <QObject>
#include <memory>
#include "action.h"

namespace INDI {
    class BaseDevice;
    class BaseClientQt;
}
class INDIDevice : public QObject
{
public:

    INDIDevice(INDI::BaseDevice *device);
    typedef std::shared_ptr<INDIDevice> ptr;
    QString name() const;
public slots:
    virtual void onJoystick(const Action<JoystickPayload> &action) = 0;
    virtual void onAxis(const Action<AxisPayload> &action) = 0;
    virtual void onButton(const Action<ButtonPayload> &action) = 0;
private:
    INDI::BaseDevice *m_device;

protected:
    INDI::BaseClientQt *client();
};

#endif // INDIDEVICE_H
