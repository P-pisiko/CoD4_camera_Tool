#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>

// Minimal AOB (array-of-bytes) signature scanner + lazily-resolved game
// addresses, almost a copy paste iwxmvm's Signatures.hpp 

namespace Signatures
{

    uintptr_t FindPattern(const char* pattern, int offset, uintptr_t moduleBase, size_t moduleSize);

    
    uintptr_t FindPatternAndReadDword(const char* pattern, int offset, uintptr_t moduleBase, size_t moduleSize);

    // Lazily resolves and caches the handful of addresses the viewmodel
    // bone exporter needs against the main module
    struct GameAddresses
    {
        // CG_DObjGetWorldBoneMatrix(centity_s* entity/*eax*/, int boneIndex/*ecx*/, float* matrix/*esi*/, DObj_s* dobj, float* origin) -> bool
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
        // (*(DWORD*)addr) on every call since the table can be reallocated between level loads.
        uintptr_t slConstTableAddr();

        bool AllResolved() const { return m_core; }

    private:
        bool Resolve();
        bool m_attempted = false;
        bool m_core = false;  // THE addresses actually required to capture
        uintptr_t m_CG_DObjGetWorldBoneMatrix = 0;
        uintptr_t m_viewmodelEntity = 0;
        uintptr_t m_poseArray = 0;
        uintptr_t m_iw3dUnknown1 = 0;
        uintptr_t m_iw3dUnknown2 = 0;
        uintptr_t m_slConstTableAddr = 0;
    };

    GameAddresses& Get();
}
