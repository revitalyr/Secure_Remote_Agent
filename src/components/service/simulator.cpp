// service/simulator.cpp
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <iostream>
#include "service_controller.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Secure Remote Agent Demo Simulator ===";
    qDebug() << "This simulator runs the service functional code without Windows Service infrastructure";
    qDebug() << "Press Ctrl+C to stop";
    qDebug() << Qt::endl;
    
    // Create service controller
    ServiceController controller(&app);
    
    // Connect signals for status updates
    QObject::connect(&controller, &ServiceController::started, []() {
        qDebug() << "[SIMULATOR] Service controller started";
        qDebug() << "[SIMULATOR] - IPC server running";
        qDebug() << "[SIMULATOR] - Plugins loaded";
        qDebug() << "[SIMULATOR] - Sending metrics to backend";
    });
    
    QObject::connect(&controller, &ServiceController::stopped, []() {
        std::cout << "[SIMULATOR] Service controller stopped" << std::endl;
        QCoreApplication::quit();
    });
    
    QObject::connect(&controller, &ServiceController::paused, []() {
        std::cout << "[SIMULATOR] Service controller paused" << std::endl;
    });
    
    QObject::connect(&controller, &ServiceController::resumed, []() {
        std::cout << "[SIMULATOR] Service controller resumed" << std::endl;
    });
    
    // Start the controller
    controller.start();
    
    // Run Qt event loop
    return app.exec();
}
