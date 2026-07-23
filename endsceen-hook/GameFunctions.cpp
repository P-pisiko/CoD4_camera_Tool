#include "GameFunctions.h"
#include "GameAddresses.h"
#include <Windows.h>

namespace CoD4Hook::GameFunctions
{
    // Register-based calling convention, identical to iwxmvm's iw3/src/Functions.cpp
    // CG_DObjGetWorldBoneMatrix shim
    bool CG_DObjGetWorldBoneMatrix(Game::centity_s* entity, int boneIndex, float* matrix,
        Game::DObj_s* dobj, float* origin)
    {
#if defined(_M_IX86)
        const std::uintptr_t address = GameAddresses::CG_DObjGetWorldBoneMatrix();
        __asm
        {
            mov eax, entity
            mov ecx, boneIndex
            mov esi, matrix
            push origin
            push dobj
            call address
            add esp, 8
        }
#else
        (void)entity; (void)boneIndex; (void)matrix; (void)dobj; (void)origin;
        return false;
#endif
    }

    uint16_t SL_GetStringOfSize(const char* string, int entityType, int stringLength)
    {
        using SL_GetStringOfSize_t = uint16_t(__cdecl*)(const char*, int, int);
        const auto fn = reinterpret_cast<SL_GetStringOfSize_t>(GameAddresses::SL_GetStringOfSize());
        return fn(string, entityType, stringLength);
    }
}
