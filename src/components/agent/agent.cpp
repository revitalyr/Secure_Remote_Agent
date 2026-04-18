// agent/agent.cpp
#include <QCoreApplication>
#include <QPluginLoader>
#include <QDir>
#include <QDebug>
#include "../../core/ipc_client.h"
#include "../loader/plugin_interface.h"

class Agent : public QObject {
    Q_OBJECT

public:
    Agent(QObject* parent = nullptr) : QObject(parent) {
        m_ipcClient = new IpcClient(this);
        connect(m_ipcClient, &IpcClient::connected, this, &Agent::onConnected);
        connect(m_ipcClient, &IpcClient::disconnected, this, &Agent::onDisconnected);
    }

    void start() {
        qDebug() << "Agent starting...";
        loadPlugins();
        m_ipcClient->connectToService("local:service_ipc");
    }

    void stop() {
        qDebug() << "Agent stopping...";
        unloadPlugins();
        m_ipcClient->disconnectFromService();
    }

    QJsonObject getSystemMetrics() {
        QJsonObject metrics;
        
        // Request metrics from service via IPC
        if (m_ipcClient->isConnected()) {
            metrics = m_ipcClient->getSystemMetrics();
        } else {
            qWarning() << "Agent: Not connected to service";
            metrics["error"] = "Not connected to service";
        }
        
        return metrics;
    }

private:
    void loadPlugins() {
        QString pluginsDir = QCoreApplication::applicationDirPath() + "/plugins";
        QDir dir(pluginsDir);
        
        if (!dir.exists()) {
            qDebug() << "Agent: Plugins directory not found:" << pluginsDir;
            return;
        }
        
        QStringList pluginFiles = dir.entryList(QStringList() << "*_plugin.dll", QDir::Files);
        
        for (const QString& pluginFile : pluginFiles) {
            QString pluginPath = dir.absoluteFilePath(pluginFile);
            QPluginLoader* loader = new QPluginLoader(pluginPath);
            
            if (loader->load()) {
                QObject* pluginObj = loader->instance();
                IPlugin* plugin = qobject_cast<IPlugin*>(pluginObj);
                
                if (plugin) {
                    plugin->initialize();
                    plugin->start();
                    m_loadedPlugins.push_back(loader);
                    qDebug() << "Agent: Loaded plugin:" << pluginFile;
                } else {
                    loader->unload();
                    delete loader;
                    qDebug() << "Agent: Failed to cast plugin:" << pluginFile;
                }
            } else {
                qDebug() << "Agent: Failed to load plugin:" << pluginFile << "Error:" << loader->errorString();
                delete loader;
            }
        }
    }

    void unloadPlugins() {
        for (QPluginLoader* loader : m_loadedPlugins) {
            QObject* pluginObj = loader->instance();
            IPlugin* plugin = qobject_cast<IPlugin*>(pluginObj);
            if (plugin) {
                plugin->stop();
                plugin->cleanup();
            }
            loader->unload();
            delete loader;
        }
        m_loadedPlugins.clear();
    }

    void onConnected() {
        qDebug() << "Agent: Connected to service";
    }

    void onDisconnected() {
        qDebug() << "Agent: Disconnected from service";
    }

    IpcClient* m_ipcClient;
    QVector<QPluginLoader*> m_loadedPlugins;
};

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Secure Remote Agent ===";
    qDebug() << "Agent component for demo";
    
    Agent agent;
    agent.start();
    
    return app.exec();
}

#include "agent.moc"
