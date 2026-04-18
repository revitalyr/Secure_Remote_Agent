/**
 * @file plugin_interface.h
 * @brief Plugin interface for dynamic plugin loading with agent commands
 */

#pragma once
#include <QtPlugin>
#include <QString>
#include <QVariantMap>
#include <functional>
#include <QStringList>

/**
 * @brief Plugin status enumeration
 */
enum class PluginStatus {
    Unloaded, ///< Plugin is not loaded
    Loaded,   ///< Plugin is loaded but not running
    Running,  ///< Plugin is actively running
    Error     ///< Plugin encountered an error
};

/**
 * @brief Command callback function type
 */
using CommandCallback = std::function<QVariantMap(const QVariantMap&)>;

/**
 * @brief Command descriptor structure
 */
struct CommandDescriptor {
    QString name;           ///< Command name
    QString description;    ///< Command description
    QStringList parameters; ///< Required parameters
    CommandCallback callback; ///< Command execution callback
};

/**
 * @brief Interface for dynamic plugin loading
 *
 * Defines the contract that all plugins must implement.
 * Supports lifecycle management, configuration, execution,
 * status monitoring, and agent command registration.
 */
class IPlugin {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~IPlugin() = default;

    // Basic plugin info
    /**
     * @brief Gets the plugin name
     * @return Plugin name
     */
    virtual QString name() const = 0;

    /**
     * @brief Gets the plugin version
     * @return Plugin version string
     */
    virtual QString version() const = 0;

    /**
     * @brief Gets the plugin description
     * @return Plugin description
     */
    virtual QString description() const = 0;

    // Lifecycle management
    /**
     * @brief Initializes the plugin with configuration
     * @param config Configuration map (default: empty)
     * @return true if initialization succeeded, false otherwise
     */
    virtual bool initialize(const QVariantMap& config = QVariantMap()) = 0;

    /**
     * @brief Starts the plugin
     */
    virtual void start() = 0;

    /**
     * @brief Stops the plugin
     */
    virtual void stop() = 0;

    /**
     * @brief Cleans up plugin resources
     */
    virtual void cleanup() = 0;

    // Execution
    /**
     * @brief Executes the plugin with default parameters
     */
    virtual void execute() = 0;

    /**
     * @brief Executes the plugin with custom parameters
     * @param params Execution parameters
     * @return Result map
     */
    virtual QVariantMap executeWithParams(const QVariantMap& params) = 0;

    // Agent commands
    /**
     * @brief Registers a command that can be executed by the agent
     * @param descriptor Command descriptor
     * @return true if registration succeeded, false otherwise
     */
    virtual bool registerCommand(const CommandDescriptor& descriptor) = 0;

    /**
     * @brief Unregisters a command
     * @param commandName Name of the command to unregister
     * @return true if unregistration succeeded, false otherwise
     */
    virtual bool unregisterCommand(const QString& commandName) = 0;

    /**
     * @brief Executes a registered command
     * @param commandName Name of the command to execute
     * @param params Command parameters
     * @return Result map
     */
    virtual QVariantMap executeCommand(const QString& commandName, const QVariantMap& params) = 0;

    /**
     * @brief Gets all registered commands
     * @return List of command names
     */
    virtual QStringList getRegisteredCommands() const = 0;

    /**
     * @brief Gets command descriptor
     * @param commandName Name of the command
     * @return Command descriptor
     */
    virtual CommandDescriptor getCommandDescriptor(const QString& commandName) const = 0;

    // Status and monitoring
    /**
     * @brief Gets the current plugin status
     * @return Plugin status
     */
    virtual PluginStatus status() const = 0;

    /**
     * @brief Gets plugin metrics
     * @return Metrics map
     */
    virtual QVariantMap getMetrics() const = 0;

    /**
     * @brief Gets the last error message
     * @return Error message string
     */
    virtual QString getLastError() const = 0;

    // Configuration
    /**
     * @brief Sets plugin configuration
     * @param config Configuration map
     * @return true if configuration was set successfully, false otherwise
     */
    virtual bool setConfig(const QVariantMap& config) = 0;

    /**
     * @brief Gets plugin configuration
     * @return Configuration map
     */
    virtual QVariantMap getConfig() const = 0;

    // Callback support for async operations
    /**
     * @brief Sets a callback for async operations
     * @param callback Callback function
     */
    virtual void setCallback(std::function<void(const QString&, const QVariantMap&)> callback) = 0;
};

#define IPlugin_iid "com.secureagent.IPlugin"
Q_DECLARE_INTERFACE(IPlugin, IPlugin_iid)

