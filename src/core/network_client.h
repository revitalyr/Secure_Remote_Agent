/**
 * @file network_client.h
 * @brief Network client for HTTP/HTTPS communication
 */

#pragma once
#include <QtNetwork>

/**
 * @brief Network client for HTTP/HTTPS requests
 *
 * Provides a Qt-based HTTP client for sending JSON data to remote
 * servers. Supports POST requests with JSON payloads and handles
 * SSL/TLS connections.
 */
class NetworkClient : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Constructs a network client
     * @param parent Parent QObject (default: nullptr)
     */
    explicit NetworkClient(QObject* parent = nullptr);

    /**
     * @brief Sends a JSON POST request to the specified URL
     * @param url Target URL for the POST request
     * @param obj JSON object to send in the request body
     */
    void postJson(const QUrl& url, const QJsonObject& obj);

signals:
    /**
     * @brief Emitted when a response is received from the server
     * @param data Raw response data
     */
    void responseReceived(QByteArray data);

private:
    QNetworkAccessManager manager;
};

