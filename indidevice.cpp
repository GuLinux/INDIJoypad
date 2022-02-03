#include "indidevice.h"
#include <libindi/basedevice.h>
#include <libindi/baseclientqt.h>


INDIDevice::INDIDevice(INDI::BaseDevice *device) : m_device(device)
{

}

QString INDIDevice::name() const
{
    return m_device->getDeviceName();
}

INDI::BaseClientQt *INDIDevice::client()
{
    return dynamic_cast<INDI::BaseClientQt*>(m_device->getMediator());
}
