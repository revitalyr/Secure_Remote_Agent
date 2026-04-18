/**
 * @file ui_manager.h
 * @brief UI manager for Qt QML application
 */

#pragma once
#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include <QProcess>
#include <QThread>
#include <QDateTime>
#include <QFileInfo>
#include <QStringList>

/**
 * @brief UI manager for Qt QML application
 *
 * Manages the UI state and provides data to QML views.
 * Handles service control, plugin management, agent management,
 * system metrics, and log streaming.
 */
class UiManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString serviceStatus READ serviceStatus NOTIFY serviceStatusChanged)
    Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY cpuUsageChanged)
    Q_PROPERTY(double memoryUsage READ memoryUsage NOTIFY memoryUsageChanged)
    Q_PROPERTY(double memoryTotal READ memoryTotal NOTIFY memoryTotalChanged)
    Q_PROPERTY(double memoryAvailable READ memoryAvailable NOTIFY memoryAvailableChanged)
    Q_PROPERTY(QStringList logEntries READ logEntries NOTIFY logEntriesChanged)
    Q_PROPERTY(QStringList connectedAgents READ connectedAgents NOTIFY connectedAgentsChanged)
    Q_PROPERTY(int uptime READ uptime NOTIFY uptimeChanged)

public:
    /**
     * @brief Constructs a UI manager
     * @param parent Parent QObject (default: nullptr)
     */
    explicit UiManager(QObject *parent = nullptr);

    /**
     * @brief Destroys the UI manager
     */
    ~UiManager();

    // Service control
    /**
     * @brief Starts the Windows service
     */
    Q_INVOKABLE void startService();

    /**
     * @brief Stops the Windows service
     */
    Q_INVOKABLE void stopService();

    /**
     * @brief Restarts the Windows service
     */
    Q_INVOKABLE void restartService();

    /**
     * @brief Sends a ping to the service
     */
    Q_INVOKABLE void pingService();

    // Plugin management
    /**
     * @brief Loads a plugin from the specified path
     * @param pluginPath Path to the plugin DLL
     */
    Q_INVOKABLE void loadPlugin(const QString& pluginPath);

    /**
     * @brief Unloads a plugin
     * @param pluginName Name of the plugin to unload
     */
    Q_INVOKABLE void unloadPlugin(const QString& pluginName);

    /**
     * @brief Gets the status of a plugin
     * @param pluginName Name of the plugin
     * @return Map containing plugin status information
     */
    Q_INVOKABLE QVariantMap getPluginStatus(const QString& pluginName);

    // Agent management
    /**
     * @brief Starts an agent
     * @param agentId ID of the agent to start
     */
    Q_INVOKABLE void startAgent(const QString& agentId);

    /**
     * @brief Stops an agent
     * @param agentId ID of the agent to stop
     */
    Q_INVOKABLE void stopAgent(const QString& agentId);

    /**
     * @brief Restarts an agent
     * @param agentId ID of the agent to restart
     */
    Q_INVOKABLE void restartAgent(const QString& agentId);

    // Properties
    /**
     * @brief Gets the service status
     * @return Service status string
     */
    QString serviceStatus() const;

    /**
     * @brief Gets the CPU usage percentage
     * @return CPU usage percentage
     */
    double cpuUsage() const;

    /**
     * @brief Gets the memory usage percentage
     * @return Memory usage percentage
     */
    double memoryUsage() const;

    /**
     * @brief Gets the total memory in MB
     * @return Total memory in MB
     */
    double memoryTotal() const;

    /**
     * @brief Gets the available memory in MB
     * @return Available memory in MB
     */
    double memoryAvailable() const;

    /**
     * @brief Gets the log entries
     * @return List of log entries
     */
    QStringList logEntries() const;

    /**
     * @brief Gets the connected agents
     * @return List of connected agent IDs
     */
    QStringList connectedAgents() const;

    /**
     * @brief Gets the uptime in seconds
     * @return Uptime in seconds
     */
    int uptime() const;

signals:
    /**
     * @brief Emitted when the service status changes
     */
    void serviceStatusChanged();

    /**
     * @brief Emitted when the CPU usage changes
     */
    void cpuUsageChanged();

    /**
     * @brief Emitted when the memory usage changes
     */
    void memoryUsageChanged();

    /**
     * @brief Emitted when the total memory changes
     */
    void memoryTotalChanged();

    /**
     * @brief Emitted when the available memory changes
     */
    void memoryAvailableChanged();

    /**
     * @brief Emitted when log entries change
     */
    void logEntriesChanged();

    /**
     * @brief Emitted when connected agents change
     */
    void connectedAgentsChanged();

    /**
     * @brief Emitted when uptime changes
     */
    void uptimeChanged();

    /**
     * @brief Emitted when connected to the service
     */
    void serviceConnected();

    /**
     * @brief Emitted when disconnected from the service
     */
    void serviceDisconnected();

    /**
     * @brief Emitted when an error occurs
     * @param error Error message
     */
    void errorOccurred(const QString& error);

private slots:
    /**
     * @brief Updates system metrics
     */
    void updateMetrics();

    /**
     * @brief Updates the log stream
     */
    void updateLogStream();

    /**
     * @brief Checks the service status
     */
    void checkServiceStatus();

private:
    /**
     * @brief Initializes metrics
     */
    void initializeMetrics();

    /**
     * @brief Adds a log entry
     * @param level Log level (INFO, WARNING, ERROR)
     * @param message Log message
     */
    void addLogEntry(const QString& level, const QString& message);

    /**
     * @brief Connects to the service via IPC
     */
    void connectToService();

    /**
     * @brief Disconnects from the service
     */
    void disconnectFromService();

    // System metrics
    /**
     * @brief Gets the CPU usage percentage
     * @return CPU usage percentage
     */
    double getCpuUsage();

    /**
     * @brief Gets the memory usage percentage
     * @return Memory usage percentage
     */
    double getMemoryUsage();

    // Service connection
    bool m_serviceConnected;
    QString m_serviceStatus;

    // Metrics
    double m_cpuUsage;
    double m_memoryUsage;
    double m_memoryTotal;
    double m_memoryAvailable;
    int m_uptime;
    QDateTime m_startTime;

    // Data
    QStringList m_logEntries;
    QStringList m_connectedAgents;
    QVariantMap m_pluginStatuses;

    // Timers
    QTimer* m_metricsTimer;
    QTimer* m_logTimer;
    QTimer* m_statusTimer;

    // Constants
    static const int MAX_LOG_ENTRIES = 1000;
    static const int METRICS_UPDATE_INTERVAL = 2000; // 2 seconds
    static const int LOG_UPDATE_INTERVAL = 1000;    // 1 second
    static const int STATUS_CHECK_INTERVAL = 5000;   // 5 seconds
};
