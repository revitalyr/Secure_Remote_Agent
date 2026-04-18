// plugins/sample_plugin.cpp
#include "plugin_interface.h"
#include <QObject>
#include <QDebug>

class SamplePlugin : public QObject, public IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPlugin_iid)
    Q_INTERFACES(IPlugin)

public:
    QString name() const override { return "SamplePlugin"; }

    void execute() override {
        qDebug() << "Plugin executed";
    }
};

