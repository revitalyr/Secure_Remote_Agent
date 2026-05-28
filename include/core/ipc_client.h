#pragma once

#include <QObject>
#include <QJsonObject>
#include <QRemoteObjectNode>

class ServiceInterfaceReplica;

class IpcClient : public QObject {
    Q_OBJECT

public:
    IpcClient(QObject* parent = nullptr);
    ~IpcClient();

    void connectToService(const QString& serverName);
    void disconnectFromService();
    bool isConnected() const;
    void ping();
    QJsonObject getSystemMetrics();

    QString serviceStatus() const;
    double cpuUsage() const;
    double memoryUsage() const;
    double memoryTotal() const;
    double memoryAvailable() const;
    double netRxKbps() const;
    double netTxKbps() const;
    int uptime() const;

signals:
    void connected();
    void disconnected();
    void pongReceived(const QString& message);
    void logMessage(const QString& level, const QString& message);
    void metricsUpdated();
    void netMetricsUpdated();

private slots:
    void onReplicaInitialized();
    void onReplicaPongReceived(const QString& message);
    void onReplicaLogMessage(const QString& level, const QString& message);

private:
    QRemoteObjectNode* m_node;
    ServiceInterfaceReplica* m_replica;
};
