// proxy/proxy_server.h
#pragma once
#include <QTcpServer>

class ProxyServer : public QTcpServer {
    Q_OBJECT
protected:
    void incomingConnection(qintptr socketDescriptor) override;
};

