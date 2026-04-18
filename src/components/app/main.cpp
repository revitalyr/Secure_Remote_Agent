// app/main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "ui_manager.h"
#include "../../core/ipc_client.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    
    // Register custom types
    qmlRegisterType<UiManager>("SecureAgent", 1, 0, "UiManager");
    qmlRegisterType<IpcClient>("SecureAgent", 1, 0, "IpcClient");
    
    // Create and expose UI manager (heap-allocated to ensure it stays alive)
    UiManager* uiManager = new UiManager(&app);
    engine.rootContext()->setContextProperty("uiManager", uiManager);
    
    // Create and expose IPC client (heap-allocated to ensure it stays alive)
    IpcClient* ipcClient = new IpcClient(&app);
    engine.rootContext()->setContextProperty("ipcClient", ipcClient);
    
    // Try loading from resource first
    engine.load(QUrl("qrc:/main.qml"));
    
    // Fallback: load from filesystem if resource fails
    if (engine.rootObjects().isEmpty()) {
        QString qmlPath = QCoreApplication::applicationDirPath() + "/main.qml";
        engine.load(QUrl::fromLocalFile(qmlPath));
    }

    return app.exec();
}
