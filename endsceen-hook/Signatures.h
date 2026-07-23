#pragma once
#ifndef SIGNATURES_H
#define SIGNATURES_H
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <cstdint>

// In-process AOB (array-of-bytes) signature scanner,

namespace Signatures
{
    enum class AddressType : uint8_t
    {
        Data = 4,
        Code = 5
    };

    struct Pattern
    {
        const char* bytes;          // space-separated hex bytes / "??" wildcards
        AddressType type;
        std::intptr_t offset = 0;   // added to the address of the pattern's first byte
        bool dereference = false;   // apply one `*(uintptr_t*)address` after offset
        bool followCodeFlow = false;// resolve a jmp(0xE9)/call(0xE8) thunk chain at the result
    };

    // Scans `moduleBase`'s mapped image (defaults to the current process's main module
    // iw3mp.exe itself when called from inside the injected DLL - no ReadProcessMemory needed
    // this DLL is already loaded in the target process) for pattern
    std::uintptr_t Scan(const Pattern& pattern, HMODULE moduleBase = nullptr);
}

#endif
