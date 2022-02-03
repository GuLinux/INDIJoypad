#ifndef INDIDEVICE_H
#define INDIDEVICE_H
#include <QObject>
#include <memory>

namespace INDI {
    class BaseDevice;
    class BaseClientQt;
}
struct Action;
class INDIDevice : public QObject
{
public:

    INDIDevice(INDI::BaseDevice *device);
    typedef std::shared_ptr<INDIDevice> ptr;
    QString name() const;
public slots:
    virtual void onJoystick(const Action &action, double magnitude, double angle) = 0;
    virtual void onAxis(const Action &action, double value) = 0;
    virtual void onButton(const Action &action, int value) = 0;
private:
    INDI::BaseDevice *m_device;

protected:
    INDI::BaseClientQt *client();
};

#endif // INDIDEVICE_H
