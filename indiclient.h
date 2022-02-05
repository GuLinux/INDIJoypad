#ifndef INDICLIENT_H
#define INDICLIENT_H

#include <QObject>
#include <libindi/baseclientqt.h>
#include <memory>
#include <QList>
#include "telescope.h"
#include "focuser.h"

class INDIClient : public INDI::BaseClientQt
{
    Q_OBJECT
public:
    typedef std::shared_ptr<INDIClient> ptr;
    typedef std::unique_ptr<INDIClient> uPtr;

    explicit INDIClient(const QString &server, QObject *parent = nullptr);

    void newDevice(INDI::BaseDevice *device) override;
    void removeDevice (INDI::BaseDevice *dp) override;
    void newProperty (INDI::Property *property) override;
    void removeProperty(INDI::Property *property) override;
    void newBLOB(IBLOB *bp) override;
    void newSwitch(ISwitchVectorProperty *svp) override;
    void newNumber(INumberVectorProperty *nvp) override;
    void newText(ITextVectorProperty *tvp) override;
    void newLight(ILightVectorProperty *lvp) override;
    void newMessage(INDI::BaseDevice *dp, int messageID) override;
    void serverConnected() override;
    void serverDisconnected(int exit_code) override;

    QString server() const { return _server; }
    QMap<QString, Telescope::ptr> telescopes() const { return _telescopes; }
    QMap<QString, Focuser::ptr> focusers() const { return _focusers; }
private slots:
    void deviceTypeDiscovery(INDI::BaseDevice *device);
private:
    QMap<QString, Telescope::ptr> _telescopes;
    QMap<QString, Focuser::ptr> _focusers;
    QString _server;
signals:
    void connected();
    void disconnected(int);
};

#endif // INDICLIENT_H
