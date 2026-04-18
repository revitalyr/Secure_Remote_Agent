// app/ui_manager.cpp
#include "ui_manager.h"
#include <QDebug>
#include <QDateTime>
#include <QProcess>
#include <QCoreApplication>
#include <windows.h>
#include <psapi.h>

UiManager::UiManager(QObject *parent)
    : QObject(parent)
    , m_serviceConnected(false)
    , m_serviceStatus("Disconnected")
    , m_cpuUsage(0.0)
    , m_memoryUsage(0.0)
    , m_memoryTotal(0.0)
    , m_memoryAvailable(0.0)
    , m_uptime(0)
    , m_metricsTimer(nullptr)
    , m_logTimer(nullptr)
    , m_statusTimer(nullptr)
{
    initializeMetrics();
    
    // Setup timers
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
    
    // In a real implementation, this would start the Windows Service
    // For demo, we simulate service start
    m_serviceStatus = "Starting";
    emit serviceStatusChanged();
    
    // Simulate service startup delay
    QTimer::singleShot(2000, [this]() {
        m_serviceStatus = "Running";
        m_serviceConnected = true;
        emit serviceStatusChanged();
        emit serviceConnected();
        addLogEntry("INFO", "Service started successfully");
    });
}

void UiManager::stopService() {
    addLogEntry("INFO", "Stopping service...");
    
    m_serviceStatus = "Stopping";
    emit serviceStatusChanged();
    
    // Simulate service shutdown delay
    QTimer::singleShot(1500, [this]() {
        m_serviceStatus = "Stopped";
        m_serviceConnected = false;
        emit serviceStatusChanged();
        emit serviceDisconnected();
        addLogEntry("INFO", "Service stopped");
    });
}

void UiManager::restartService() {
    addLogEntry("INFO", "Restarting service...");
    stopService();
    QTimer::singleShot(3000, this, &UiManager::startService);
}

void UiManager::pingService() {
    addLogEntry("INFO", "Pinging service...");
    
    if (m_serviceConnected) {
        // Simulate ping response
        QTimer::singleShot(500, [this]() {
            addLogEntry("INFO", "Pong received from service");
        });
    } else {
        addLogEntry("ERROR", "Service not connected");
        emit errorOccurred("Service not connected");
    }
}

void UiManager::loadPlugin(const QString& pluginPath) {
    addLogEntry("INFO", "Loading plugin: " + pluginPath);
    
    // Simulate plugin loading
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

QString UiManager::serviceStatus() const {
    return m_serviceStatus;
}

double UiManager::cpuUsage() const {
    return m_cpuUsage;
}

double UiManager::memoryUsage() const {
    return m_memoryUsage;
}

double UiManager::memoryTotal() const {
    return m_memoryTotal;
}

double UiManager::memoryAvailable() const {
    return m_memoryAvailable;
}

QStringList UiManager::logEntries() const {
    return m_logEntries;
}

QStringList UiManager::connectedAgents() const {
    return m_connectedAgents;
}

int UiManager::uptime() const {
    return m_uptime;
}

void UiManager::updateMetrics() {
    // Update system metrics
    m_cpuUsage = getCpuUsage();
    m_memoryUsage = getMemoryUsage();
    
    // Update uptime
    if (m_startTime.isValid()) {
        m_uptime = m_startTime.secsTo(QDateTime::currentDateTime());
    }
    
    emit cpuUsageChanged();
    emit memoryUsageChanged();
    emit memoryTotalChanged();
    emit memoryAvailableChanged();
    emit uptimeChanged();
}

void UiManager::updateLogStream() {
    // In a real implementation, this would fetch logs from the service
    // For demo, we add periodic status messages
    static int counter = 0;
    counter++;
    
    if (counter % 30 == 0) { // Every 30 seconds
        addLogEntry("INFO", QString("System healthy - CPU: %1%, Memory: %2%")
                   .arg(m_cpuUsage, 0, 'f', 1)
                   .arg(m_memoryUsage, 0, 'f', 1));
    }
}

void UiManager::checkServiceStatus() {
    // In a real implementation, this would check the actual service status
    // For demo, we simulate status checking
    if (m_serviceConnected && m_serviceStatus == "Running") {
        // Service is healthy
    } else if (!m_serviceConnected && m_serviceStatus == "Running") {
        // Service disconnected unexpectedly
        m_serviceStatus = "Disconnected";
        m_serviceConnected = false;
        emit serviceStatusChanged();
        emit serviceDisconnected();
        addLogEntry("WARNING", "Service disconnected unexpectedly");
    }
}

void UiManager::initializeMetrics() {
    addLogEntry("INFO", "Initializing system metrics...");
    
    // Initialize with demo agents
    m_connectedAgents << "Agent-001" << "Agent-002";
    emit connectedAgentsChanged();
    
    // Initialize with demo plugins
    m_pluginStatuses["ProxyPlugin"] = "Loaded";
    m_pluginStatuses["ScraperPlugin"] = "Loaded";
}

void UiManager::addLogEntry(const QString& level, const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString entry = QString("[%1] %2: %3").arg(timestamp, level, message);
    
    m_logEntries.append(entry);
    
    // Limit log entries to prevent memory issues
    while (m_logEntries.size() > MAX_LOG_ENTRIES) {
        m_logEntries.removeFirst();
    }
    
    emit logEntriesChanged();
    qDebug() << entry;
}

void UiManager::connectToService() {
    addLogEntry("INFO", "Connecting to service...");
    
    // Simulate connection
    QTimer::singleShot(1000, [this]() {
        m_serviceConnected = true;
        m_serviceStatus = "Connected";
        emit serviceStatusChanged();
        emit serviceConnected();
        addLogEntry("INFO", "Connected to service");
    });
}

void UiManager::disconnectFromService() {
    if (m_serviceConnected) {
        m_serviceConnected = false;
        m_serviceStatus = "Disconnected";
        emit serviceStatusChanged();
        emit serviceDisconnected();
        addLogEntry("INFO", "Disconnected from service");
    }
}

double UiManager::getCpuUsage() {
    // Get CPU usage using Windows API
    static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
    static int numProcessors = 0;
    static HANDLE self = GetCurrentProcess();
    
    if (numProcessors == 0) {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        numProcessors = sysInfo.dwNumberOfProcessors;
        
        FILETIME ftime, fsys, fuser;
        GetSystemTimeAsFileTime(&ftime);
        memcpy(&lastCPU, &ftime, sizeof(FILETIME));
        
        GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
        memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
        memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
    }
    
    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now, sys, user;
    
    GetSystemTimeAsFileTime(&ftime);
    memcpy(&now, &ftime, sizeof(FILETIME));
    
    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&sys, &fsys, sizeof(FILETIME));
    memcpy(&user, &fuser, sizeof(FILETIME));
    
    double percent = (sys.QuadPart - lastSysCPU.QuadPart) + (user.QuadPart - lastUserCPU.QuadPart);
    percent /= (now.QuadPart - lastCPU.QuadPart);
    percent /= numProcessors;
    
    lastCPU = now;
    lastUserCPU = user;
    lastSysCPU = sys;
    
    return percent * 100;
}

double UiManager::getMemoryUsage() {
    // Get memory usage using Windows API
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        double totalMem = pmc.WorkingSetSize;
        double totalPhysMem = 0;
        
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        totalPhysMem = memInfo.ullTotalPhys;
        
        // Update total and available memory in MB
        m_memoryTotal = (double)(memInfo.ullTotalPhys / (1024 * 1024));
        m_memoryAvailable = (double)(memInfo.ullAvailPhys / (1024 * 1024));
        
        return (totalMem / totalPhysMem) * 100.0;
    }
    return 0.0;
}

#include "ui_manager.moc"
