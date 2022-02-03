#include "indiclient.h"
#include <QDebug>
#include <QTimer>

#include <libindi/basedevice.h>

INDIClient::INDIClient(const QString &server, QObject *parent)
    : INDI::BaseClientQt{parent}
{
    auto serverComponents = server.split(":", Qt::SkipEmptyParts);
    auto serverPort = serverComponents.size() > 1 ? serverComponents[1].toInt() : 7624;
    auto serverAddress = serverComponents[0];
    qDebug() << "Created new INDI client: " << serverAddress << serverPort;
    setServer(serverAddress.toLatin1(), serverPort);
    // setVerbose(true);
    this->_server = QString("%1:%2").arg(serverAddress).arg(serverPort);
}

void INDIClient::deviceTypeDiscovery(INDI::BaseDevice *device)
{
    qDebug() << "deviceTypeDiscovery: " << device->getDeviceName() << device->getDriverInterface();
    if(device->getDriverInterface() & INDI::BaseDevice::TELESCOPE_INTERFACE) {
        _telescopes[device->getDeviceName()] = std::make_shared<Telescope>(reinterpret_cast<INDI::Telescope*>(device));
    }
    if(device->getDriverInterface() & INDI::BaseDevice::FOCUSER_INTERFACE) {
        _focusers[device->getDeviceName()] = std::make_shared<Focuser>(reinterpret_cast<INDI::Focuser*>(device));
    }
}

void INDIClient::newDevice(INDI::BaseDevice *device)
{
    qDebug() << "newDevice: " << device->getDeviceName() << device->getDriverInterface() << device->getDriverName();
    QTimer::singleShot(100, this, [this, device] () { this->deviceTypeDiscovery(device); } );
}

void INDIClient::removeDevice(INDI::BaseDevice *dp)
{
    qDebug() << "removeDevice: " << dp->getDeviceName();
    _telescopes.erase(std::remove_if(_telescopes.begin(), _telescopes.end(), [dp] (const Telescope::ptr &t) { return t->hasDevice(dp); }));
}

void INDIClient::newProperty(INDI::Property *property)
{
    qDebug() << "newProperty: " << property->getDeviceName() << property->getName() << property->getLabel() << property->getType();
}

void INDIClient::removeProperty(INDI::Property *property)
{

}

void INDIClient::newBLOB(IBLOB *bp)
{

}

void INDIClient::newSwitch(ISwitchVectorProperty *svp)
{

}

void INDIClient::newNumber(INumberVectorProperty *nvp)
{

}

void INDIClient::newText(ITextVectorProperty *tvp)
{

}

void INDIClient::newLight(ILightVectorProperty *lvp)
{

}

void INDIClient::newMessage(INDI::BaseDevice *dp, int messageID)
{

}

void INDIClient::serverConnected()
{
    qDebug() << "serverConnected";
}

void INDIClient::serverDisconnected(int exit_code)
{
    qDebug() << "serverDisconnected" << exit_code;

}

