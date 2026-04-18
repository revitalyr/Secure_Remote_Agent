/**
 * @file proxy_plugin.h
 * @brief Proxy plugin for HTTP/HTTPS traffic interception
 */

#pragma once
#include "../loader/plugin_interface.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QMutex>

/**
 * @brief Proxy plugin for HTTP/HTTPS traffic interception
 *
 * Implements a proxy server that intercepts and forwards HTTP/HTTPS
 * traffic. Provides metrics on requests handled and bytes transferred.
 */
class ProxyPlugin : public QObject, public IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.secureagent.IPlugin" FILE "proxy_plugin.json")
    Q_INTERFACES(IPlugin)

public:
    /**
     * @brief Constructs a proxy plugin
     */
    ProxyPlugin();

    /**
     * @brief Destroys the proxy plugin
     */
    ~ProxyPlugin() override;

    // IPlugin interface
    /**
     * @brief Gets the plugin name
     * @return Plugin name
     */
    QString name() const override;

    /**
     * @brief Gets the plugin version
     * @return Plugin version string
     */
    QString version() const override;

    /**
     * @brief Gets the plugin description
     * @return Plugin description
     */
    QString description() const override;

    /**
     * @brief Initializes the plugin with configuration
     * @param config Configuration map (default: empty)
     * @return true if initialization succeeded, false otherwise
     */
    bool initialize(const QVariantMap& config = QVariantMap()) override;

    /**
     * @brief Starts the proxy server
     */
    void start() override;

    /**
     * @brief Stops the proxy server
     */
    void stop() override;

    /**
     * @brief Cleans up plugin resources
     */
    void cleanup() override;

    /**
     * @brief Executes the plugin with default parameters
     */
    void execute() override;

    /**
     * @brief Executes the plugin with custom parameters
     * @param params Execution parameters
     * @return Result map
     */
    QVariantMap executeWithParams(const QVariantMap& params) override;

    /**
     * @brief Gets the current plugin status
     * @return Plugin status
     */
    PluginStatus status() const override;

    /**
     * @brief Gets plugin metrics
     * @return Metrics map (requests handled, bytes transferred)
     */
    QVariantMap getMetrics() const override;

    /**
     * @brief Gets the last error message
     * @return Error message string
     */
    QString getLastError() const override;

    /**
     * @brief Sets plugin configuration
     * @param config Configuration map
     * @return true if configuration was set successfully, false otherwise
     */
    bool setConfig(const QVariantMap& config) override;

    /**
     * @brief Gets plugin configuration
     * @return Configuration map
     */
    QVariantMap getConfig() const override;

    /**
     * @brief Sets a callback for async operations
     * @param callback Callback function
     */
    void setCallback(std::function<void(const QString&, const QVariantMap&)> callback) override;

private slots:
    /**
     * @brief Handles request completion
     */
    void onRequestFinished();

    /**
     * @brief Handles metrics timer timeout
     */
    void onMetricsTimer();

private:
    /**
     * @brief Sets up the proxy server
     */
    void setupProxyServer();

    /**
     * @brief Handles a proxy request
     * @param url Target URL
     * @param data Request data
     */
    void handleProxyRequest(const QString& url, const QByteArray& data);

    /**
     * @brief Updates plugin metrics
     */
    void updateMetrics();

    PluginStatus m_status;
    QVariantMap m_config;
    QString m_lastError;
    std::function<void(const QString&, const QVariantMap&)> m_callback;

    QNetworkAccessManager* m_network;
    QTimer* m_metricsTimer;

    // Metrics
    qint64 m_requestsHandled;
    qint64 m_bytesTransferred;
    qint64 m_startTime;

    mutable QMutex m_mutex;
};
