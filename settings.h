#ifndef SETTINGS_H
#define SETTINGS_H
#include <QSettings>
#define APP_VERSION "0.1.0"
#define SETTINGS_ORG_NAME "GuLinux"
#define SETTINGS_APP_NAME "INDIJoypad"

class Settings
{
public:
    Settings();
    void reload();
private:
    QSettings qSettings;
};

#endif // SETTINGS_H
