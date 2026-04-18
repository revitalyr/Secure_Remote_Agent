// core/network_client.cpp
#include "network_client.h"

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent) {}

void NetworkClient::postJson(const QUrl& url, const QJsonObject& obj) {
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    auto reply = manager.post(req, QJsonDocument(obj).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        emit responseReceived(reply->readAll());
        reply->deleteLater();
    });
}

