#pragma once
#ifndef HOOK_H
#define HOOK_H
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

// Thin wrapper around MinHook

// MinHook suspends every other thread in the process while it patches or
// restores a target function's bytes, which removes the "torn instruction"
// hazard the old hand-rolled trampoline had when hot-attaching/detaching
// against a running game. It does NOT wait for a call that had already
// entered the detour before Hook_Detach() ran -- see g_hookActiveCalls in
// d3d.h/d3d.cpp for how that remaining race is closed.
bool Hook_Init();
void Hook_Shutdown();

// Creates and enables a hook. `original` receives the trampoline used to
// call onward to the real function.
bool Hook_Attach(void* target, void* detour, void** original);

// Disables and removes a previously attached hook.
bool Hook_Detach(void* target);

#endif
