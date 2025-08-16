#include <iostream>
#include <tchar.h>
#include <windows.h>
#include <TlHelp32.h>
#include <chrono>
#include <thread>
#include <ostream>
#include <string>

// allows to easily switch between retail and openjk version of the game
int debug = 0;

// path to the dll to be injected
char dllPath[256] = "C:\\Users\\T-Box\\source\\repos\\CoD4-dm1\\bin\\Release\\net8.0\\endsceen-hook.dll"; //ACTÝVE

std::string dllname = "endsceen-hook.dll";
BOOL FileExists(LPCTSTR path)
{
    DWORD dwAttrib = GetFileAttributes(path);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL checkAlreadyInjected(DWORD PID, std::string moduleName)
{
    // get a snapshot of all processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);

    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        std::cerr << "[ loader ] Invalid handle value" << std::endl;
        exit(99);
    }

    MODULEENTRY32 ModEntry;
    if (Module32First(hSnapshot, &ModEntry))
    {
        do
        {
            // check if the dll is already present
            if (!strcmp((char*)ModEntry.szModule, moduleName.c_str()))
            {
                CloseHandle(hSnapshot);
                return TRUE;
            }
        } while (Module32Next(hSnapshot, &ModEntry));
    }

    CloseHandle(hSnapshot);
    return FALSE;
}

extern "C" __declspec(dllexport) int loaderMain(const wchar_t* RootPath)
{

    std::wstring EXE_DIR;
    std::wstring EXE_NAME;
    if (debug == 0)
    {
        // openjk version C:\\Windows\\system32\\notepad.exe
        //"D:\\SteamLibrary\\steamapps\\common\\Call of Duty 4"
        EXE_DIR = RootPath;
        EXE_NAME = L"iw3mp.exe";
    }

    else
    {
        // Retail
        EXE_DIR = L"C:\\Windows\\system32";
        EXE_NAME = L"notepad.exe";
    }

    std::wstring EXE_PATH = std::wstring(RootPath) + L"\\" + EXE_NAME;
    // Launch Process
    // additional information
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // start the program up
    if (!CreateProcessW(EXE_PATH.c_str(), // the path
        NULL,             // Command line
        NULL,             // Process handle not inheritable
        NULL,             // Thread handle not inheritable
        FALSE,            // Set handle inheritance to FALSE
        0,                // No creation flags
        NULL,             // Use parent's environment block
        EXE_DIR.c_str(),  // Use parent's starting directory
        &si,              // Pointer to STARTUPINFO structure
        &pi               // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    ))
    {
        std::wcerr << "[ loader ] Can't create process" << std::endl;
        exit(88);
    }

    Sleep(1);

    // Close process and thread handles
    // this does not stop the game
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    PROCESSENTRY32W entry;
    DWORD PID = -1;
    BOOL found = FALSE;

    // Do Windows things
    entry.dwSize = sizeof(PROCESSENTRY32W);
    // Get all processes
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE)
    {
        std::cerr << "[ loader ] Can't get list of running processes" << std::endl;
        exit(1);
    }

    // Search for the process using the exe file name
    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            // check if a process with the executable file name is present
            if (wcscmp(entry.szExeFile, EXE_NAME.c_str()) == 0)
            {
                found = TRUE;
                PID = entry.th32ProcessID;
                std::cout << "[ loader ] Got process: Pid " << PID << " Entry " << entry.szExeFile << std::endl;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    else
    {
        std::cerr << "[ loader ] Can't process list of running processes" << std::endl;
        exit(2);
    }

    if (!found)
    {
        std::cerr << "[ loader ] Can't find process" << std::endl;
        exit(3);
    }

    if (!FileExists((LPCTSTR)dllPath))
    {
        std::cerr << "[ loader ] DLL not found" << std::endl;
        exit(4);
    }

    /*if (checkAlreadyInjected(PID, dllPath))
    {
        std::cerr << "Already injected" << std::endl;
        exit(5);
    }*/

    // Get a handle to the process
    HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
    if (!procHandle)
    {
        std::cerr << "[ loader ] Can't open process" << std::endl;
        exit(6);
    }

    // Get the address of LoadLibraryA in kernel32.dll, used to
    // pass it to the remote process in order to load our dll
    LPVOID loadFunctionAddress = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
    if (!loadFunctionAddress)
    {
        std::cerr << "[ loader ] LoadLibraryA not found" << std::endl;
        exit(7);
    }

    // Allocate space in the target process for our DLL path
    LPVOID allocatedMem = LPVOID(VirtualAllocEx(procHandle, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (!allocatedMem)
    {
        std::cerr << "[ loader ] Can't allocate memory in the target process" << std::endl;
        exit(8);
    }

    // Write the path of our DLL in the allocated memory
    if (!WriteProcessMemory(procHandle, allocatedMem, dllPath, MAX_PATH, nullptr))
    {
        std::cerr << "[ loader ] Can't write memory into the target process" << std::endl;
        exit(9);
    }

    // Load the DLL by causing the remote process to spawn a thread which calls LoadLibraryA
    // which loads the DLL using the path we allocated previously.
    HANDLE threadHandle = CreateRemoteThread(procHandle, nullptr, NULL, LPTHREAD_START_ROUTINE(loadFunctionAddress), allocatedMem, NULL, nullptr);
    if (!threadHandle)
    {
        std::cerr << " [ loader ] Can't start the remote thread" << std::endl;
        exit(10);
    }

    std::cout << "[ loader ] endsceen-hook is loaded" << std::endl;

    // work is done
    CloseHandle(procHandle);
    // free memory
    VirtualFreeEx(procHandle, LPVOID(allocatedMem), 0, MEM_RELEASE);

    std::cout << "[ loader ] Done!" << std::endl;
    return 0;
}