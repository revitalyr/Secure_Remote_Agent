// core/network_client.h
#pragma once
#include <QtNetwork>

class NetworkClient : public QObject {
    Q_OBJECT
public:
    explicit NetworkClient(QObject* parent = nullptr);

    void postJson(const QUrl& url, const QJsonObject& obj);

signals:
    void responseReceived(QByteArray data);

private:
    QNetworkAccessManager manager;
};

