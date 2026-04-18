/**
 * @file remote_executor.h
 * @brief Remote command executor for agent operations
 */

#pragma once
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QProcess>
#include <QTimer>

/**
 * @brief Remote command executor for agent operations
 *
 * Provides secure remote command execution capabilities including
 * file operations, process management, and system commands.
 * Supports command validation, timeout handling, and result streaming.
 */
class RemoteExecutor : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Constructs a remote executor
     * @param parent Parent QObject (default: nullptr)
     */
    explicit RemoteExecutor(QObject* parent = nullptr);

    /**
     * @brief Destroys the remote executor
     */
    ~RemoteExecutor();

    /**
     * @brief Sets the allowed command whitelist
     * @param commands List of allowed command names
     */
    void setCommandWhitelist(const QStringList& commands);

    /**
     * @brief Sets the execution timeout in milliseconds
     * @param timeoutMs Timeout in milliseconds
     */
    void setExecutionTimeout(int timeoutMs);

    /**
     * @brief Executes a system command
     * @param command Command to execute
     * @param args Command arguments
     * @return Result map with exit code, stdout, stderr
     */
    QVariantMap executeCommand(const QString& command, const QStringList& args);

    /**
     * @brief Executes a command asynchronously
     * @param command Command to execute
     * @param args Command arguments
     */
    void executeCommandAsync(const QString& command, const QStringList& args);

    /**
     * @brief Executes a script file
     * @param scriptPath Path to the script file
     * @param args Script arguments
     * @return Result map with exit code, stdout, stderr
     */
    QVariantMap executeScript(const QString& scriptPath, const QStringList& args);

    /**
     * @brief Lists files in a directory
     * @param directoryPath Directory path
     * @return List of file entries
     */
    QVariantMap listDirectory(const QString& directoryPath);

    /**
     * @brief Gets file information
     * @param filePath File path
     * @return File information map
     */
    QVariantMap getFileInfo(const QString& filePath);

    /**
     * @brief Deletes a file
     * @param filePath File path
     * @return true if deletion succeeded, false otherwise
     */
    bool deleteFile(const QString& filePath);

    /**
     * @brief Creates a directory
     * @param directoryPath Directory path
     * @return true if creation succeeded, false otherwise
     */
    bool createDirectory(const QString& directoryPath);

    /**
     * @brief Kills a process by PID
     * @param pid Process ID
     * @return true if kill succeeded, false otherwise
     */
    bool killProcess(qint64 pid);

    /**
     * @brief Lists running processes
     * @return List of process information
     */
    QVariantMap listProcesses();

signals:
    /**
     * @brief Emitted when command execution completes
     * @param result Result map
     */
    void commandCompleted(QVariantMap result);

    /**
     * @brief Emitted when command output is received
     * @param output Output text
     */
    void commandOutput(const QString& output);

    /**
     * @brief Emitted when command execution fails
     * @param error Error message
     */
    void commandError(const QString& error);

private slots:
    /**
     * @brief Handles process completion
     */
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief Handles process error
     */
    void onProcessError(QProcess::ProcessError error);

    /**
     * @brief Handles process ready to read standard output
     */
    void onProcessReadyReadStandardOutput();

    /**
     * @brief Handles process ready to read standard error
     */
    void onProcessReadyReadStandardError();

    /**
     * @brief Handles execution timeout
     */
    void onExecutionTimeout();

private:
    /**
     * @brief Validates command against whitelist
     * @param command Command to validate
     * @return true if command is allowed, false otherwise
     */
    bool validateCommand(const QString& command);

    /**
     * @brief Sanitizes command arguments
     * @param args Arguments to sanitize
     * @return Sanitized arguments
     */
    QStringList sanitizeArguments(const QStringList& args);

    QStringList commandWhitelist;
    int executionTimeout;
    QProcess* currentProcess;
    QTimer* timeoutTimer;
    QString currentOutput;
    QString currentError;
};
