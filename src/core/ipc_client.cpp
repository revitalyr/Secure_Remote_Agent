// core/ipc_client.cpp
#include "ipc_client.h"
#include <QLocalSocket>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

IpcClient::IpcClient(QObject* parent) : QObject(parent) {
    socket = new QLocalSocket(this);
    connect(socket, &QLocalSocket::connected, this, &IpcClient::connected);
    connect(socket, &QLocalSocket::disconnected, this, &IpcClient::disconnected);
}

IpcClient::~IpcClient() {
    if (socket) {
        socket->disconnectFromServer();
        delete socket;
    }
}

void IpcClient::connectToService(const QString& serverName) {
    socket->connectToServer(serverName);
    if (!socket->waitForConnected(3000)) {
        qWarning() << "Failed to connect to IPC server:" << socket->errorString();
    }
}

void IpcClient::disconnectFromService() {
    socket->disconnectFromServer();
}

bool IpcClient::isConnected() const {
    return socket->state() == QLocalSocket::ConnectedState;
}

void IpcClient::ping() {
    if (!isConnected()) {
        qWarning() << "Not connected to service";
        return;
    }
    
    socket->write("ping");
    socket->flush();
    
    if (socket->waitForReadyRead(1000)) {
        QByteArray response = socket->readAll();
        QString message = QString::fromUtf8(response);
        emit pongReceived(message);
    }
}

QJsonObject IpcClient::getSystemMetrics() {
    QJsonObject metrics;
    
    if (!isConnected()) {
        qWarning() << "Not connected to service";
        metrics["error"] = "Not connected to service";
        return metrics;
    }
    
    socket->write("GET_METRICS");
    socket->flush();
    
    if (socket->waitForReadyRead(2000)) {
        QByteArray response = socket->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        if (doc.isObject()) {
            metrics = doc.object();
        }
    } else {
        qWarning() << "Timeout waiting for metrics response";
        metrics["error"] = "Timeout";
    }
    
    return metrics;
}

