// service/service_main.cpp
#include <windows.h>
#include <winsvc.h>
#include <iostream>
#include "service_controller.h"
#include "../../core/ipc_server.h"
#include <QCoreApplication>
#include <thread>
#include <atomic>

// Service globals
SERVICE_STATUS_HANDLE g_statusHandle = nullptr;
SERVICE_STATUS g_serviceStatus{};
std::atomic<bool> g_running{true};
std::atomic<bool> g_paused{false};
HANDLE g_serviceStopEvent = nullptr;
ServiceController* g_controller = nullptr;
QCoreApplication* g_app = nullptr;

// Service name
#define SERVICE_NAME L"SecureRemoteAgent"

// Forward declarations
VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler(DWORD ctrlCode);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

void ReportServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode = NO_ERROR, DWORD dwWaitHint = 0) {
    static DWORD dwCheckPoint = 1;

    g_serviceStatus.dwCurrentState = dwCurrentState;
    g_serviceStatus.dwWin32ExitCode = dwWin32ExitCode;
    g_serviceStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING) {
        g_serviceStatus.dwControlsAccepted = 0;
    } else {
        g_serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
    }

    if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED)) {
        g_serviceStatus.dwCheckPoint = 0;
    } else {
        g_serviceStatus.dwCheckPoint = dwCheckPoint++;
    }

    SetServiceStatus(g_statusHandle, &g_serviceStatus);
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam) {
    // Initialize Qt application
    int argc = 0;
    char** argv = nullptr;
    g_app = new QCoreApplication(argc, argv);
    
    // Create and start service controller
    g_controller = new ServiceController(g_app);
    
    // Connect signals to service status updates
    QObject::connect(g_controller, &ServiceController::started, []() {
        ReportServiceStatus(SERVICE_RUNNING);
    });
    
    QObject::connect(g_controller, &ServiceController::stopped, []() {
        ReportServiceStatus(SERVICE_STOPPED);
    });
    
    QObject::connect(g_controller, &ServiceController::paused, []() {
        ReportServiceStatus(SERVICE_PAUSED);
    });
    
    QObject::connect(g_controller, &ServiceController::resumed, []() {
        ReportServiceStatus(SERVICE_RUNNING);
    });
    
    // Start the controller
    g_controller->start();
    
    // Wait for service stop
    while (g_running) {
        if (WaitForSingleObject(g_serviceStopEvent, 1000) == WAIT_OBJECT_0) {
            break;
        }
        g_app->processEvents();
    }
    
    // Cleanup
    g_controller->stop();
    delete g_controller;
    g_controller = nullptr;
    delete g_app;
    g_app = nullptr;
    
    return ERROR_SUCCESS;
}

VOID WINAPI ServiceCtrlHandler(DWORD ctrlCode) {
    switch (ctrlCode) {
        case SERVICE_CONTROL_STOP:
            if (g_serviceStatus.dwCurrentState != SERVICE_RUNNING) {
                break;
            }
            
            ReportServiceStatus(SERVICE_STOP_PENDING);
            
            // Signal worker thread to stop
            g_running = false;
            SetEvent(g_serviceStopEvent);
            
            ReportServiceStatus(SERVICE_STOPPED);
            break;
            
        case SERVICE_CONTROL_PAUSE:
            if (g_serviceStatus.dwCurrentState != SERVICE_RUNNING) {
                break;
            }
            
            ReportServiceStatus(SERVICE_PAUSE_PENDING);
            g_paused = true;
            if (g_controller) {
                g_controller->pause();
            }
            ReportServiceStatus(SERVICE_PAUSED);
            break;
            
        case SERVICE_CONTROL_CONTINUE:
            if (g_serviceStatus.dwCurrentState != SERVICE_PAUSED) {
                break;
            }
            
            ReportServiceStatus(SERVICE_CONTINUE_PENDING);
            g_paused = false;
            if (g_controller) {
                g_controller->resume();
            }
            ReportServiceStatus(SERVICE_RUNNING);
            break;
            
        case SERVICE_CONTROL_INTERROGATE:
            break;
            
        default:
            break;
    }
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv) {
    // Register service control handler
    g_statusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (g_statusHandle == nullptr) {
        return;
    }
    
    // Initialize service status
    g_serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_serviceStatus.dwServiceSpecificExitCode = 0;
    
    // Report service starting
    ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000);
    
    // Create service stop event
    g_serviceStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (g_serviceStopEvent == nullptr) {
        ReportServiceStatus(SERVICE_STOPPED, GetLastError());
        return;
    }
    
    // Start worker thread
    HANDLE hThread = CreateThread(nullptr, 0, ServiceWorkerThread, nullptr, 0, nullptr);
    if (hThread == nullptr) {
        ReportServiceStatus(SERVICE_STOPPED, GetLastError());
        CloseHandle(g_serviceStopEvent);
        return;
    }
    
    // Wait for service stop
    WaitForSingleObject(hThread, INFINITE);
    
    // Cleanup
    CloseHandle(hThread);
    CloseHandle(g_serviceStopEvent);
}

int main(int argc, char* argv[]) {
    SERVICE_TABLE_ENTRY serviceTable[] = {
        { (LPWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { nullptr, nullptr }
    };
    
    if (StartServiceCtrlDispatcher(serviceTable) == FALSE) {
        DWORD error = GetLastError();
        if (error == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
            // Running in console mode for debugging
            std::cout << "Running in console mode (debug)" << std::endl;
            
            // Initialize Qt application
            QCoreApplication app(argc, argv);
            
            // Create and start service controller directly
            ServiceController controller(&app);
            controller.start();
            
            // Connect signals
            QObject::connect(&controller, &ServiceController::started, []() {
                std::cout << "Service started" << std::endl;
            });
            
            QObject::connect(&controller, &ServiceController::stopped, []() {
                std::cout << "Service stopped" << std::endl;
            });
            
            // Run Qt event loop
            return app.exec();
        } else {
            std::cerr << "StartServiceCtrlDispatcher failed with error: " << error << std::endl;
            return 1;
        }
    }
    
    return 0;
}
