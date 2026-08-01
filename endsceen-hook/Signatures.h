#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>

// Minimal AOB (array-of-bytes) signature scanner + lazily-resolved game
// addresses, mirroring iwxmvm's Signatures.hpp / FindPattern approach so we
// never hardcode absolute addresses that could shift between game builds.
//
// The viewmodelEntity/poseArray/iw3dUnknown1/iw3dUnknown2 addresses and the
// arithmetic that combines them are ported from IW3MVM's IW3D.cpp
// (Patch_IW3D / AimTarget_GetTagPos_Hook) -- the only known-working
// reference for resolving the *viewmodel* DObj specifically (as opposed to
// a body DObj). Semantics of exactly why that arithmetic works are not
// fully documented even in the source it's ported from; it's reproduced
// verbatim rather than reverse-derived.
namespace Signatures
{
    // Parses a pattern like "5C 24 38 ?? 8B 6C" (?? = wildcard byte) and
    // scans [moduleBase, moduleBase+moduleSize) for the first match.
    // Returns the match address + `offset`, or 0 if not found.
    uintptr_t FindPattern(const char* pattern, int offset, uintptr_t moduleBase, size_t moduleSize);

    // Same, but reads the found address (already offset-adjusted) as a
    // little-endian DWORD literal embedded in the instruction stream --
    // e.g. the absolute address operand of `mov edx, imm32` -- rather than
    // returning the address itself.
    uintptr_t FindPatternAndReadDword(const char* pattern, int offset, uintptr_t moduleBase, size_t moduleSize);

    // Lazily resolves and caches the handful of addresses the viewmodel
    // bone exporter needs against the main module (iw3mp.exe). Only ever
    // called from the render thread (no locking).
    struct GameAddresses
    {
        // CG_DObjGetWorldBoneMatrix(centity_s* entity/*eax*/, int boneIndex/*ecx*/,
        //                           float* matrix/*esi*/, DObj_s* dobj, float* origin) -> bool
        uintptr_t CG_DObjGetWorldBoneMatrix();

        // Direct centity_s* dedicated to viewmodel rendering (IW3MVM: `viewmodelEntity`).
        uintptr_t viewmodelEntity();

        // Base of a per-"pose slot" array; index 17*v0 holds the viewmodel
        // DObj_s* for the currently active pose slot (IW3MVM: `poseArray_Address`).
        uintptr_t poseArray();

        // Address of a variable selecting which pose slot is active; must be
        // re-read every call (IW3MVM: `iw3dUnknown1_Address`).
        uintptr_t iw3dUnknown1();

        // Address of a flag variable gating how iw3dUnknown1 is interpreted
        // (IW3MVM: `iw3dUnknown2_Address`).
        uintptr_t iw3dUnknown2();

        // Fixed memory location holding a pointer to the string-table base,
        // used to resolve boneNames[] handles to text. Must be re-read
        // (*(DWORD*)addr) on every call since the table can be reallocated
        // between level loads. Returns 0 if unresolved.
        uintptr_t slConstTableAddr();

        bool AllResolved() const { return m_core; }

    private:
        bool Resolve();
        bool m_attempted = false;
        bool m_core = false;  // the addresses actually required to capture
        uintptr_t m_CG_DObjGetWorldBoneMatrix = 0;
        uintptr_t m_viewmodelEntity = 0;
        uintptr_t m_poseArray = 0;
        uintptr_t m_iw3dUnknown1 = 0;
        uintptr_t m_iw3dUnknown2 = 0;
        uintptr_t m_slConstTableAddr = 0;
    };

    GameAddresses& Get();
}
