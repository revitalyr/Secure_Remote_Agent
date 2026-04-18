/**
 * @file ipc_client.h
 * @brief IPC client for inter-process communication
 */

#pragma once

#include <QObject>
#include <QLocalSocket>
#include <QJsonObject>

/**
 * @brief IPC client for communicating with IPC server
 *
 * Provides a Qt-based client for inter-process communication using
 * QLocalSocket. Supports connection management, ping/pong protocol,
 * and system metrics retrieval.
 */
class IpcClient : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructs an IPC client
     * @param parent Parent QObject (default: nullptr)
     */
    IpcClient(QObject* parent = nullptr);

    /**
     * @brief Destroys the IPC client and disconnects from service
     */
    ~IpcClient();

    /**
     * @brief Connects to the IPC server
     * @param serverName Name of the IPC server to connect to
     */
    void connectToService(const QString& serverName);

    /**
     * @brief Disconnects from the IPC server
     */
    void disconnectFromService();

    /**
     * @brief Checks if client is connected to the server
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Sends a ping request to the server
     */
    void ping();

    /**
     * @brief Retrieves system metrics from the server
     * @return JSON object containing CPU, memory, and uptime metrics
     */
    QJsonObject getSystemMetrics();

signals:
    /**
     * @brief Emitted when successfully connected to the server
     */
    void connected();

    /**
     * @brief Emitted when disconnected from the server
     */
    void disconnected();

    /**
     * @brief Emitted when a pong response is received
     * @param message The pong message from the server
     */
    void pongReceived(const QString& message);

private:
    QLocalSocket* socket;
};
