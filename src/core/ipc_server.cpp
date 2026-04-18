// core/ipc_server.cpp
#include <QLocalServer>
#include <QLocalSocket>
#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include "ipc_server.h"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

static QLocalServer* g_server = nullptr;

QJsonObject getRealSystemMetrics() {
    QJsonObject metrics;
    
#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);
    
    metrics["memory_percent"] = (qint64)memoryStatus.dwMemoryLoad;
    metrics["memory_total_mb"] = (qint64)(memoryStatus.ullTotalPhys / (1024 * 1024));
    metrics["memory_available_mb"] = (qint64)(memoryStatus.ullAvailPhys / (1024 * 1024));
    
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        static ULONGLONG lastIdle = 0, lastKernel = 0, lastUser = 0;
        ULONGLONG idle = (((ULONGLONG)idleTime.dwHighDateTime) << 32) | idleTime.dwLowDateTime;
        ULONGLONG kernel = (((ULONGLONG)kernelTime.dwHighDateTime) << 32) | kernelTime.dwLowDateTime;
        ULONGLONG user = (((ULONGLONG)userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime;
        
        if (lastIdle != 0) {
            ULONGLONG idleDiff = idle - lastIdle;
            ULONGLONG kernelDiff = kernel - lastKernel;
            ULONGLONG userDiff = user - lastUser;
            
            ULONGLONG total = kernelDiff + userDiff;
            if (total > 0) {
                double cpuUsage = 100.0 * (1.0 - (double)idleDiff / (double)total);
                metrics["cpu_percent"] = cpuUsage;
            }
        }
        
        lastIdle = idle;
        lastKernel = kernel;
        lastUser = user;
    }
#else
    // Fallback for non-Windows
    metrics["cpu_percent"] = 0.0;
    metrics["memory_percent"] = 0.0;
#endif
    
    metrics["timestamp"] = QDateTime::currentSecsSinceEpoch();
    
    return metrics;
}

void startIpcServer() {
    if (g_server) return;
    
    g_server = new QLocalServer();
    
    // Remove any existing socket file
    QLocalServer::removeServer("service_ipc");
    
    if (!g_server->listen("service_ipc")) {
        qWarning() << "Failed to start IPC server:" << g_server->errorString();
        delete g_server;
        g_server = nullptr;
        return;
    }
    
    qDebug() << "IPC server started on 'service_ipc'";
    
    QObject::connect(g_server, &QLocalServer::newConnection, []() {
        QLocalSocket* client = g_server->nextPendingConnection();
        QObject::connect(client, &QLocalSocket::readyRead, [client]() {
            QByteArray data = client->readAll();
            QString request = QString::fromUtf8(data);
            
            // Handle different requests
            if (request.trimmed() == "ping") {
                client->write("pong");
                client->flush();
            } else if (request.trimmed() == "GET_METRICS") {
                QJsonObject metrics = getRealSystemMetrics();
                QJsonDocument doc(metrics);
                client->write(doc.toJson());
                client->flush();
            }
        });
        
        QObject::connect(client, &QLocalSocket::disconnected, [client]() {
            client->deleteLater();
        });
    });
}

void stopIpcServer() {
    if (g_server) {
        g_server->close();
        delete g_server;
        g_server = nullptr;
        qDebug() << "IPC server stopped";
    }
}

