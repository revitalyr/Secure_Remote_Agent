// proxy/proxy_plugin.cpp
#include "proxy_plugin.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QDebug>

ProxyPlugin::ProxyPlugin() 
    : m_status(PluginStatus::Unloaded)
    , m_network(nullptr)
    , m_metricsTimer(nullptr)
    , m_requestsHandled(0)
    , m_bytesTransferred(0)
    , m_startTime(0)
{
}

ProxyPlugin::~ProxyPlugin() {
    cleanup();
}

QString ProxyPlugin::name() const {
    return "ProxyPlugin";
}

QString ProxyPlugin::version() const {
    return "1.0.0";
}

QString ProxyPlugin::description() const {
    return "HTTP/HTTPS proxy plugin for traffic interception and forwarding";
}

bool ProxyPlugin::initialize(const QVariantMap& config) {
    QMutexLocker locker(&m_mutex);
    
    if (m_status != PluginStatus::Unloaded) {
        m_lastError = "Plugin already initialized";
        return false;
    }
    
    m_config = config;
    m_config["listen_port"] = m_config.value("listen_port", 8080);
    m_config["target_host"] = m_config.value("target_host", "localhost");
    m_config["target_port"] = m_config.value("target_port", 5000);
    m_config["enable_ssl"] = m_config.value("enable_ssl", false);
    
    m_network = new QNetworkAccessManager(this);
    
    m_status = PluginStatus::Loaded;
    m_lastError.clear();
    
    if (m_callback) {
        m_callback(name(), {{"event", "initialized"}, {"config", m_config}});
    }
    
    return true;
}

void ProxyPlugin::start() {
    QMutexLocker locker(&m_mutex);
    
    if (m_status != PluginStatus::Loaded) {
        m_lastError = "Plugin not initialized";
        return;
    }
    
    setupProxyServer();
    
    m_metricsTimer = new QTimer(this);
    connect(m_metricsTimer, &QTimer::timeout, this, &ProxyPlugin::onMetricsTimer);
    m_metricsTimer->start(5000); // Update metrics every 5 seconds
    
    m_startTime = QDateTime::currentSecsSinceEpoch();
    m_status = PluginStatus::Running;
    m_lastError.clear();
    
    if (m_callback) {
        m_callback(name(), {{"event", "started"}, {"port", m_config["listen_port"]}});
    }
}

void ProxyPlugin::stop() {
    QMutexLocker locker(&m_mutex);
    
    if (m_status != PluginStatus::Running) {
        return;
    }
    
    if (m_metricsTimer) {
        m_metricsTimer->stop();
        delete m_metricsTimer;
        m_metricsTimer = nullptr;
    }
    
    m_status = PluginStatus::Loaded;
    
    if (m_callback) {
        m_callback(name(), {{"event", "stopped"}});
    }
}

void ProxyPlugin::cleanup() {
    QMutexLocker locker(&m_mutex);
    
    if (m_status == PluginStatus::Running) {
        stop();
    }
    
    if (m_network) {
        delete m_network;
        m_network = nullptr;
    }
    
    m_status = PluginStatus::Unloaded;
    m_requestsHandled = 0;
    m_bytesTransferred = 0;
}

void ProxyPlugin::execute() {
    QVariantMap params;
    params["action"] = "status_check";
    executeWithParams(params);
}

QVariantMap ProxyPlugin::executeWithParams(const QVariantMap& params) {
    QMutexLocker locker(&m_mutex);
    
    QVariantMap result;
    result["plugin"] = name();
    result["timestamp"] = QDateTime::currentSecsSinceEpoch();
    
    QString action = params.value("action", "status").toString();
    
    if (action == "forward_request") {
        QString url = params.value("url").toString();
        QByteArray data = params.value("data", "").toByteArray();
        
        if (!url.isEmpty()) {
            handleProxyRequest(url, data);
            result["status"] = "request_forwarded";
            result["url"] = url;
        } else {
            result["error"] = "Missing URL parameter";
        }
    } else if (action == "get_stats") {
        result["requests_handled"] = m_requestsHandled;
        result["bytes_transferred"] = m_bytesTransferred;
        result["uptime_seconds"] = m_startTime > 0 ? QDateTime::currentSecsSinceEpoch() - m_startTime : 0;
    } else {
        result["status"] = "unknown_action";
        result["action"] = action;
    }
    
    return result;
}

PluginStatus ProxyPlugin::status() const {
    QMutexLocker locker(&m_mutex);
    return m_status;
}

QVariantMap ProxyPlugin::getMetrics() const {
    QMutexLocker locker(&m_mutex);
    
    QVariantMap metrics;
    metrics["requests_handled"] = m_requestsHandled;
    metrics["bytes_transferred"] = m_bytesTransferred;
    metrics["uptime_seconds"] = m_startTime > 0 ? QDateTime::currentSecsSinceEpoch() - m_startTime : 0;
    metrics["status"] = static_cast<int>(m_status);
    
    return metrics;
}

QString ProxyPlugin::getLastError() const {
    QMutexLocker locker(&m_mutex);
    return m_lastError;
}

bool ProxyPlugin::setConfig(const QVariantMap& config) {
    QMutexLocker locker(&m_mutex);
    m_config = config;
    return true;
}

QVariantMap ProxyPlugin::getConfig() const {
    QMutexLocker locker(&m_mutex);
    return m_config;
}

void ProxyPlugin::setCallback(std::function<void(const QString&, const QVariantMap&)> callback) {
    m_callback = callback;
}

void ProxyPlugin::setupProxyServer() {
    // In a real implementation, this would set up an HTTP server
    // For now, we simulate proxy functionality
    qDebug() << "Proxy server setup on port:" << m_config["listen_port"].toInt();
}

void ProxyPlugin::handleProxyRequest(const QString& url, const QByteArray& data) {
    if (!m_network) return;
    
    QUrl targetUrl(url);
    QNetworkRequest request(targetUrl);
    request.setRawHeader("User-Agent", "SecureAgent-Proxy/1.0");
    
    QNetworkReply* reply = m_network->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &ProxyPlugin::onRequestFinished);
    
    m_requestsHandled++;
    m_bytesTransferred += data.size();
}

void ProxyPlugin::onRequestFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        m_bytesTransferred += responseData.size();
        
        if (m_callback) {
            m_callback(name(), {
                {"event", "request_completed"},
                {"status_code", reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)},
                {"response_size", responseData.size()}
            });
        }
    } else {
        m_lastError = reply->errorString();
        
        if (m_callback) {
            m_callback(name(), {
                {"event", "request_error"},
                {"error", reply->errorString()}
            });
        }
    }
    
    reply->deleteLater();
}

void ProxyPlugin::onMetricsTimer() {
    updateMetrics();
}

void ProxyPlugin::updateMetrics() {
    if (m_callback) {
        m_callback(name(), {
            {"event", "metrics_update"},
            {"metrics", getMetrics()}
        });
    }
}

#include "proxy_plugin.moc"
