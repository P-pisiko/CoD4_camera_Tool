// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>

int main()
{
    std::cout << "Hello World!\n";
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