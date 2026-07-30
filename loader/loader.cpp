#include <iostream>
#include <tchar.h>
#include <windows.h>
#include <TlHelp32.h>
#include <chrono>
#include <thread>
#include <ostream>
#include <string>
#include <cstdio>
#include <cstring>

int debug = 0;

// path to the dll to be injected
char dllPath[MAX_PATH] = ""; 

std::string dllname = "endsceen-hook.dll";

bool GetCrtDir(char* outDir, size_t outSize)
{
    char exePath[MAX_PATH];
    if (!GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
        return false;
    }

    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) {
        *lastSlash = '\0'; 
    }

    if (strlen(exePath) + 1 > outSize) {
        return false; 
    }

    strcpy_s(outDir, outSize, exePath);
    return true;
}

BOOL FileExists(LPCTSTR path)
{
    DWORD dwAttrib = GetFileAttributes(path);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}



BOOL FindProcessIdByName(const std::wstring& exeName, DWORD& outPid)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    BOOL found = FALSE;
    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            if (_wcsicmp(entry.szExeFile, exeName.c_str()) == 0)
            {
                found = TRUE;
                outPid = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return found;
}

// Checks whether `moduleName` is already loaded as a module inside process
// `PID`. Used to detect a prior injection so we never re-hook an
// already-hooked EndScene (that would clobber the trampoline's saved
BOOL checkAlreadyInjected(DWORD PID, const std::string& moduleName)
{
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;

    // module snapshots can transiently fail immediately after a process is
    // created/attached to; retry briefly instead of giving up (or nuking the
    // whole host process, which previously happened via exit(99) here).
    for (int attempt = 0; attempt < 10; ++attempt)
    {
        hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
        if (hSnapshot != INVALID_HANDLE_VALUE)
        {
            break;
        }
        Sleep(50);
    }

    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        std::cerr << "[ loader ] Could not snapshot modules for PID " << PID
            << " (already-injected check skipped)" << std::endl;
        return FALSE;
    }

    BOOL isInjected = FALSE;
    MODULEENTRY32 ModEntry;
    ModEntry.dwSize = sizeof(MODULEENTRY32);
    if (Module32First(hSnapshot, &ModEntry))
    {
        do
        {
            if (!_stricmp(ModEntry.szModule, moduleName.c_str()))
            {
                isInjected = TRUE;
                break;
            }
        } while (Module32Next(hSnapshot, &ModEntry));
    }

    CloseHandle(hSnapshot);
    return isInjected;
}

extern "C" __declspec(dllexport) int loaderMain(const wchar_t* RootPath)
{
    char crtDir[MAX_PATH];
    if (!GetCrtDir(crtDir, MAX_PATH)) {
        std::cerr << "Failed to get current directory for some reason" << std::endl;
        return 1;
    }
    sprintf_s(dllPath, "%s\\endsceen-hook.dll", crtDir);

    std::wstring EXE_DIR;
    std::wstring EXE_NAME;
    if (debug == 0)
    {
        
        //"Games Root Path"
        EXE_DIR = RootPath;
        EXE_NAME = L"iw3mp.exe";
    }

    else
    {
        EXE_DIR = L"C:\\Windows\\system32";
        EXE_NAME = L"notepad.exe";
    }

    
    

    DWORD PID = 0;
    BOOL alreadyRunning = FindProcessIdByName(EXE_NAME, PID);

    if (alreadyRunning)
    {
        std::cout << "[ loader ] Found already-running process: Pid " << PID << std::endl;
    }
    else
    {
        std::wstring EXE_PATH = EXE_DIR + L"\\" + EXE_NAME;
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

        Sleep(50);

        // Close process and thread handles
        // this does not stop the game
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        // The process may not be immediately enumerable right after
        // CreateProcessW returns; poll briefly instead of scanning once.
        BOOL found = FALSE;
        for (int attempt = 0; attempt < 40 && !found; ++attempt)
        {
            found = FindProcessIdByName(EXE_NAME, PID);
            if (!found)
            {
                Sleep(50);
            }
        }

        if (!found)
        {
            std::cerr << "[ loader ] Can't find process" << std::endl;
            exit(3);
        }

        std::cout << "[ loader ] Got process: Pid " << PID << std::endl;
    }

    if (!FileExists((LPCTSTR)dllPath))
    {
        std::cerr << "[ loader ] DLL not found" << std::endl;
        exit(4);
    }

    if (checkAlreadyInjected(PID, dllname))
    {
        std::cerr << "[ loader ] " << dllname << " is already injected into PID "
            << PID << " -- skipping re-injection" << std::endl;
        return 5;
    }

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
