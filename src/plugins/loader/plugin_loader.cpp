// plugins/plugin_loader.cpp
#include <QPluginLoader>
#include <QDir>
#include "plugin_interface.h"

void loadPlugins() {
    QDir pluginsDir("plugins");

    for (const auto& file : pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(file));
        QObject* plugin = loader.instance();

        if (auto p = qobject_cast<IPlugin*>(plugin)) {
            p->execute();
        }
    }
}

