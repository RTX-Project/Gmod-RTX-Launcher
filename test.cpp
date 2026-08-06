#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#pragma comment(lib, "user32.lib")

int main() {
    {
        std::wofstream f("test_launcher_settings.txt", std::ios::trunc);
        f << L"hasLaunchedGame=1\n";
        f << L"receiveBetaUpdates=1\n";
    }

    bool receiveBetaUpdates = false;
    std::wifstream f("test_launcher_settings.txt");
    std::wstring line;
    while (std::getline(f, line)) {
        if (line.rfind(L"receiveBetaUpdates=", 0) == 0) {
            receiveBetaUpdates = (_wtoi(line.substr(19).c_str()) != 0);
        }
    }
    std::cout << "receiveBetaUpdates is " << (receiveBetaUpdates ? "true" : "false") << std::endl;
    return 0;
}
