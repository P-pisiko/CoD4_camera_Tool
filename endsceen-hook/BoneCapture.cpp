#include "BoneCapture.h"
#include "GameAddresses.h"
#include "GameFunctions.h"
#include "GameStructures.h"
#include <cstdio>
#include <unordered_map>

namespace CoD4Hook::BoneCapture
{
    namespace
    {
        // Best-effort name resolution only -- a bone that isn't in this list still gets
        // captured (world position + rotation), just under a synthetic "bone_<index>" name
        // instead of a readable one. Getting a name wrong/missing here can never corrupt a
        // transform, so this list can be extended freely as more names are confirmed.
        //
        // Sources: iwxmvm's third-person GetSupportedBoneNames() whitelist (tags shared with
        // the viewmodel attach point) plus CoD4's publicly documented viewmodel hand rig
        // naming convention (COD Modding & Mapping Wiki, "Call of Duty 4: ViewModels" --
        // exported names have the rig:/rig:DefMesh:/VM_Gun: prefixes stripped).
        const std::vector<std::string>& CandidateBoneNames()
        {
            static const std::vector<std::string> names = [] {
                std::vector<std::string> list = {
                    "tag_weapon", "tag_flash", "tag_clip", "tag_brass", "tag_origin",
                    "tag_view", "tag_cambone", "tag_torso",
                    "j_head", "j_mainroot",
                    "j_wrist_le", "j_wrist_ri",
                    "j_shoulder_le", "j_shoulder_ri",
                    "j_ankle_le", "j_ankle_ri",
                    "controlWrist", "controlWrist1",
                };

                // Finger FK chains: {Index,Mid,Pinky,Ring,Thumb}_FK_{LE,RI}_{0,1,2,3}
                static const char* fingers[] = { "Index", "Mid", "Pinky", "Ring", "Thumb" };
                static const char* sides[] = { "LE", "RI" };
                for (const char* finger : fingers)
                {
                    for (const char* side : sides)
                    {
                        for (int segment = 0; segment <= 3; ++segment)
                        {
                            char buf[64];
                            sprintf_s(buf, "%s_FK_%s_%d", finger, side, segment);
                            list.emplace_back(buf);
                        }
                    }
                }

                return list;
                }();

            return names;
        }

        // string-table id -> readable name, built once by forward-matching every candidate
        // name through SL_GetStringOfSize (the same interning function the game itself uses
        // to build boneNames[] in the first place -- so a match here is exact, not fuzzy).
        const std::unordered_map<uint16_t, std::string>& BoneNameLookup()
        {
            static const std::unordered_map<uint16_t, std::string> lookup = [] {
                std::unordered_map<uint16_t, std::string> map;

                // SL_GetStringOfSize is optional (see GameAddresses::HasStringTable) --
                // without it every bone just falls back to "bone_<index>" in
                // ResolveBoneName below rather than calling through a null address.
                if (!GameAddresses::HasStringTable())
                    return map;

                for (const std::string& name : CandidateBoneNames())
                {
                    const uint16_t id = GameFunctions::SL_GetStringOfSize(
                        name.c_str(), 1, static_cast<int>(name.size()) + 1);
                    if (id != 0)
                        map.emplace(id, name);
                }
                return map;
                }();

            return lookup;
        }

        std::string ResolveBoneName(uint16_t stringId, int boneIndex)
        {
            const auto& lookup = BoneNameLookup();
            const auto it = lookup.find(stringId);
            if (it != lookup.end())
                return it->second;

            char buf[16];
            sprintf_s(buf, "bone_%d", boneIndex);
            return buf;
        }

        // clientNum bounds the bgs.clientinfo[64] / cg_entities[] indexing below -- reject
        // anything outside the game's own maxclients range instead of trusting it blindly.
        constexpr int kMaxClients = 64;

        // objBuf is a large but finite pool; a wildly out-of-range dobj index means
        // iDObjWeapon wasn't actually valid this frame (e.g. no weapon equipped yet).
        constexpr int kMaxDObjIndex = 4096;
    }

    bool CaptureViewmodel(std::vector<BoneObjectSample>& outObjects)
    {
        if (!GameAddresses::IsAvailable())
            return false;

        auto* cg = reinterpret_cast<Game::cg_s*>(GameAddresses::ClientGlobals());
        if (!cg)
            return false;

        const int clientNum = cg->clientNum;
        if (clientNum < 0 || clientNum >= kMaxClients)
            return false;

        const Game::clientInfo_t& clientInfo = cg->bgs.clientinfo[clientNum];
        const int dobjIndex = clientInfo.iDObjWeapon;
        if (dobjIndex < 0 || dobjIndex >= kMaxDObjIndex)
            return false;

        auto* objBufBase = reinterpret_cast<Game::DObj_s*>(GameAddresses::ObjBuf());
        Game::DObj_s* dobj = &objBufBase[dobjIndex];
        if (!dobj->models || dobj->numModels <= 0)
            return false;

        auto* entities = reinterpret_cast<Game::centity_s*>(GameAddresses::CgEntities());
        Game::centity_s* entity = &entities[clientNum];

        // Note for the third-person pass (see cod4-dm1-bone-export-progress.md): third-person
        // bodies can be lazily-skinned when off-screen, which iwxmvm works around by forcing
        // dobj->skel.timeStamp to the client's current skelTimeStamp before the call and
        // restoring it after. The viewmodel is rendered every frame it's active, so its
        // skeleton is already current -- this override isn't needed here, but will be once
        // third-person bodies are added.
        BoneObjectSample viewmodel;
        viewmodel.name = "viewmodel";

        int totalBones = 0;
        for (int m = 0; m < dobj->numModels; ++m)
        {
            Game::XModel* model = dobj->models[m];
            if (!model || model->numBones <= 0 || !model->boneNames)
            {
                continue;
            }

            for (int b = 0; b < static_cast<int>(static_cast<unsigned char>(model->numBones)); ++b)
            {
                const int boneIndex = totalBones + b;

                float rotationMatrix[9] = {};
                float origin[3] = {};
                const bool valid = GameFunctions::CG_DObjGetWorldBoneMatrix(
                    entity, boneIndex, rotationMatrix, dobj, origin);

                BoneSample sample;
                sample.name = ResolveBoneName(model->boneNames[b], boneIndex);
                sample.valid = valid;
                sample.position[0] = origin[0];
                sample.position[1] = origin[1];
                sample.position[2] = origin[2];
                for (int i = 0; i < 9; ++i)
                    sample.rotation[i] = rotationMatrix[i];

                viewmodel.bones.push_back(std::move(sample));
            }

            totalBones += static_cast<unsigned char>(model->numBones);
        }

        if (viewmodel.bones.empty())
            return false;

        outObjects.push_back(std::move(viewmodel));
        return true;
    }
}
