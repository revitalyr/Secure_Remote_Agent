// service/service_controller.cpp
#include "service_controller.h"
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QDir>
#include <QPluginLoader>
#include "../../core/ipc_server.h"
#include "../loader/plugin_interface.h"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

ServiceController::ServiceController(QObject* parent)
    : QObject(parent)
    , m_networkManager(nullptr)
    , m_running(false)
    , m_paused(false)
    , m_startTime(0)
    , m_lastHeartbeat(0)
    , m_lastPluginStatus(0)
    , m_backendUrl("http://localhost:5000")
    , m_ipcServer(nullptr)
{
}

ServiceController::~ServiceController() {
    stop();
}

void ServiceController::initialize() {
    // Initialize network manager
    m_networkManager = new QNetworkAccessManager(this);
    
    // Start IPC server
    startIpcServer();
    
    // Load plugins
    loadPlugins();
    
    // Set start time
    m_startTime = QDateTime::currentSecsSinceEpoch();
    
    // Send initial heartbeat
    sendHeartbeat();
    
    // Send initial plugin status
    sendPluginStatus();
}

void ServiceController::start() {
    if (m_running) return;
    
    initialize();
    m_running = true;
    m_paused = false;
    
    emit started();
    
    // Use QTimer for periodic tasks instead of separate thread
    QTimer* heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &ServiceController::sendHeartbeat);
    heartbeatTimer->start(30000); // 30 seconds
    
    QTimer* pluginStatusTimer = new QTimer(this);
    connect(pluginStatusTimer, &QTimer::timeout, this, &ServiceController::sendPluginStatus);
    pluginStatusTimer->start(10000); // 10 seconds
    
    QTimer* metricsTimer = new QTimer(this);
    connect(metricsTimer, &QTimer::timeout, this, &ServiceController::sendMetrics);
    metricsTimer->start(1000); // 1 second
}

void ServiceController::stop() {
    if (!m_running) return;
    
    m_running = false;
    
    // Cleanup
    unloadPlugins();
    stopIpcServer();
    
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

void ServiceController::sendHeartbeat() {
    if (!m_networkManager) return;
    
    QJsonObject json;
    json["service_id"] = "SecureRemoteAgent";
    json["timestamp"] = QDateTime::currentSecsSinceEpoch();
    
    QNetworkRequest request(QUrl(m_backendUrl + "/api/heartbeat"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(json).toJson());
    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        reply->deleteLater();
    });
}

void ServiceController::sendMetrics() {
    if (!m_networkManager) return;
    
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    qint64 uptime = currentTime - m_startTime;
    
    // Get real system metrics
    QJsonObject systemMetrics = measureSystemMetrics();
    
    QJsonObject json;
    json["service_id"] = "SecureRemoteAgent";
    json["uptime"] = uptime;
    json["requests_handled"] = 0;
    json["timestamp"] = currentTime;
    json["cpu_usage"] = systemMetrics["cpu_percent"].toDouble();
    json["memory_usage"] = systemMetrics["memory_percent"].toDouble();
    
    QNetworkRequest request(QUrl(m_backendUrl + "/api/metrics"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(json).toJson());
    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        reply->deleteLater();
    });
}

void ServiceController::sendPluginStatus() {
    if (!m_networkManager) return;
    
    QJsonObject pluginsJson;
    
    // ProxyPlugin status
    QJsonObject proxyPlugin;
    proxyPlugin["status"] = "loaded";
    proxyPlugin["version"] = "1.0.0";
    proxyPlugin["requests_handled"] = 0;
    pluginsJson["ProxyPlugin"] = proxyPlugin;
    
    // ScraperPlugin status
    QJsonObject scraperPlugin;
    scraperPlugin["status"] = "loaded";
    scraperPlugin["version"] = "1.0.0";
    scraperPlugin["pages_scraped"] = 0;
    pluginsJson["ScraperPlugin"] = scraperPlugin;
    
    QNetworkRequest request(QUrl(m_backendUrl + "/api/plugins"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(pluginsJson).toJson());
    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        reply->deleteLater();
    });
}

void ServiceController::startIpcServer() {
    if (m_ipcServer) return;
    
    m_ipcServer = new QLocalServer(this);
    
    // Remove any existing socket file
    QLocalServer::removeServer("service_ipc");
    
    if (!m_ipcServer->listen("service_ipc")) {
        qWarning() << "Failed to start IPC server:" << m_ipcServer->errorString();
        delete m_ipcServer;
        m_ipcServer = nullptr;
        return;
    }
    
    qDebug() << "IPC server started on 'service_ipc'";
    
    connect(m_ipcServer, &QLocalServer::newConnection, this, [this]() {
        QLocalSocket* client = m_ipcServer->nextPendingConnection();
        if (client) {
            handleIpcRequest(client);
        }
    });
}

void ServiceController::stopIpcServer() {
    if (m_ipcServer) {
        m_ipcServer->close();
        delete m_ipcServer;
        m_ipcServer = nullptr;
        qDebug() << "IPC server stopped";
    }
}

void ServiceController::handleIpcRequest(QLocalSocket* client) {
    connect(client, &QLocalSocket::readyRead, [this, client]() {
        QByteArray data = client->readAll();
        QString request = QString::fromUtf8(data);
        
        qDebug() << "Received IPC request:" << request;
        
        // Handle different requests
        if (request.trimmed() == "ping") {
            client->write("pong");
            client->flush();
        } else if (request.trimmed() == "GET_METRICS") {
            QJsonObject metrics = measureSystemMetrics();
            QJsonDocument doc(metrics);
            client->write(doc.toJson());
            client->flush();
        }
    });
    
    connect(client, &QLocalSocket::disconnected, [client]() {
        client->deleteLater();
    });
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
                // Clamp to valid range [0, 100]
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
    // Fallback for non-Windows
    metrics["cpu_percent"] = 0.0;
    metrics["memory_percent"] = 0.0;
#endif
    
    metrics["timestamp"] = QDateTime::currentSecsSinceEpoch();
    qint64 uptime = QDateTime::currentSecsSinceEpoch() - m_startTime;
    metrics["uptime"] = uptime;
    
    return metrics;
}
