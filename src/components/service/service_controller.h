/**
 * @file service_controller.h
 * @brief Service controller for Windows service functionality
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QVector>
#include <QAtomicInt>
#include <QPluginLoader>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include "plugin_interface.h"

/**
 * @brief Service controller for Windows service functionality
 *
 * Manages the lifecycle of the Windows service, including:
 * - Plugin loading and management
 * - Network communication with backend server
 * - IPC server for inter-process communication
 * - System metrics measurement and reporting
 * - Heartbeat and status reporting
 */
class ServiceController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructs a service controller
     * @param parent Parent QObject (default: nullptr)
     */
    ServiceController(QObject* parent = nullptr);

    /**
     * @brief Destroys the service controller and stops all services
     */
    ~ServiceController();

    /**
     * @brief Starts the service controller
     *
     * Initializes the service, loads plugins, starts the IPC server,
     * and begins periodic heartbeat/metrics reporting.
     */
    void start();

    /**
     * @brief Stops the service controller
     *
     * Gracefully shuts down the service, unloads plugins, and stops
     * the IPC server.
     */
    void stop();

    /**
     * @brief Pauses the service controller
     *
     * Temporarily pauses all service operations while keeping
     * the service running.
     */
    void pause();

    /**
     * @brief Resumes the service controller
     *
     * Resumes service operations after a pause.
     */
    void resume();

    /**
     * @brief Checks if the service is running
     * @return true if running, false otherwise
     */
    bool isRunning() const { return m_running; }

    /**
     * @brief Checks if the service is paused
     * @return true if paused, false otherwise
     */
    bool isPaused() const { return m_paused; }

signals:
    /**
     * @brief Emitted when the service is started
     */
    void started();

    /**
     * @brief Emitted when the service is stopped
     */
    void stopped();

    /**
     * @brief Emitted when the service is paused
     */
    void paused();

    /**
     * @brief Emitted when the service is resumed
     */
    void resumed();

private:
    /**
     * @brief Initializes the service controller
     */
    void initialize();

    /**
     * @brief Loads plugins from the plugins directory
     */
    void loadPlugins();

    /**
     * @brief Unloads all loaded plugins
     */
    void unloadPlugins();

    /**
     * @brief Sends a heartbeat to the backend server
     */
    void sendHeartbeat();

    /**
     * @brief Sends system metrics to the backend server
     */
    void sendMetrics();

    /**
     * @brief Sends plugin status to the backend server
     */
    void sendPluginStatus();

    /**
     * @brief Runs the main service loop
     */
    void runMainLoop();

    /**
     * @brief Starts the IPC server
     */
    void startIpcServer();

    /**
     * @brief Stops the IPC server
     */
    void stopIpcServer();

    /**
     * @brief Handles an IPC request from a client
     * @param client The client socket
     */
    void handleIpcRequest(QLocalSocket* client);

    /**
     * @brief Measures system metrics (CPU, memory, uptime)
     * @return JSON object containing system metrics
     */
    QJsonObject measureSystemMetrics();

    QNetworkAccessManager* m_networkManager;
    QVector<QPluginLoader*> m_loadedPlugins;
    QAtomicInt m_running;
    QAtomicInt m_paused;
    qint64 m_startTime;
    qint64 m_lastHeartbeat;
    qint64 m_lastPluginStatus;
    QString m_backendUrl;
    QLocalServer* m_ipcServer;
};
