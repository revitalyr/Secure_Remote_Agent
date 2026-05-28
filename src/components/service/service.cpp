#include <windows.h>
#include <winsvc.h>
#include <iostream>
#include "components/service/service_controller.h"
#include <QCoreApplication>
#include <thread>
#include <atomic>

SERVICE_STATUS_HANDLE g_statusHandle = nullptr;
SERVICE_STATUS g_serviceStatus{};
std::atomic<bool> g_running{true};
std::atomic<bool> g_paused{false};
HANDLE g_serviceStopEvent = nullptr;
ServiceController* g_controller = nullptr;
QCoreApplication* g_app = nullptr;

#define SERVICE_NAME L"SecureRemoteAgent"
#define SERVICE_DISPLAY_NAME L"Secure Remote Agent"
#define SERVICE_DESCRIPTION L"Desktop monitoring service with IPC, TLS, and plugin architecture"

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
    int argc = 0;
    char** argv = nullptr;
    g_app = new QCoreApplication(argc, argv);

    g_controller = new ServiceController(g_app);

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

    g_controller->start();

    while (g_running) {
        if (WaitForSingleObject(g_serviceStopEvent, 1000) == WAIT_OBJECT_0) {
            break;
        }
        g_app->processEvents();
    }

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
    g_statusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (g_statusHandle == nullptr) {
        return;
    }

    g_serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_serviceStatus.dwServiceSpecificExitCode = 0;

    ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    g_serviceStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (g_serviceStopEvent == nullptr) {
        ReportServiceStatus(SERVICE_STOPPED, GetLastError());
        return;
    }

    HANDLE hThread = CreateThread(nullptr, 0, ServiceWorkerThread, nullptr, 0, nullptr);
    if (hThread == nullptr) {
        ReportServiceStatus(SERVICE_STOPPED, GetLastError());
        CloseHandle(g_serviceStopEvent);
        return;
    }

    WaitForSingleObject(hThread, INFINITE);

    CloseHandle(hThread);
    CloseHandle(g_serviceStopEvent);
}

bool InstallService() {
    SC_HANDLE scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!scm) {
        std::cerr << "OpenSCManager failed: " << GetLastError() << std::endl;
        return false;
    }

    WCHAR path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wcout << L"Installing service with binary: " << path << std::endl;

    SC_HANDLE service = CreateServiceW(
        scm,
        SERVICE_NAME,
        SERVICE_DISPLAY_NAME,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        path,
        nullptr, nullptr, nullptr, nullptr, nullptr
    );

    if (!service) {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS) {
            std::cout << "Service already exists, reconfiguring..." << std::endl;
            service = OpenServiceW(scm, SERVICE_NAME, SERVICE_ALL_ACCESS);
            if (!service) {
                std::cerr << "Failed to open existing service: " << GetLastError() << std::endl;
                CloseServiceHandle(scm);
                return false;
            }
            ChangeServiceConfigW(service, SERVICE_NO_CHANGE, SERVICE_AUTO_START,
                                 SERVICE_NO_CHANGE, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        } else {
            std::cerr << "CreateService failed: " << err << std::endl;
            CloseServiceHandle(scm);
            return false;
        }
    }

    wchar_t descBuffer[] = SERVICE_DESCRIPTION;
    SERVICE_DESCRIPTIONW desc = { descBuffer };
    ChangeServiceConfig2W(service, SERVICE_CONFIG_DESCRIPTION, &desc);

    SERVICE_FAILURE_ACTIONSW failureActions = {};
    SC_ACTION actions[3] = {
        { SC_ACTION_RESTART, 5000 },
        { SC_ACTION_RESTART, 10000 },
        { SC_ACTION_RESTART, 30000 }
    };
    failureActions.dwResetPeriod = 86400;
    failureActions.lpRebootMsg = nullptr;
    failureActions.lpCommand = nullptr;
    failureActions.cActions = 3;
    failureActions.lpsaActions = actions;
    ChangeServiceConfig2W(service, SERVICE_CONFIG_FAILURE_ACTIONS, &failureActions);

    SERVICE_FAILURE_ACTIONS_FLAG flag = { TRUE };
    ChangeServiceConfig2W(service, SERVICE_CONFIG_FAILURE_ACTIONS_FLAG, &flag);

    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    std::cout << "Service installed successfully (auto-start, 3 restart attempts)" << std::endl;
    return true;
}

bool UninstallService() {
    SC_HANDLE scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!scm) {
        std::cerr << "OpenSCManager failed: " << GetLastError() << std::endl;
        return false;
    }

    SC_HANDLE service = OpenServiceW(scm, SERVICE_NAME, SERVICE_ALL_ACCESS);
    if (!service) {
        std::cerr << "Service not found: " << GetLastError() << std::endl;
        CloseServiceHandle(scm);
        return false;
    }

    SERVICE_STATUS status;
    ControlService(service, SERVICE_CONTROL_STOP, &status);

    if (!DeleteService(service)) {
        std::cerr << "DeleteService failed: " << GetLastError() << std::endl;
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return false;
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    std::cout << "Service uninstalled successfully" << std::endl;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--install" || arg == "-i") {
            return InstallService() ? 0 : 1;
        }
        if (arg == "--uninstall" || arg == "-u") {
            return UninstallService() ? 0 : 1;
        }
        std::cerr << "Usage: service_main [--install | --uninstall]" << std::endl;
        return 1;
    }

    SERVICE_TABLE_ENTRY serviceTable[] = {
        { (LPWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { nullptr, nullptr }
    };

    if (StartServiceCtrlDispatcher(serviceTable) == FALSE) {
        DWORD error = GetLastError();
        if (error == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
            std::cout << "Running in console mode (debug)" << std::endl;

            QCoreApplication app(argc, argv);

            ServiceController controller(&app);
            controller.start();

            QObject::connect(&controller, &ServiceController::started, []() {
                std::cout << "Service started" << std::endl;
            });

            QObject::connect(&controller, &ServiceController::stopped, []() {
                std::cout << "Service stopped" << std::endl;
            });

            return app.exec();
        } else {
            std::cerr << "StartServiceCtrlDispatcher failed with error: " << error << std::endl;
            return 1;
        }
    }

    return 0;
}
