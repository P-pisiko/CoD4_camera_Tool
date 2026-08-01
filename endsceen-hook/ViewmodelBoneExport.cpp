#include "ViewmodelBoneExport.h"
#include "GameStructures.h"
#include "Signatures.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <vector>
#include <string>

// NOTE on SEH usage in this file: MSVC forbids `__try` in any function scope
// that also has a local object requiring unwind cleanup (std::string,
// std::vector, ...) -- C2712. Every `__try` below therefore lives in a tiny
// leaf function touching only POD locals/pointers; the functions that own
// std::string/std::vector call those leaves instead of wrapping themselves.

namespace ViewmodelBoneExport
{
    namespace
    {

        // eax=entity, ecx=boneIndex, esi=matrix, stack: push origin; push dobj
        // (dobj ends up as the top-of-stack arg the callee reads first)
        // iwxmvm's Functions::CG_DObjGetWorldBoneMatrix
        bool CG_DObjGetWorldBoneMatrix(Game::centity_s* entity, int boneIndex, float* matrix,
                                            Game::DObj_s* dobj, float* origin)
        {

            uintptr_t address = Signatures::Get().CG_DObjGetWorldBoneMatrix();
            if (!address)
                return false;

            __try
            {
                bool result = false;
                __asm
                {
                    mov eax, entity
                    mov ecx, boneIndex
                    mov esi, matrix
                    push origin
                    push dobj
                    call address
                    add esp, 8
                    mov result, al
                }
                return result;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return false;
            }
        }

        bool ReadDword(uintptr_t addr, uint32_t& out)
        {
            __try
            {
                out = *reinterpret_cast<uint32_t*>(addr);
                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                out = 0;
                return false;
            }
        }

        // Copies a NUL-terminated (or length-bounded) string out of
        // game memory into a fixed local buffer.
        bool StrCopy(const char* src, char* dst, size_t dstCapacity, size_t& outLen)
        {
            __try
            {
                size_t len = strnlen_s(src, dstCapacity - 1);
                memcpy(dst, src, len);
                dst[len] = '\0';
                outLen = len;
                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                outLen = 0;
                return false;
            }
        }

        // Ported from IW3MVM's IW3D.cpp (AimTarget_GetTagPos_Hook /
        // Patch_IW3D): resolves the viewmodel's DObj_s* through a per-slot pose array
        // bgs.clientinfo[clientNum].iDObjWeapon field.
        struct FrameContext
        {
            Game::centity_s* entity = nullptr;
            Game::DObj_s* dobj = nullptr;
        };

        bool BuildFrameContext(FrameContext& out)
        {
            auto& a = Signatures::Get();
            const uintptr_t entityAddr = a.viewmodelEntity();
            const uintptr_t poseArrayAddr = a.poseArray();
            const uintptr_t unknown1Addr = a.iw3dUnknown1();
            const uintptr_t unknown2Addr = a.iw3dUnknown2();
            if (!entityAddr || !poseArrayAddr || !unknown1Addr || !unknown2Addr)
                return false;

            uint32_t flag = 0;
            if (!ReadDword(unknown2Addr, flag))
                return false;

            uint32_t v0 = 0;
            const bool gotV0 = (flag & 2) ? ReadDword(unknown1Addr, v0)
                                           : ReadDword(unknown1Addr + 8, v0);
            if (!gotV0)
                return false;

            uint32_t dobjPtr = 0;
            if (!ReadDword(poseArrayAddr + 4 * (17 * v0), dobjPtr) || !dobjPtr)
                return false;

            out.entity = reinterpret_cast<Game::centity_s*>(entityAddr);
            out.dobj = reinterpret_cast<Game::DObj_s*>(dobjPtr);
            return true;
        }

        bool ReadDObjModels(Game::DObj_s* dobj, Game::XModel**& outModels, int& outNumModels)
        {
            __try
            {
                outModels = dobj->models;
                outNumModels = dobj->numModels;
                return outModels != nullptr && outNumModels > 0;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                outModels = nullptr;
                outNumModels = 0;
                return false;
            }
        }

        bool ReadModelSlot(Game::XModel** models, int index, Game::XModel*& outModel)
        {
            __try
            {
                outModel = models[index];
                return outModel != nullptr;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                outModel = nullptr;
                return false;
            }
        }

        bool ReadModelBoneInfo(Game::XModel* model, int& outNumBones, uint16_t*& outBoneNames)
        {
            __try
            {
                outNumBones = model->numBones;
                outBoneNames = model->boneNames;
                return outBoneNames != nullptr && outNumBones > 0;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                outNumBones = 0;
                outBoneNames = nullptr;
                return false;
            }
        }

        bool ReadBoneNameHandle(uint16_t* boneNames, int index, uint16_t& outHandle)
        {
            __try
            {
                outHandle = boneNames[index];
                return true;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                outHandle = 0;
                return false;
            }
        }

        // Resolves a boneNames[] string-table handle to text.
        std::string ResolveBoneName(uint16_t handle)
        {
            if (handle == 0)
                return {};

            const uintptr_t tableAddr = Signatures::Get().slConstTableAddr();
            if (!tableAddr)
                return {};

            uint32_t tableBase = 0;
            if (!ReadDword(tableAddr, tableBase) || !tableBase)
                return {};

            const char* str = reinterpret_cast<const char*>(tableBase + 0xC * handle + 0x4);

            char buffer[128];
            size_t len = 0;
            if (!StrCopy(str, buffer, sizeof(buffer), len))
                return {};

            return std::string(buffer, len);
        }

        struct BoneRef
        {
            Game::XModel* model;
            int localIndex;   // index within model->boneNames[]
            uint16_t nameHandle;
        };

        // Walks every model attached to `dobj` and returns the flattened bone list
        std::vector<BoneRef> EnumerateBones(Game::DObj_s* dobj)
        {
            std::vector<BoneRef> bones;
            if (!dobj)
                return bones;

            Game::XModel** models = nullptr;
            int numModels = 0;
            if (!ReadDObjModels(dobj, models, numModels))
                return bones;

            for (int m = 0; m < numModels; ++m)
            {
                Game::XModel* model = nullptr;
                if (!ReadModelSlot(models, m, model))
                    continue;

                int numBones = 0;
                uint16_t* boneNames = nullptr;
                if (!ReadModelBoneInfo(model, numBones, boneNames))
                    continue;

                for (int b = 0; b < numBones; ++b)
                {
                    uint16_t handle = 0;
                    ReadBoneNameHandle(boneNames, b, handle);
                    bones.push_back({ model, b, handle });
                }
            }
            return bones;
        }

        FILE* g_captureFile = nullptr;
        Game::DObj_s* g_sessionDobj = nullptr;
        uint32_t g_boneCountInHeader = 0;

        void CloseCapture()
        {
            if (g_captureFile)
            {
                fclose(g_captureFile);
                g_captureFile = nullptr;
            }
            g_sessionDobj = nullptr;
            g_boneCountInHeader = 0;
        }

        bool EnsureBoneInfoDir()
        {
            return CreateDirectoryA("bone_info", nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
        }

        bool StartCapture(Game::DObj_s* dobj, const std::vector<BoneRef>& bones)
        {
            CloseCapture();

            if (!EnsureBoneInfoDir())
                return false;

            char path[MAX_PATH];
            sprintf_s(path, "bone_info\\capture_%llu.bin",
                      static_cast<unsigned long long>(GetTickCount64()));

            FILE* f = nullptr;
            if (fopen_s(&f, path, "wb") != 0 || !f)
                return false;

            fwrite("CD4VMBONE", 1, 9, f);
            const uint8_t version = 1;
            fwrite(&version, 1, 1, f);
            const uint32_t boneCount = static_cast<uint32_t>(bones.size());
            fwrite(&boneCount, sizeof(boneCount), 1, f);

            for (const auto& bone : bones)
            {
                std::string name = ResolveBoneName(bone.nameHandle);
                const uint16_t nameLen = static_cast<uint16_t>(name.size());
                fwrite(&nameLen, sizeof(nameLen), 1, f);
                if (nameLen)
                    fwrite(name.data(), 1, nameLen, f);
            }

            g_captureFile = f;
            g_sessionDobj = dobj;
            g_boneCountInHeader = boneCount;
            return true;
        }

        void WriteFrame(int frameIndex, Game::centity_s* entity, Game::DObj_s* dobj,
                         const std::vector<BoneRef>& bones)
        {
            if (!g_captureFile || bones.size() != g_boneCountInHeader)
                return;

            const uint32_t idx = static_cast<uint32_t>(frameIndex);
            fwrite(&idx, sizeof(idx), 1, g_captureFile);

            for (size_t i = 0; i < bones.size(); ++i)
            {
                float rotation[9] = {};
                float origin[3] = {};
                // Global bone index across the DObj's models: bones[] is
                // built in model order, so its position IS the global index CG_DObjGetWorldBoneMatrix expects.
                const uint8_t valid =
                    CG_DObjGetWorldBoneMatrix(entity, static_cast<int>(i), rotation, dobj, origin) ? 1 : 0;

                fwrite(&valid, 1, 1, g_captureFile);
                fwrite(rotation, sizeof(rotation), 1, g_captureFile);
                fwrite(origin, sizeof(origin), 1, g_captureFile);
            }
        }

        void DebugLog(const char* fmt, ...)
        {
            FILE* f = nullptr;
            if (fopen_s(&f, "endsceen_debug.log", "a") != 0 || !f)
                return;
            va_list args;
            va_start(args, fmt);
            vfprintf(f, fmt, args);
            va_end(args);
            fprintf(f, "\n");
            fclose(f);
        }
    }

    void OnFrame(bool recording, int frameIndex)
    {
        static bool loggedSigs = false;
        if (!loggedSigs)
        {
            loggedSigs = true;
            auto& a = Signatures::Get();
            DebugLog("[sigs] CG_DObjGetWorldBoneMatrix=0x%X viewmodelEntity=0x%X poseArray=0x%X "
                     "iw3dUnknown1=0x%X iw3dUnknown2=0x%X slConst=0x%X AllResolved=%d",
                     a.CG_DObjGetWorldBoneMatrix(), a.viewmodelEntity(), a.poseArray(),
                     a.iw3dUnknown1(), a.iw3dUnknown2(), a.slConstTableAddr(), (int)a.AllResolved());
        }

        static int lastReason = -1;
        auto logOnce = [&](int reason, const char* fmt, ...)
        {
            if (reason == lastReason) return;
            lastReason = reason;
            char buf[256];
            va_list args;
            va_start(args, fmt);
            vsprintf_s(buf, fmt, args);
            va_end(args);
            DebugLog("%s", buf);
        };

        if (!recording)
        {
            logOnce(0, "[state] recording=false");
            CloseCapture();
            return;
        }

        auto& addrs = Signatures::Get();
        if (!addrs.AllResolved())
        {
            logOnce(1, "[state] signatures unresolved, skipping frame");
            return;
        }

        FrameContext ctx;
        if (!BuildFrameContext(ctx))
        {
            logOnce(2, "[state] TryBuildFrameContext failed (no active viewmodel dobj)");
            return;
        }

        auto bones = EnumerateBones(ctx.dobj);
        if (bones.empty())
        {
            logOnce(3, "[state] EnumerateBones empty: dobj=0x%p", (void*)ctx.dobj);
            return;
        }

        if (!g_captureFile || ctx.dobj != g_sessionDobj)
        {
            if (!StartCapture(ctx.dobj, bones))
            {
                logOnce(4, "[state] StartCapture failed: dobj=0x%p boneCount=%zu", (void*)ctx.dobj, bones.size());
                return;
            }
            DebugLog("[state] StartCapture OK: dobj=0x%p boneCount=%zu", (void*)ctx.dobj, bones.size());
        }
        lastReason = 5;

        WriteFrame(frameIndex, ctx.entity, ctx.dobj, bones);
    }

    void Shutdown()
    {
        CloseCapture();
    }
}
