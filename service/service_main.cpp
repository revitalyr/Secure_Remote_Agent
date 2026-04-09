// service/service_main.cpp
#include <windows.h>
#include <iostream>

void runServiceLogic() {
    while (true) {
        std::cout << "Service heartbeat" << std::endl;
        Sleep(5000);
    }
}

int main() {
    runServiceLogic();
    return 0;
}

