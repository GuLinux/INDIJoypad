#include "mainwindow.h"
#include "settings.h"

#include <QCoreApplication>
#include <QTimer>
#include <QCommandLineParser>
#include <QDebug>
#include <iostream>

#include "indiclient.h"
#include "joystickdriver.h"
#include "mappings.h"

class INDIJoypadCLI : public QObject {
    Q_OBJECT
public:
    INDIJoypadCLI(QCommandLineParser &parser) :
        QObject(nullptr),
        parser{parser},
        indiClient(parser.value("indi-server")),
        joystickDriver(),
        mapping(indiClient, joystickDriver)
    {
        joystickDriver.setPort(parser.value("joypad").toLatin1());
        connect(this, &INDIJoypadCLI::exit, qApp, &QCoreApplication::exit, Qt::QueuedConnection);
    }
    void run() {
        if(parser.isSet("joypad-capabilities")) {
            if(openJoypad()) {
                std::cout << joystickDriver.getName() << std::endl;
                std::cout << static_cast<int>(joystickDriver.getNumOfJoysticks()) << " joysticks" << std::endl;
                std::cout << static_cast<int>(joystickDriver.getNumOfAxes()) << " axes" << std::endl;
                std::cout << static_cast<int>(joystickDriver.getNumrOfButtons()) << " buttons" << std::endl;
                emit exit(0);
            }
            return;
        }
        if(parser.isSet("joypad-events")) {
            if(openJoypad()) {
                joystickDriver.setJoystickCallback([](int joystick, double mag, double value){
                    std::cout << "joystick-" << joystick << ", mag: " << mag << ", value: " << value << std::endl;
                });
                joystickDriver.setAxisCallback([](int axis, double value){
                    std::cout << "axis-" << axis << ", value: " << value << std::endl;
                });
                joystickDriver.setButtonCallback([](int button, int value){
                    std::cout << "button-" << button << ", value: " << value << std::endl;
                });
            }
            return;
        }
        if(!parser.isSet("mappings")) {
            std::cerr << "Error: mappings file not set" << std::endl;
            emit exit(1);
            return;
        }
        const QString indiServerAddress = parser.value("indi-server");
        qDebug() << "CLI: indiServerAddress=" << indiServerAddress;

        connect(&indiClient, &INDIClient::disconnected, this, &INDIJoypadCLI::exit, Qt::QueuedConnection);
        connect(&indiClient, &INDIClient::connected, this, &INDIJoypadCLI::startEventListening, Qt::QueuedConnection);
        indiClient.connectServer();
    }
public slots:

    void startEventListening() {
        qDebug() << "Starting event listening";
        mapping.load(parser.value("mappings"));
        openJoypad();
    }
private:

    bool openJoypad() {
        if(!joystickDriver.Connect()) {
            std::cerr << "Unable to open joypad at port " << parser.value("joypad").toStdString() << std::endl;
            emit exit(1);
            return false;
        }
        return true;
    }
private:
    QCommandLineParser &parser;
    INDIClient indiClient;
    JoyStickDriver joystickDriver;
    Mappings mapping;
signals:
    void exit(int);
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(SETTINGS_ORG_NAME);
    QCoreApplication::setApplicationName("INDIJoypad CLI");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription("INDI Jopypad controller");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{"i", "indi-server"}, "INDI server address", "indi-server", "localhost"});
    parser.addOption({{"j", "joypad"}, "Joypad device", "joypad", "/dev/input/js0"});
    parser.addOption({{"m", "mappings"}, "Mappings file", "mappings"});
    parser.addOption({"joypad-capabilities", "List joypad triggers"});
    parser.addOption({"joypad-events", "Show joypad events"});

    parser.process(app);


    INDIJoypadCLI indiJoypadCLI(parser);
    QMetaObject::invokeMethod(&indiJoypadCLI, &INDIJoypadCLI::run);
    return app.exec();
}
#include "main-cli.moc"

