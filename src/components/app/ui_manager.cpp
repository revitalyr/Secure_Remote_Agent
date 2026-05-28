#include "components/app/ui_manager.h"
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>

UiManager::UiManager(QObject *parent)
    : QObject(parent)
    , m_ipcClient(nullptr)
    , m_serviceConnected(false)
    , m_serviceStatus("Disconnected")
    , m_cpuUsage(0.0)
    , m_memoryUsage(0.0)
    , m_memoryTotal(0.0)
    , m_memoryAvailable(0.0)
    , m_uptime(0)
    , m_netRxKbps(0.0)
    , m_netTxKbps(0.0)
    , m_metricsTimer(nullptr)
    , m_logTimer(nullptr)
    , m_statusTimer(nullptr)
{
    initializeMetrics();

    m_metricsTimer = new QTimer(this);
    connect(m_metricsTimer, &QTimer::timeout, this, &UiManager::updateMetrics);
    m_metricsTimer->start(METRICS_UPDATE_INTERVAL);

    m_logTimer = new QTimer(this);
    connect(m_logTimer, &QTimer::timeout, this, &UiManager::updateLogStream);
    m_logTimer->start(LOG_UPDATE_INTERVAL);

    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &UiManager::checkServiceStatus);
    m_statusTimer->start(STATUS_CHECK_INTERVAL);

    m_startTime = QDateTime::currentDateTime();

    addLogEntry("INFO", "UI Manager initialized");
    connectToService();
}

UiManager::~UiManager() {
    disconnectFromService();
}

void UiManager::startService() {
    addLogEntry("INFO", "Starting service...");
    m_serviceStatus = "Starting";
    emit serviceStatusChanged();

    if (m_ipcClient && m_ipcClient->isConnected()) {
        addLogEntry("INFO", "Service is already running via IPC");
        m_serviceStatus = "Running";
        m_serviceConnected = true;
        emit serviceStatusChanged();
        emit serviceConnected();
    } else {
        connectToService();
    }
}

void UiManager::stopService() {
    addLogEntry("INFO", "Stopping service...");
    disconnectFromService();
}

void UiManager::restartService() {
    addLogEntry("INFO", "Restarting service...");
    disconnectFromService();
    QTimer::singleShot(1000, this, &UiManager::connectToService);
}

void UiManager::pingService() {
    addLogEntry("INFO", "Pinging service...");

    if (m_ipcClient && m_ipcClient->isConnected()) {
        m_ipcClient->ping();
    } else {
        addLogEntry("ERROR", "Service not connected");
        emit errorOccurred("Service not connected");
    }
}

void UiManager::loadPlugin(const QString& pluginPath) {
    addLogEntry("INFO", "Loading plugin: " + pluginPath);

    QTimer::singleShot(1000, [this, pluginPath]() {
        QString pluginName = QFileInfo(pluginPath).baseName();
        m_pluginStatuses[pluginName] = "Loaded";
        addLogEntry("INFO", "Plugin loaded: " + pluginName);
    });
}

void UiManager::unloadPlugin(const QString& pluginName) {
    addLogEntry("INFO", "Unloading plugin: " + pluginName);
    m_pluginStatuses.remove(pluginName);
    addLogEntry("INFO", "Plugin unloaded: " + pluginName);
}

QVariantMap UiManager::getPluginStatus(const QString& pluginName) {
    QVariantMap status;
    if (m_pluginStatuses.contains(pluginName)) {
        status["name"] = pluginName;
        status["status"] = m_pluginStatuses[pluginName];
        status["timestamp"] = QDateTime::currentSecsSinceEpoch();
    } else {
        status["name"] = pluginName;
        status["status"] = "Not Loaded";
        status["timestamp"] = QDateTime::currentSecsSinceEpoch();
    }
    return status;
}

void UiManager::startAgent(const QString& agentId) {
    addLogEntry("INFO", "Starting agent: " + agentId);
    if (!m_connectedAgents.contains(agentId)) {
        m_connectedAgents.append(agentId);
        emit connectedAgentsChanged();
        addLogEntry("INFO", "Agent started: " + agentId);
    }
}

void UiManager::stopAgent(const QString& agentId) {
    addLogEntry("INFO", "Stopping agent: " + agentId);
    if (m_connectedAgents.contains(agentId)) {
        m_connectedAgents.removeAll(agentId);
        emit connectedAgentsChanged();
        addLogEntry("INFO", "Agent stopped: " + agentId);
    }
}

void UiManager::restartAgent(const QString& agentId) {
    addLogEntry("INFO", "Restarting agent: " + agentId);
    stopAgent(agentId);
    QTimer::singleShot(1000, [this, agentId]() {
        startAgent(agentId);
    });
}

QString UiManager::serviceStatus() const { return m_serviceStatus; }
double UiManager::cpuUsage() const { return m_cpuUsage; }
double UiManager::memoryUsage() const { return m_memoryUsage; }
double UiManager::memoryTotal() const { return m_memoryTotal; }
double UiManager::memoryAvailable() const { return m_memoryAvailable; }
QStringList UiManager::logEntries() const { return m_logEntries; }
QStringList UiManager::connectedAgents() const { return m_connectedAgents; }
int UiManager::uptime() const { return m_uptime; }

double UiManager::netRxKbps() const { return m_netRxKbps; }
double UiManager::netTxKbps() const { return m_netTxKbps; }

void UiManager::updateMetrics() {
    if (m_ipcClient && m_ipcClient->isConnected()) {
        m_cpuUsage = m_ipcClient->cpuUsage();
        m_memoryUsage = m_ipcClient->memoryUsage();
        m_memoryTotal = m_ipcClient->memoryTotal();
        m_memoryAvailable = m_ipcClient->memoryAvailable();
        m_netRxKbps = m_ipcClient->netRxKbps();
        m_netTxKbps = m_ipcClient->netTxKbps();
        m_serviceStatus = m_ipcClient->serviceStatus();
        m_uptime = m_ipcClient->uptime();
    }

    if (m_serviceConnected && m_startTime.isValid() && m_uptime == 0) {
        m_uptime = static_cast<int>(m_startTime.secsTo(QDateTime::currentDateTime()));
    }

    emit cpuUsageChanged();
    emit memoryUsageChanged();
    emit memoryTotalChanged();
    emit memoryAvailableChanged();
    emit uptimeChanged();
    emit serviceStatusChanged();
    emit netMetricsChanged();
}

void UiManager::updateLogStream() {
    static int counter = 0;
    counter++;

    if (counter % 30 == 0) {
        addLogEntry("INFO", QString("System healthy - CPU: %1%, Memory: %2%")
                   .arg(m_cpuUsage, 0, 'f', 1)
                   .arg(m_memoryUsage, 0, 'f', 1));
    }
}

void UiManager::checkServiceStatus() {
    if (!m_ipcClient) return;

    bool wasConnected = m_serviceConnected;
    bool nowConnected = m_ipcClient->isConnected();

    if (nowConnected && !wasConnected) {
        onIpcConnected();
    } else if (!nowConnected && wasConnected) {
        onIpcDisconnected();
    }
}

void UiManager::onIpcConnected() {
    m_serviceConnected = true;
    m_serviceStatus = "Connected";
    emit serviceStatusChanged();
    emit serviceConnected();
    addLogEntry("INFO", "Connected to service via IPC");
}

void UiManager::onIpcDisconnected() {
    m_serviceConnected = false;
    m_serviceStatus = "Disconnected";
    emit serviceStatusChanged();
    emit serviceDisconnected();
    addLogEntry("WARNING", "Disconnected from service");
}

void UiManager::onIpcPong(const QString& message) {
    addLogEntry("INFO", "Pong received from service: " + message);
}

void UiManager::onIpcLog(const QString& level, const QString& message) {
    addLogEntry(level, "Service: " + message);
}

void UiManager::initializeMetrics() {
    m_connectedAgents << "Agent-001" << "Agent-002";
    emit connectedAgentsChanged();

    m_pluginStatuses["ProxyPlugin"] = "Loaded";
    m_pluginStatuses["ScraperPlugin"] = "Loaded";
}

void UiManager::addLogEntry(const QString& level, const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString entry = QString("[%1] %2: %3").arg(timestamp, level, message);

    m_logEntries.append(entry);

    while (m_logEntries.size() > MAX_LOG_ENTRIES) {
        m_logEntries.removeFirst();
    }

    emit logEntriesChanged();
    qDebug() << entry;
}

void UiManager::connectToService() {
    addLogEntry("INFO", "Connecting to service...");

    m_ipcClient = new IpcClient(this);

    connect(m_ipcClient, &IpcClient::connected, this, &UiManager::onIpcConnected);
    connect(m_ipcClient, &IpcClient::disconnected, this, &UiManager::onIpcDisconnected);
    connect(m_ipcClient, &IpcClient::pongReceived, this, &UiManager::onIpcPong);
    connect(m_ipcClient, &IpcClient::logMessage, this, &UiManager::onIpcLog);
    connect(m_ipcClient, &IpcClient::metricsUpdated, this, &UiManager::updateMetrics);
    connect(m_ipcClient, &IpcClient::netMetricsUpdated, this, &UiManager::updateMetrics);

    m_ipcClient->connectToService("local:service_ipc");
}

void UiManager::disconnectFromService() {
    if (m_ipcClient) {
        m_ipcClient->disconnectFromService();
        m_ipcClient->deleteLater();
        m_ipcClient = nullptr;
    }
    m_serviceConnected = false;
    m_serviceStatus = "Disconnected";
    emit serviceStatusChanged();
    emit serviceDisconnected();
    addLogEntry("INFO", "Disconnected from service");
}


