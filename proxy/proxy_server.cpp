// proxy/proxy_server.cpp
#include "proxy_server.h"
#include <QTcpSocket>

void ProxyServer::incomingConnection(qintptr socketDescriptor) {
    auto socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::readyRead, [socket]() {
        auto data = socket->readAll();
        qDebug() << "Proxy captured:" << data;
    });
}

