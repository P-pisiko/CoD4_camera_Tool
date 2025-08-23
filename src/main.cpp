// main.cpp : This file contains the 'main' function. 
//

#include <iostream>
#include <Windows.h>
#include <string>
void console();
HANDLE connectToPipe(const char*);


int main()
{
    std::cout << "Hello World!\n"; // for good luck

    const char* pipeName = R"(\\.\pipe\MyPipe)";

    HANDLE hPipe = connectToPipe(pipeName);

    OVERLAPPED overlapped = {};
    //overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    overlapped.hEvent = NULL;


    while (!GetAsyncKeyState(VK_ESCAPE)) {
        SHORT keyState = GetAsyncKeyState(VK_NUMPAD5);
        std::int16_t v = (keyState & 0x8000) ? 1 : 0; // high bit set if pressed |  0x8000: Hold down some time | 0x0001 press sinse last checked

        OVERLAPPED ov = {};
        DWORD bytesWritten = 0;

        BOOL success = WriteFile(
            hPipe,
            &v,
            sizeof(v),
            &bytesWritten,
            &ov
        );

        if (!success) {
            DWORD err = GetLastError();
            if (err == ERROR_IO_PENDING) {
                std::cout << "Write pending, dont really care..\n";
            }
            else {
                std::cerr << "WriteFile failed. Error: " << err << std::endl;
                break;
            }
        }
        else {
            std::cout << "Sent: " << v << " (" << bytesWritten << " bytes)\n";
        }
        Sleep(150);
    }
}

void console() { // 
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