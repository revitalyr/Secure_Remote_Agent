#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "components/app/ui_manager.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<UiManager>("SecureAgent", 1, 0, "UiManager");

    UiManager* uiManager = new UiManager(&app);
    engine.rootContext()->setContextProperty("uiManager", uiManager);

    engine.load(QUrl("qrc:/main.qml"));

    if (engine.rootObjects().isEmpty()) {
        QString qmlPath = QCoreApplication::applicationDirPath() + "/main.qml";
        engine.load(QUrl::fromLocalFile(qmlPath));
    }

    return app.exec();
}
