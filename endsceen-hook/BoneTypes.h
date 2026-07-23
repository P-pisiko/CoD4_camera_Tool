#pragma once
#ifndef BONE_TYPES_H
#define BONE_TYPES_H

#include <string>
#include <vector>

// Shared between BoneCapture (produces these, in-process) and BoneLog (serializes them to
// disk). Deliberately object/bone-list shaped rather than "camera + fixed viewmodel fields"
// so a later third-person pass can add more entries to `objects` per frame (one per player
// body/head, one per dynamic entity, ...) without changing this shape or the file format
namespace CoD4Hook
{
    struct BoneSample
    {
        std::string name;       // e.g. "tag_weapon", or "bone_12" when name resolution misses
        bool valid = false;     // CG_DObjGetWorldBoneMatrix's own success flag, not a
                                 // zero-vector heuristic (IW3D's format used the latter and it misreads a bone that legitimately sits at world origin)
        float position[3] = {};
        float rotation[9] = {}; // world-space 3x3 rotation matrix, row-major
    };

    struct BoneObjectSample
    {
        std::string name;                  // e.g. "viewmodel"
        std::vector<BoneSample> bones;
    };
}

#endif
