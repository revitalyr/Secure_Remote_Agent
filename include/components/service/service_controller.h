#pragma once

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QVector>
#include <QAtomicInt>
#include <QPluginLoader>
#include <QDir>
#include <QRemoteObjectHost>
#include <QJsonObject>
#include "plugins/loader/plugin_interface.h"

class ServiceInterfaceSimpleSource;
class ServiceInterfaceSource;

class ServiceController : public QObject {
    Q_OBJECT

public:
    ServiceController(QObject* parent = nullptr);
    ~ServiceController();

    void start();
    void stop();
    void pause();
    void resume();

    bool isRunning() const { return m_running; }
    bool isPaused() const { return m_paused; }

    QString statusString() const;
    QJsonObject measureSystemMetrics();

void setApiKey(const QString& key);
void setBackendUrl(const QString& url);

signals:
    void started();
    void stopped();
    void paused();
    void resumed();

private:
    void initialize();
    void loadPlugins();
    void unloadPlugins();
    void sendRegistration();
    void sendHeartbeat();
    void sendMetrics();
    void sendPluginStatus();
    void pushMetricsToSource();
    void watchdogTick();

    QString m_apiAuthenticationKey;
    QString m_backendServiceUrl;
    

    QNetworkAccessManager* m_networkManager;
    QVector<QPluginLoader*> m_loadedPlugins;
    QAtomicInt m_running;
    QAtomicInt m_paused;
    qint64 m_startTime;
    qint64 m_lastHeartbeat;
    qint64 m_lastPluginStatus;
    QString backendServiceUrl;
    QString apiAuthenticationKey;

    void addAuthHeader(QNetworkRequest& request);

    QRemoteObjectHost* m_qtroHost;
    ServiceInterfaceSimpleSource* m_serviceSource;
    QTimer* m_watchdogTimer;
    int m_watchdogMissed;
    QString m_serviceStatus;
    double m_cpuUsage;
    double m_memoryUsage;
    double m_memoryTotal;
    double m_memoryAvailable;
    double m_netRxKbps;
    double m_netTxKbps;
    int m_uptime;

public:
    int uptime() const;
    void setUptime(int v);
    void ping();
    void requestShutdown();
    void setServiceStatus(const QString& status);
    void setCpuUsage(double usage);
    void setMemoryUsage(double usage);
    void setMemoryTotal(double total);
    void setMemoryAvailable(double available);
    void setNetRxKbps(double rx);
    void setNetTxKbps(double tx);

signals:
    void uptimeChanged(int v);
    void pongReceived(const QString& message);
    void logMessage(const QString& level, const QString& message);

private:
    void setAll(const QString& status, double cpu, double mem, double memTotal, double memAvail, double netRx, double netTx, int uptime);
};

