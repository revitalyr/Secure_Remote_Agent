#include "core/ipc_client.h"
#include "rep_ServiceInterface_merged.h"
#include <QDebug>
#include <QDateTime>

IpcClient::IpcClient(QObject* parent)
    : QObject(parent)
    , m_node(nullptr)
    , m_replica(nullptr)
{
}

IpcClient::~IpcClient() {
    disconnectFromService();
}

void IpcClient::connectToService(const QString& serverName) {
    if (m_node) {
        disconnectFromService();
    }

    m_node = new QRemoteObjectNode(this);
    m_node->connectToNode(QUrl(serverName));

    m_replica = m_node->acquire<ServiceInterfaceReplica>();

    connect(m_replica, &ServiceInterfaceReplica::initialized,
            this, &IpcClient::onReplicaInitialized);
    connect(m_replica, &ServiceInterfaceReplica::pongReceived,
            this, &IpcClient::onReplicaPongReceived);
    connect(m_replica, &ServiceInterfaceReplica::logMessage,
            this, &IpcClient::onReplicaLogMessage);
    connect(m_replica, &ServiceInterfaceReplica::serviceStatusChanged,
            this, [this]() { emit metricsUpdated(); });
    connect(m_replica, &ServiceInterfaceReplica::cpuUsageChanged,
            this, [this]() { emit metricsUpdated(); });
    connect(m_replica, &ServiceInterfaceReplica::memoryUsageChanged,
            this, [this]() { emit metricsUpdated(); });
    connect(m_replica, &ServiceInterfaceReplica::netRxKbpsChanged,
            this, [this]() { emit netMetricsUpdated(); });
    connect(m_replica, &ServiceInterfaceReplica::netTxKbpsChanged,
            this, [this]() { emit netMetricsUpdated(); });
}

void IpcClient::disconnectFromService() {
    if (m_replica) {
        delete m_replica;
        m_replica = nullptr;
    }
    if (m_node) {
        delete m_node;
        m_node = nullptr;
    }
}

bool IpcClient::isConnected() const {
    return m_replica && m_replica->isInitialized();
}

void IpcClient::ping() {
    if (isConnected()) {
        m_replica->ping();
    } else {
        qWarning() << "IpcClient: not connected, cannot ping";
    }
}

QJsonObject IpcClient::getSystemMetrics() {
    QJsonObject metrics;
    if (isConnected()) {
        metrics["cpu_percent"] = m_replica->cpuUsage();
        metrics["memory_percent"] = m_replica->memoryUsage();
        metrics["memory_total_mb"] = m_replica->memoryTotal();
        metrics["memory_available_mb"] = m_replica->memoryAvailable();
        metrics["uptime"] = m_replica->uptime();
        metrics["net_rx_kbps"] = m_replica->netRxKbps();
        metrics["net_tx_kbps"] = m_replica->netTxKbps();
        metrics["status"] = m_replica->serviceStatus();
        metrics["timestamp"] = QDateTime::currentSecsSinceEpoch();
    } else {
        metrics["error"] = "Not connected to service";
    }
    return metrics;
}

QString IpcClient::serviceStatus() const {
    return isConnected() ? m_replica->serviceStatus() : "Disconnected";
}

double IpcClient::cpuUsage() const {
    return isConnected() ? m_replica->cpuUsage() : 0.0;
}

double IpcClient::memoryUsage() const {
    return isConnected() ? m_replica->memoryUsage() : 0.0;
}

double IpcClient::memoryTotal() const {
    return isConnected() ? m_replica->memoryTotal() : 0.0;
}

double IpcClient::memoryAvailable() const {
    return isConnected() ? m_replica->memoryAvailable() : 0.0;
}

double IpcClient::netRxKbps() const {
    return isConnected() ? m_replica->netRxKbps() : 0.0;
}

double IpcClient::netTxKbps() const {
    return isConnected() ? m_replica->netTxKbps() : 0.0;
}

int IpcClient::uptime() const {
    return isConnected() ? m_replica->uptime() : 0;
}

void IpcClient::onReplicaInitialized() {
    qDebug() << "IpcClient: connected to service via QtRO";
    emit connected();
}

void IpcClient::onReplicaPongReceived(const QString& message) {
    emit pongReceived(message);
}

void IpcClient::onReplicaLogMessage(const QString& level, const QString& message) {
    emit logMessage(level, message);
}
