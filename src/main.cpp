// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <string>
void console();
HANDLE connectToPipe(const char*);


int main()
{
    std::cout << "Hello World!\n";
    const char* pipeName = R"(\\.\pipe\MyPipe)";


}

void console() {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONIN$", "r", stdin);
    std::cout << "Console Created" << std::endl;
    while (true) {
        std::string input;
        std::cin >> input;

        if (input == "exit") {
            break;
        }
        if (input == "") {
            continue;

        }
        if (input == "") {
            continue;
        }
    }
    FreeConsole();
}

HANDLE connectToPipe(const char* pipeName) {
    HANDLE hPipe = CreateFileA(
        pipeName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to connect. Err: " << GetLastError() << std::endl;
        exit(1);
    }

    std::cout << "Connected to pipe" << std::endl;
    return hPipe;
}