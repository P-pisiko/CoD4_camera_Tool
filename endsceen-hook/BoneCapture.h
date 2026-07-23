#pragma once
#ifndef BONE_CAPTURE_H
#define BONE_CAPTURE_H

#include "BoneTypes.h"
#include <vector>

// Pulls the local player's viewmodel (first-person hands + weapon) skeleton for the current
// frame, following iwxmvm's proven access path rather than IW3MVM's AimTarget_GetTagPos hook
// (see viewmodel-bone-export.md section 3 / iw3mvm-iw3map-export-findings.md section 4 for why):
//
//   cg_s::clientNum -> bgs.clientinfo[clientNum].iDObjWeapon   (viewmodel dobj index)
//   objBuf[iDObjWeapon]                                        (DObj_s* for the viewmodel)
//   dobj->models[m]->boneNames[b] for every model/bone         (full skeleton, not a whitelist)
//   CG_DObjGetWorldBoneMatrix(entity, boneIndex, ...)          (world-space transform + bool)
//
// Deliberately entity/object-shaped in its output (BoneObjectSample) so a third-person pass
// later can call the same per-bone extraction against a different DObj_s (a player body/head)
// and append more objects to the same frame -- see cod4-dm1-bone-export-progress.md.
namespace CoD4Hook::BoneCapture
{
    // Appends one BoneObjectSample (name "viewmodel") to `outObjects` with every bone the
    // local player's weapon DObj currently reports, or leaves `outObjects` untouched and
    // returns false if the viewmodel dobj can't be resolved this frame (no weapon out,
    // between loadouts, signatures unavailable, etc.) -- callers should treat that as "no
    // viewmodel this frame", not an error.
    bool CaptureViewmodel(std::vector<BoneObjectSample>& outObjects);
}

#endif
