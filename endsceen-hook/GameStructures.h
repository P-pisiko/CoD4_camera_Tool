#pragma once
// Minimal game-memory types needed to walk a DObj's bone list and call
// CG_DObjGetWorldBoneMatrix.

namespace Game
{
    struct centity_s;  // opaque -- only ever passed as a pointer

    // XModel only needs the prefix of fields this DLL reads
    struct XModel
    {
        const char* name;
        char numBones;
        char numRootBones;
        char numsurfs;
        char lodRampType;
        unsigned __int16* boneNames;
    };

    // DObj_s stride live cross-validated == 100 bytes (matches the game's
    // own `imul esi, esi, 0x64` indexing into objBuf).
    struct DSkelPartBits { int anim[4]; int control[4]; int skel[4]; };
    struct DSkel { DSkelPartBits partBits; int timeStamp; void* mat; };
    struct DObj_s
    {
        void* tree;  // XAnimTree_s*
        unsigned __int16 duplicateParts;
        unsigned __int16 entnum;
        char duplicatePartsSize;
        char numModels;
        char numBones;
        unsigned int ignoreCollision;
        volatile int locked;
        DSkel skel;
        float radius;
        unsigned int hidePartBits[4];
        XModel** models;
    };
}
