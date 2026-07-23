#include "GameAddresses.h"
#include "Signatures.h"
#include <Windows.h>
#include <string>
#include <vector>

namespace CoD4Hook::GameAddresses
{
    namespace
    {
        struct Addresses
        {
            std::uintptr_t cgDObjGetWorldBoneMatrix = 0;
            std::uintptr_t slGetStringOfSize = 0;
            std::uintptr_t clientObjMap = 0;
            std::uintptr_t objBuf = 0;
            std::uintptr_t clientGlobals = 0;
            std::uintptr_t cgEntities = 0;
        };

        Addresses g_addresses;
        bool g_available = false;
        bool g_hasStringTable = false;
        std::vector<std::pair<std::string, bool>> g_diagnostics;

        bool Resolve(const Signatures::Pattern& pattern, const char* name, std::uintptr_t& out)
        {
            out = Signatures::Scan(pattern);
            const bool ok = out != 0;
            g_diagnostics.emplace_back(name, ok);
            if (!ok)
            {
                OutputDebugStringA(("[GameAddresses] Failed to resolve " + std::string(name) + "\n").c_str());
            }
            return ok;
        }
    }

    bool Init()
    {
        using Signatures::AddressType;
        g_diagnostics.clear();

        // Every signature is attempted regardless of earlier failures (so DebugView shows
        // the full picture in one pass, not just the first failure) -- but only a subset
        // gates IsAvailable(); see the header comment for why.

        // Sig("5C 24 38 55 8B 6C 24 40 57 8B F9", GAType::Code, -5) > CG_DObjGetWorldBoneMatrix;
        const bool haveBoneMatrix = Resolve({ "5C 24 38 55 8B 6C 24 40 57 8B F9", AddressType::Code, -5 },
            "CG_DObjGetWorldBoneMatrix", g_addresses.cgDObjGetWorldBoneMatrix);

        // Sig("5C 24 20 55 56 8B 74 24 20", GAType::Code, -5) > SL_GetStringOfSize;
        const bool haveStringOfSize = Resolve({ "5C 24 20 55 56 8B 74 24 20", AddressType::Code, -5 },
            "SL_GetStringOfSize", g_addresses.slGetStringOfSize);

        // Sig("03 44 24 04 0F B7 04 45", GAType::Data, 8, Lambda::DereferenceAddress) > clientObjMap;
        // Not on the viewmodel capture path (reserved for the future third-person pass) --
        // resolved and logged, but deliberately excluded from the IsAvailable() gate below.
        Resolve({ "03 44 24 04 0F B7 04 45", AddressType::Data, 8, true },
            "clientObjMap", g_addresses.clientObjMap);

        // Sig("0F BF F0 6B F6 64 81 C6", GAType::Data, 8, Lambda::DereferenceAddress) > objBuf;
        const bool haveObjBuf = Resolve({ "0F BF F0 6B F6 64 81 C6", AddressType::Data, 8, true },
            "objBuf", g_addresses.objBuf);

        // Sig("BA ?? ?? ?? ?? E8 ?? ?? ?? ?? D9 03", GAType::Data, 1, Lambda::DereferenceAddress) > clientGlobals;
        const bool haveClientGlobals = Resolve({ "BA ?? ?? ?? ?? E8 ?? ?? ?? ?? D9 03", AddressType::Data, 1, true },
            "clientGlobals", g_addresses.clientGlobals);

        // Sig("69 C9 00 70 07 00 83 C4 0C 68 00 70 07 00 81", GAType::Data, 16, Lambda::DereferenceAddress) > cg_entities;
        const bool haveCgEntities = Resolve({ "69 C9 00 70 07 00 83 C4 0C 68 00 70 07 00 81", AddressType::Data, 16, true },
            "cg_entities", g_addresses.cgEntities);

        g_available = haveBoneMatrix && haveObjBuf && haveClientGlobals && haveCgEntities;
        g_hasStringTable = haveStringOfSize;

        if (!g_available)
        {
            OutputDebugStringA("[GameAddresses] Bone export unavailable this session: "
                "one or more required signatures failed to resolve (see above).\n");
        }
        else if (!g_hasStringTable)
        {
            OutputDebugStringA("[GameAddresses] SL_GetStringOfSize unresolved -- bone "
                "export still works, bone names will fall back to \"bone_<index>\".\n");
        }

        return g_available;
    }

    bool IsAvailable()
    {
        return g_available;
    }

    bool HasStringTable()
    {
        return g_hasStringTable;
    }

    std::string DebugText()
    {
        std::string text = "CoD4-dm1 bone export diagnostics\r\n";
        text += g_available ? "Bone capture: AVAILABLE\r\n" : "Bone capture: UNAVAILABLE\r\n";
        text += g_hasStringTable ? "Bone name resolution: AVAILABLE\r\n"
                                  : "Bone name resolution: UNAVAILABLE (bones will be named bone_<index>)\r\n";
        text += "\r\nSignature scan results:\r\n";
        for (const auto& [name, ok] : g_diagnostics)
        {
            text += "  ";
            text += (ok ? "[OK]   " : "[FAIL] ");
            text += name;
            text += "\r\n";
        }
        return text;
    }


    std::uintptr_t CG_DObjGetWorldBoneMatrix() { return g_addresses.cgDObjGetWorldBoneMatrix; }
    std::uintptr_t SL_GetStringOfSize() { return g_addresses.slGetStringOfSize; }
    std::uintptr_t ClientObjMap() { return g_addresses.clientObjMap; }
    std::uintptr_t ObjBuf() { return g_addresses.objBuf; }
    std::uintptr_t ClientGlobals() { return g_addresses.clientGlobals; }
    std::uintptr_t CgEntities() { return g_addresses.cgEntities; }
}
