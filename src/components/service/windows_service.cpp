// service/windows_service.cpp
#include <windows.h>
#include <thread>
#include <atomic>

SERVICE_STATUS_HANDLE g_statusHandle;
std::atomic<bool> running = true;

void WINAPI ServiceCtrlHandler(DWORD ctrl) {
    if (ctrl == SERVICE_CONTROL_STOP) {
        running = false;
    }
}

void ServiceWorker() {
    startIpcServer();

    while (running) {
        Sleep(3000);
    }
}

void WINAPI ServiceMain(DWORD, LPTSTR*) {
    g_statusHandle = RegisterServiceCtrlHandler("MyService", ServiceCtrlHandler);

    SERVICE_STATUS status{};
    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_statusHandle, &status);

    std::thread worker(ServiceWorker);
    worker.join();

    status.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_statusHandle, &status);
}

int main() {
    SERVICE_TABLE_ENTRY table[] = {
        { (LPSTR)"MyService", ServiceMain },
        { NULL, NULL }
    };

    StartServiceCtrlDispatcher(table);
    return 0;
}
