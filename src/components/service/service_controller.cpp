#include "components/service/service_controller.h"
#include "components/service/service_source_impl.h"
#include "rep_ServiceInterface_merged.h"
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QDir>
#include <QPluginLoader>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi")
#endif


ServiceController::ServiceController(QObject* parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_running(false)
    , m_paused(false)
    , m_startTime(0)
    , m_lastHeartbeat(0)
    , m_lastPluginStatus(0)
    , backendServiceUrl("http://localhost:5000")
    , m_qtroHost(nullptr)
    , m_serviceSource(nullptr)
    , m_watchdogTimer(nullptr)
    , m_watchdogMissed(0)
{
}

int ServiceController::uptime() const { return m_uptime; }
void ServiceController::setUptime(int v) { m_uptime = v; emit uptimeChanged(v); }

void ServiceController::ping() {
    emit pongReceived("pong");
}

void ServiceController::requestShutdown() {
    emit logMessage("INFO", "Shutdown requested via IPC");
}

void ServiceController::setServiceStatus(const QString& status) { m_serviceStatus = status; }
void ServiceController::setCpuUsage(double usage) { m_cpuUsage = usage; }
void ServiceController::setMemoryUsage(double usage) { m_memoryUsage = usage; }
void ServiceController::setMemoryTotal(double total) { m_memoryTotal = total; }
void ServiceController::setMemoryAvailable(double available) { m_memoryAvailable = available; }
void ServiceController::setNetRxKbps(double rx) { m_netRxKbps = rx; }
void ServiceController::setNetTxKbps(double tx) { m_netTxKbps = tx; }

void ServiceController::setAll(const QString& status, double cpu, double mem,
                double memTotal, double memAvail,
                double netRx, double netTx, int uptime)
{
    setServiceStatus(status);
    setCpuUsage(cpu);
    setMemoryUsage(mem);
    setMemoryTotal(memTotal);
    setMemoryAvailable(memAvail);
    setNetRxKbps(netRx);
    setNetTxKbps(netTx);
    setUptime(uptime);
}

ServiceController::~ServiceController() {
    stop();
}

QString ServiceController::statusString() const {
    if (m_paused) return "Paused";
    if (m_running) return "Running";
    return "Stopped";
}

void ServiceController::initialize() {
    m_networkManager = new QNetworkAccessManager(this);

    m_serviceSource = new ServiceSourceImpl(this);
    m_qtroHost = new QRemoteObjectHost(QUrl("local:service_ipc"), this);
    m_qtroHost->enableRemoting(m_serviceSource);

    loadPlugins();

    m_startTime = QDateTime::currentSecsSinceEpoch();

    sendRegistration();
    sendHeartbeat();
    sendPluginStatus();

    qDebug() << "QtRO IPC server started on 'local:service_ipc'";
}

void ServiceController::start() {
    if (m_running) return;

    initialize();
    m_running = true;
    m_paused = false;

    emit started();

    QTimer* heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &ServiceController::sendHeartbeat);
    heartbeatTimer->start(30000);

    QTimer* pluginStatusTimer = new QTimer(this);
    connect(pluginStatusTimer, &QTimer::timeout, this, &ServiceController::sendPluginStatus);
    pluginStatusTimer->start(10000);

    QTimer* metricsTimer = new QTimer(this);
    connect(metricsTimer, &QTimer::timeout, this, &ServiceController::pushMetricsToSource);
    metricsTimer->start(1000);

    m_watchdogTimer = new QTimer(this);
    connect(m_watchdogTimer, &QTimer::timeout, this, &ServiceController::watchdogTick);
    m_watchdogTimer->start(15000);
    m_watchdogMissed = 0;
}

void ServiceController::stop() {
    if (!m_running) return;

    m_running = false;

    unloadPlugins();

    if (m_watchdogTimer) {
        m_watchdogTimer->stop();
        m_watchdogTimer = nullptr;
    }

    if (m_qtroHost) {
        delete m_qtroHost;
        m_qtroHost = nullptr;
    }
    m_serviceSource = nullptr;

    if (m_networkManager) {
        delete m_networkManager;
        m_networkManager = nullptr;
    }

    emit stopped();
}

void ServiceController::pause() {
    if (!m_running || m_paused) return;
    m_paused = true;
    emit paused();
}

void ServiceController::resume() {
    if (!m_running || !m_paused) return;
    m_paused = false;
    emit resumed();
}

void ServiceController::loadPlugins() {
    QString pluginsDir = QCoreApplication::applicationDirPath() + "/plugins";
    QDir dir(pluginsDir);

    if (!dir.exists()) {
        qDebug() << "Plugins directory not found:" << pluginsDir;
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
                qDebug() << "Loaded plugin:" << pluginFile;
            } else {
                loader->unload();
                delete loader;
                qDebug() << "Failed to cast plugin:" << pluginFile;
            }
        } else {
            qDebug() << "Failed to load plugin:" << pluginFile << "Error:" << loader->errorString();
            delete loader;
        }
    }
}

void ServiceController::unloadPlugins() {
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

void ServiceController::addAuthHeader(QNetworkRequest& request) {
    if (!apiAuthenticationKey.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + apiAuthenticationKey).toUtf8());
        request.setRawHeader("X-API-Key", apiAuthenticationKey.toUtf8());
    }
}

void ServiceController::sendRegistration() {
    if (!m_networkManager) return;

    QJsonObject json;
    json["service_id"] = "SecureRemoteAgent";
    json["version"] = "1.0.0";
    json["hostname"] = qEnvironmentVariable("COMPUTERNAME", "unknown");
    json["timestamp"] = QDateTime::currentSecsSinceEpoch();
    json["capabilities"] = QJsonArray{"monitoring", "plugins", "remote_exec"};

    QNetworkRequest request(QUrl(backendServiceUrl + "/api/register"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    addAuthHeader(request);

    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(json).toJson());
    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Service registered with backend";
        } else {
            qWarning() << "Registration failed:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void ServiceController::sendHeartbeat() {
    if (!m_networkManager) return;

    QJsonObject json;
    json["service_id"] = "SecureRemoteAgent";
    json["timestamp"] = QDateTime::currentSecsSinceEpoch();

    QNetworkRequest request(QUrl(backendServiceUrl + "/api/heartbeat"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    addAuthHeader(request);

    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(json).toJson());
    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        reply->deleteLater();
    });
}

void ServiceController::sendMetrics() {
    if (!m_networkManager) return;

    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    qint64 uptime = currentTime - m_startTime;

    QJsonObject systemMetrics = measureSystemMetrics();

    QJsonObject json;
    json["service_id"] = "SecureRemoteAgent";
    json["uptime"] = uptime;
    json["requests_handled"] = 0;
    json["timestamp"] = currentTime;
    json["cpu_usage"] = systemMetrics["cpu_percent"].toDouble();
    json["memory_usage"] = systemMetrics["memory_percent"].toDouble();

    QNetworkRequest request(QUrl(backendServiceUrl + "/api/metrics"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    addAuthHeader(request);

    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(json).toJson());
    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        reply->deleteLater();
    });
}

void ServiceController::sendPluginStatus() {
    if (!m_networkManager) return;

    QJsonObject pluginsJson;

    QJsonObject proxyPlugin;
    proxyPlugin["status"] = "loaded";
    proxyPlugin["version"] = "1.0.0";
    proxyPlugin["requests_handled"] = 0;
    pluginsJson["ProxyPlugin"] = proxyPlugin;

    QJsonObject scraperPlugin;
    scraperPlugin["status"] = "loaded";
    scraperPlugin["version"] = "1.0.0";
    scraperPlugin["pages_scraped"] = 0;
    pluginsJson["ScraperPlugin"] = scraperPlugin;

    QNetworkRequest request(QUrl(backendServiceUrl + "/api/plugins"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    addAuthHeader(request);

    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(pluginsJson).toJson());
    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        reply->deleteLater();
    });
}

void ServiceController::pushMetricsToSource() {
    sendMetrics();
    m_watchdogMissed = 0;

    if (!m_serviceSource) return;

    QJsonObject metrics = measureSystemMetrics();
    qint64 uptime = QDateTime::currentSecsSinceEpoch() - m_startTime;

    m_serviceSource->setServiceStatus(statusString());
    m_serviceSource->setCpuUsage(metrics["cpu_percent"].toDouble());
    m_serviceSource->setMemoryUsage(metrics["memory_percent"].toDouble());
    m_serviceSource->setMemoryTotal(metrics["memory_total_mb"].toDouble());
    m_serviceSource->setMemoryAvailable(metrics["memory_available_mb"].toDouble());
    m_serviceSource->setNetRxKbps(metrics["net_rx_kbps"].toDouble());
    m_serviceSource->setNetTxKbps(metrics["net_tx_kbps"].toDouble());
    m_serviceSource->setUptime(static_cast<int>(uptime));
}

void ServiceController::watchdogTick() {
    m_watchdogMissed++;

    if (m_watchdogMissed >= 3) {
        qWarning() << "Watchdog: service appears unresponsive (" << m_watchdogMissed << "missed ticks)";

        if (m_networkManager) {
            sendHeartbeat();
        }

        m_watchdogMissed = 0;
    }
}

QJsonObject ServiceController::measureSystemMetrics() {
    QJsonObject metrics;

#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);

    metrics["memory_percent"] = (qint64)memoryStatus.dwMemoryLoad;
    metrics["memory_total_mb"] = (qint64)(memoryStatus.ullTotalPhys / (1024 * 1024));
    metrics["memory_available_mb"] = (qint64)(memoryStatus.ullAvailPhys / (1024 * 1024));

    static ULONGLONG prevRxBytes = 0, prevTxBytes = 0;
    static ULONGLONG prevNetTick = 0;
    ULONGLONG rxBytes = 0, txBytes = 0;

    MIB_IFTABLE* ifTable = (MIB_IFTABLE*)malloc(sizeof(MIB_IFTABLE));
    ULONG bufSize = sizeof(MIB_IFTABLE);
    if (GetIfTable(ifTable, &bufSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
        free(ifTable);
        ifTable = (MIB_IFTABLE*)malloc(bufSize);
    }
    if (GetIfTable(ifTable, &bufSize, FALSE) == NO_ERROR) {
        for (DWORD i = 0; i < ifTable->dwNumEntries; ++i) {
            MIB_IFROW& row = ifTable->table[i];
            if (row.dwOperStatus == IF_OPER_STATUS_OPERATIONAL &&
                row.dwType != MIB_IF_TYPE_LOOPBACK) {
                rxBytes += row.dwInOctets;
                txBytes += row.dwOutOctets;
            }
        }
    }
    free(ifTable);

    ULONGLONG now = GetTickCount64();
    if (prevRxBytes != 0 && prevTxBytes != 0 && prevNetTick != 0) {
        double dt = (double)(now - prevNetTick) / 1000.0;
        if (dt > 0.0) {
            metrics["net_rx_kbps"] = (double)(rxBytes - prevRxBytes) / dt / 1024.0;
            metrics["net_tx_kbps"] = (double)(txBytes - prevTxBytes) / dt / 1024.0;
        }
    }
    prevRxBytes = rxBytes;
    prevTxBytes = txBytes;
    prevNetTick = now;

    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        static ULONGLONG lastIdle = 0, lastKernel = 0, lastUser = 0;
        ULONGLONG idle = (((ULONGLONG)idleTime.dwHighDateTime) << 32) | idleTime.dwLowDateTime;
        ULONGLONG kernel = (((ULONGLONG)kernelTime.dwHighDateTime) << 32) | kernelTime.dwLowDateTime;
        ULONGLONG user = (((ULONGLONG)userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime;

        if (lastIdle != 0) {
            ULONGLONG idleDiff = idle - lastIdle;
            ULONGLONG kernelDiff = kernel - lastKernel;
            ULONGLONG userDiff = user - lastUser;

            ULONGLONG total = kernelDiff + userDiff;
            if (total > 0) {
                double cpuUsage = 100.0 * (1.0 - (double)idleDiff / (double)total);
                if (cpuUsage < 0) cpuUsage = 0;
                if (cpuUsage > 100) cpuUsage = 100;
                metrics["cpu_percent"] = cpuUsage;
            }
        }

        lastIdle = idle;
        lastKernel = kernel;
        lastUser = user;
    }
#else
    metrics["cpu_percent"] = 0.0;
    metrics["memory_percent"] = 0.0;
    metrics["net_rx_kbps"] = 0.0;
    metrics["net_tx_kbps"] = 0.0;
#endif

    metrics["timestamp"] = QDateTime::currentSecsSinceEpoch();
    qint64 uptime = QDateTime::currentSecsSinceEpoch() - m_startTime;
    metrics["uptime"] = uptime;

    return metrics;
}


