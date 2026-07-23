#pragma once
#ifndef GAME_ADDRESSES_H
#define GAME_ADDRESSES_H

#include <cstdint>
#include <string>

// Patterns are copied verbatim from iwxmvm's iw3/src/Signatures.hpp (proven against
// real CoD4 builds) and cross-checked where possible against IW3MVM's independently-derived
// patterns for the same functions (see iw3mvm-iw3map-export-findings.md, section 2).
// Deliberately scoped to bones only -- the existing camera capture (config/Offsets.cs, C#
// side) is untouched by this migration; see cod4-dm1-bone-export-progress.md for why.
namespace CoD4Hook::GameAddresses
{
    // Scans for every address below. Individual signature failures are logged (via
    // OutputDebugStringA) but do NOT all gate IsAvailable() -- only the addresses the
    // viewmodel capture path actually reads (CG_DObjGetWorldBoneMatrix, objBuf,
    // clientGlobals, cg_entities) do. `clientObjMap` is resolved and kept for the future
    // third-person pass but isn't on the viewmodel path today, so a stale pattern for it
    // must not disable bone capture entirely. `SL_GetStringOfSize` (readable bone names)
    // is optional too -- see HasStringTable().
    bool Init();

    // True once Init() has resolved every address the viewmodel bone capture path needs.
    bool IsAvailable();

    // True if SL_GetStringOfSize resolved. Bone capture still works without it (falls back
    // to "bone_<index>" names) -- callers must check this before calling
    // GameFunctions::SL_GetStringOfSize, which would otherwise call through a null address.
    bool HasStringTable();

    // Formats a plain-text report of every signature Init() attempted (pass/fail) plus
    // the two availability flags above -- written to bone_capture/diagnostics.txt by
    // BoneRecorder so "why is bone export empty" is answerable from a text file.
    std::string DebugText();

    // CG_DObjGetWorldBoneMatrix(centity_s* entity, int boneIndex, float* matrix,
    //                           DObj_s* dobj, float* origin) -> bool
    // eax=entity, ecx=boneIndex, esi=matrix, stack-pushed dobj/origin (cdecl-ish cleanup).
    std::uintptr_t CG_DObjGetWorldBoneMatrix();

    // uint16_t SL_GetStringOfSize(const char* string, int entityType, int stringLength) -> cdecl
    std::uintptr_t SL_GetStringOfSize();

    // uint16_t* clientObjMap -- entity id -> dobj index
    std::uintptr_t ClientObjMap();

    // DObj_s* objBuf -- the dobj pool clientObjMap/iDObjWeapon index into
    std::uintptr_t ObjBuf();

    // cg_s* clientGlobals -- local client's cgame state (clientNum, bgs.clientinfo[], ...)
    std::uintptr_t ClientGlobals();

    // centity_s* cg_entities -- entity array, indexed by entity id (== clientNum for players)
    std::uintptr_t CgEntities();
}

#endif
