#pragma once
#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include <QProcess>
#include <QDateTime>
#include <QStringList>
#include "core/ipc_client.h"

class UiManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString serviceStatus READ serviceStatus NOTIFY serviceStatusChanged)
    Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY cpuUsageChanged)
    Q_PROPERTY(double memoryUsage READ memoryUsage NOTIFY memoryUsageChanged)
    Q_PROPERTY(double memoryTotal READ memoryTotal NOTIFY memoryTotalChanged)
    Q_PROPERTY(double memoryAvailable READ memoryAvailable NOTIFY memoryAvailableChanged)
    Q_PROPERTY(QStringList logEntries READ logEntries NOTIFY logEntriesChanged)
    Q_PROPERTY(QStringList connectedAgents READ connectedAgents NOTIFY connectedAgentsChanged)
    Q_PROPERTY(double netRxKbps READ netRxKbps NOTIFY netMetricsChanged)
    Q_PROPERTY(double netTxKbps READ netTxKbps NOTIFY netMetricsChanged)
    Q_PROPERTY(int uptime READ uptime NOTIFY uptimeChanged)

public:
    explicit UiManager(QObject *parent = nullptr);
    ~UiManager();

    Q_INVOKABLE void startService();
    Q_INVOKABLE void stopService();
    Q_INVOKABLE void restartService();
    Q_INVOKABLE void pingService();

    Q_INVOKABLE void loadPlugin(const QString& pluginPath);
    Q_INVOKABLE void unloadPlugin(const QString& pluginName);
    Q_INVOKABLE QVariantMap getPluginStatus(const QString& pluginName);

    Q_INVOKABLE void startAgent(const QString& agentId);
    Q_INVOKABLE void stopAgent(const QString& agentId);
    Q_INVOKABLE void restartAgent(const QString& agentId);

    QString serviceStatus() const;
    double cpuUsage() const;
    double memoryUsage() const;
    double memoryTotal() const;
    double memoryAvailable() const;
    QStringList logEntries() const;
    QStringList connectedAgents() const;
    double netRxKbps() const;
    double netTxKbps() const;
    int uptime() const;

signals:
    void serviceStatusChanged();
    void cpuUsageChanged();
    void memoryUsageChanged();
    void memoryTotalChanged();
    void memoryAvailableChanged();
    void logEntriesChanged();
    void connectedAgentsChanged();
    void uptimeChanged();
    void netMetricsChanged();
    void serviceConnected();
    void serviceDisconnected();
    void errorOccurred(const QString& error);

private slots:
    void updateMetrics();
    void updateLogStream();
    void checkServiceStatus();
    void onIpcConnected();
    void onIpcDisconnected();
    void onIpcPong(const QString& message);
    void onIpcLog(const QString& level, const QString& message);

private:
    void initializeMetrics();
    void addLogEntry(const QString& level, const QString& message);
    void connectToService();
    void disconnectFromService();

    IpcClient* m_ipcClient;
    bool m_serviceConnected;
    QString m_serviceStatus;

    double m_cpuUsage;
    double m_memoryUsage;
    double m_memoryTotal;
    double m_memoryAvailable;
    int m_uptime;
    QDateTime m_startTime;
    QStringList m_logEntries;
    double m_netRxKbps;
    double m_netTxKbps;
    QStringList m_connectedAgents;
    QVariantMap m_pluginStatuses;
    QTimer* m_metricsTimer;
    QTimer* m_logTimer;
    QTimer* m_statusTimer;

    static const int MAX_LOG_ENTRIES = 1000;
    static const int METRICS_UPDATE_INTERVAL = 2000;
    static const int LOG_UPDATE_INTERVAL = 1000;
    static const int STATUS_CHECK_INTERVAL = 5000;
};
