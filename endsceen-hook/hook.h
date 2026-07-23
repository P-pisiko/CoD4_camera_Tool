#pragma once
#ifndef HOOK_H
#define HOOK_H
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

// Thin wrapper around MinHook (https://github.com/TsudaKageyu/minhook), the same
// hooking library iwxmvm uses (IW3MVM uses Microsoft Detours for the same purpose).
// Replaces the previous hand-rolled inline-patch + trampoline implementation.
namespace Hook
{
    // Initializes MinHook. Must be called exactly once before any Create() call.
    bool Init();

    // Creates and enables an inline hook at `target`, redirecting to `detour`.
    // On success, *original receives a callable trampoline to the original function.
    bool Create(void* target, void* detour, void** original, const char* descr);

    // Disables and removes a single hook previously created via Create().
    bool Remove(void* target, const char* descr);

    // Disables every hook and uninitializes MinHook. Call once during shutdown.
    void Shutdown();
}

#endif
