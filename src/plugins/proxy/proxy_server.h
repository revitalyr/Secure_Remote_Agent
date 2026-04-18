/**
 * @file proxy_server.h
 * @brief Proxy server for HTTP/HTTPS traffic
 */

#pragma once
#include <QTcpServer>

/**
 * @brief Proxy server for handling HTTP/HTTPS connections
 *
 * Extends QTcpServer to handle incoming proxy connections.
 * Manages socket descriptors for proxy traffic forwarding.
 */
class ProxyServer : public QTcpServer {
    Q_OBJECT
protected:
    /**
     * @brief Handles incoming connection
     * @param socketDescriptor Socket descriptor for the new connection
     */
    void incomingConnection(qintptr socketDescriptor) override;
};

