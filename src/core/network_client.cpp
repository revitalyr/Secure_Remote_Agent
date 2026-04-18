// core/network_client.cpp
#include "network_client.h"
#include <QFile>
#include <QHttpMultiPart>

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent)
    , certificateVerificationEnabled(true)
    , maxRetries(3)
    , initialDelay(1000)
    , maxDelay(30000)
    , currentRetry(0)
    , reconnectTimer(new QTimer(this))
    , isFileTransfer(false)
{
    reconnectTimer->setSingleShot(true);
    connect(reconnectTimer, &QTimer::timeout, this, &NetworkClient::attemptReconnect);

    connect(&manager, &QNetworkAccessManager::sslErrors, this, &NetworkClient::onSslErrors);
}

NetworkClient::~NetworkClient() {
    if (reconnectTimer) {
        reconnectTimer->stop();
    }
}

void NetworkClient::setClientCertificate(const QSslCertificate& certificate, const QSslKey& privateKey) {
    clientCertificate = certificate;
    clientPrivateKey = privateKey;
}

void NetworkClient::setCaCertificate(const QSslCertificate& certificate) {
    caCertificate = certificate;
}

void NetworkClient::setPinnedCertificate(const QSslCertificate& certificate) {
    pinnedCertificate = certificate;
}

void NetworkClient::setCertificateVerification(bool enabled) {
    certificateVerificationEnabled = enabled;
}

void NetworkClient::setReconnectStrategy(int maxRetries, int initialDelay, int maxDelay) {
    this->maxRetries = maxRetries;
    this->initialDelay = initialDelay;
    this->maxDelay = maxDelay;
}

void NetworkClient::postJson(const QUrl& url, const QJsonObject& obj) {
    lastUrl = url;
    lastData = obj;
    lastFilePath.clear();
    isFileTransfer = false;
    currentRetry = 0;

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    configureSsl(req);

    auto reply = manager.post(req, QJsonDocument(obj).toJson());

    connect(reply, &QNetworkReply::finished, this, &NetworkClient::onRequestFinished);
    connect(reply, &QNetworkReply::sslErrors, this, &NetworkClient::onSslErrors);
}

void NetworkClient::postFile(const QUrl& url, const QString& filePath) {
    lastUrl = url;
    lastFilePath = filePath;
    isFileTransfer = true;
    currentRetry = 0;

    QFile* file = new QFile(filePath, this);
    if (!file->open(QIODevice::ReadOnly)) {
        emit errorOccurred("Failed to open file: " + filePath);
        file->deleteLater();
        return;
    }

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(filePath).fileName() + "\""));
    filePart.setBodyDevice(file);
    file->setParent(multiPart);

    multiPart->append(filePart);

    QNetworkRequest req(url);
    configureSsl(req);

    auto reply = manager.post(req, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, &NetworkClient::onRequestFinished);
    connect(reply, &QNetworkReply::sslErrors, this, &NetworkClient::onSslErrors);
}

void NetworkClient::onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors) {
    if (!certificateVerificationEnabled) {
        reply->ignoreSslErrors();
        return;
    }

    QSslConfiguration sslConfig = reply->sslConfiguration();
    if (!pinnedCertificate.isNull()) {
        if (!verifyPinnedCertificate(sslConfig.peerCertificate())) {
            emit errorOccurred("Certificate pinning failed: server certificate does not match pinned certificate");
            reply->abort();
            return;
        }
    }

    for (const QSslError& error : errors) {
        if (error.error() != QSslError::CertificateUntrusted && 
            error.error() != QSslError::SelfSignedCertificate) {
            emit errorOccurred("SSL error: " + error.errorString());
            reply->abort();
            return;
        }
    }

    reply->ignoreSslErrors();
}

void NetworkClient::onRequestFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Network error: " + reply->errorString());

        if (currentRetry < maxRetries) {
            currentRetry++;
            emit reconnectAttempt(currentRetry, maxRetries);
            qint64 delay = calculateBackoff(currentRetry);
            reconnectTimer->start(delay);
        }
    } else {
        emit responseReceived(reply->readAll());
        currentRetry = 0;
    }

    reply->deleteLater();
}

void NetworkClient::onRequestTimeout() {
    emit errorOccurred("Request timeout");

    if (currentRetry < maxRetries) {
        currentRetry++;
        emit reconnectAttempt(currentRetry, maxRetries);
        qint64 delay = calculateBackoff(currentRetry);
        reconnectTimer->start(delay);
    }
}

void NetworkClient::attemptReconnect() {
    if (isFileTransfer && !lastFilePath.isEmpty()) {
        postFile(lastUrl, lastFilePath);
    } else {
        postJson(lastUrl, lastData);
    }
}

void NetworkClient::configureSsl(QNetworkRequest& request) {
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();

    if (!clientCertificate.isNull() && !clientPrivateKey.isNull()) {
        sslConfig.setLocalCertificate(clientCertificate);
        sslConfig.setPrivateKey(clientPrivateKey);
    }

    if (!caCertificate.isNull()) {
        sslConfig.setCaCertificates({caCertificate});
    }

    sslConfig.setProtocol(QSsl::TlsV1_3);
    sslConfig.setPeerVerifyMode(certificateVerificationEnabled ? QSslSocket::VerifyPeer : QSslSocket::VerifyNone);

    request.setSslConfiguration(sslConfig);
}

bool NetworkClient::verifyPinnedCertificate(const QSslCertificate& certificate) {
    if (pinnedCertificate.isNull()) {
        return true;
    }

    return certificate.digest(QCryptographicHash::Sha256) == pinnedCertificate.digest(QCryptographicHash::Sha256);
}

qint64 NetworkClient::calculateBackoff(int attempt) {
    qint64 delay = initialDelay * static_cast<qint64>(qPow(2, attempt - 1));
    return qMin(delay, static_cast<qint64>(maxDelay));
}

