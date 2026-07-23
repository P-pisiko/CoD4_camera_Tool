#pragma once
#ifndef GAME_FUNCTIONS_H
#define GAME_FUNCTIONS_H

#include "GameStructures.h"
#include <cstdint>

// Thin __asm call shims for the two game functions bone export needs, matching iwxmvm's
// iw3/src/Functions.cpp exactly
namespace CoD4Hook::GameFunctions
{
    bool CG_DObjGetWorldBoneMatrix(Game::centity_s* entity, int boneIndex, float* matrix,
        Game::DObj_s* dobj, float* origin);

    uint16_t SL_GetStringOfSize(const char* string, int entityType, int stringLength);
}

#endif
