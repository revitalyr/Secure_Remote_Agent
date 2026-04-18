/**
 * @file network_client.h
 * @brief Network client for HTTP/HTTPS communication with mTLS support
 */

#pragma once
#include <QtNetwork>
#include <QSslCertificate>
#include <QSslKey>
#include <QTimer>

/**
 * @brief Network client for HTTP/HTTPS requests
 *
 * Provides a Qt-based HTTP client for sending JSON data to remote
 * servers. Supports POST requests with JSON payloads, mTLS with
 * client certificates, certificate pinning, and automatic reconnection
 * with exponential backoff.
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
     * @brief Destroys the network client
     */
    ~NetworkClient();

    /**
     * @brief Sets the client certificate for mTLS
     * @param certificate Client certificate
     * @param privateKey Private key
     */
    void setClientCertificate(const QSslCertificate& certificate, const QSslKey& privateKey);

    /**
     * @brief Sets the CA certificate for server verification
     * @param certificate CA certificate
     */
    void setCaCertificate(const QSslCertificate& certificate);

    /**
     * @brief Sets the expected server certificate for pinning
     * @param certificate Expected server certificate
     */
    void setPinnedCertificate(const QSslCertificate& certificate);

    /**
     * @brief Enables or disables certificate verification
     * @param enabled true to enable verification, false to disable
     */
    void setCertificateVerification(bool enabled);

    /**
     * @brief Sets the reconnection strategy parameters
     * @param maxRetries Maximum number of reconnection attempts
     * @param initialDelay Initial delay in milliseconds
     * @param maxDelay Maximum delay in milliseconds
     */
    void setReconnectStrategy(int maxRetries, int initialDelay, int maxDelay);

    /**
     * @brief Sends a JSON POST request to the specified URL
     * @param url Target URL for the POST request
     * @param obj JSON object to send in the request body
     */
    void postJson(const QUrl& url, const QJsonObject& obj);

    /**
     * @brief Sends a file to the specified URL
     * @param url Target URL
     * @param filePath Path to the file to send
     */
    void postFile(const QUrl& url, const QString& filePath);

signals:
    /**
     * @brief Emitted when a response is received from the server
     * @param data Raw response data
     */
    void responseReceived(QByteArray data);

    /**
     * @brief Emitted when an error occurs
     * @param error Error message
     */
    void errorOccurred(const QString& error);

    /**
     * @brief Emitted when reconnection is attempted
     * @param attempt Current attempt number
     * @param maxAttempts Maximum attempts
     */
    void reconnectAttempt(int attempt, int maxAttempts);

private slots:
    /**
     * @brief Handles SSL errors
     * @param reply Network reply with SSL errors
     * @param errors List of SSL errors
     */
    void onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors);

    /**
     * @brief Handles request completion
     */
    void onRequestFinished();

    /**
     * @brief Handles request timeout
     */
    void onRequestTimeout();

    /**
     * @brief Attempts to reconnect
     */
    void attemptReconnect();

private:
    /**
     * @brief Configures SSL configuration for the request
     * @param request Network request to configure
     */
    void configureSsl(QNetworkRequest& request);

    /**
     * @brief Verifies the server certificate against pinned certificate
     * @param certificate Server certificate
     * @return true if certificate matches pin, false otherwise
     */
    bool verifyPinnedCertificate(const QSslCertificate& certificate);

    /**
     * @brief Calculates exponential backoff delay
     * @param attempt Current attempt number
     * @return Delay in milliseconds
     */
    qint64 calculateBackoff(int attempt);

    QNetworkAccessManager manager;
    QSslCertificate clientCertificate;
    QSslKey clientPrivateKey;
    QSslCertificate caCertificate;
    QSslCertificate pinnedCertificate;
    bool certificateVerificationEnabled;
    int maxRetries;
    int initialDelay;
    int maxDelay;
    int currentRetry;
    QUrl lastUrl;
    QJsonObject lastData;
    QString lastFilePath;
    QTimer* reconnectTimer;
    bool isFileTransfer;
};

