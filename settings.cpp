#include "settings.h"
#include <QCoreApplication>

Settings::Settings() : qSettings{SETTINGS_ORG_NAME, SETTINGS_APP_NAME}
{

}

