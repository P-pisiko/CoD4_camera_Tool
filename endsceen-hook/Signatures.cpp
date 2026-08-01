#include "Signatures.h"
#include <vector>
#include <cstring>

namespace Signatures
{
    namespace
    {
        struct PatternByte
        {
            uint8_t value;
            bool wildcard;
        };

        std::vector<PatternByte> ParsePattern(const char* pattern)
        {
            std::vector<PatternByte> bytes;
            const char* p = pattern;
            while (*p)
            {
                while (*p == ' ') ++p;
                if (!*p) break;

                if (p[0] == '?' )
                {
                    bytes.push_back({ 0, true });
                    ++p;
                    if (*p == '?') ++p;  // tolerate "??" as one token
                }
                else
                {
                    bytes.push_back({ static_cast<uint8_t>(strtoul(p, nullptr, 16)), false });
                    while (*p && *p != ' ') ++p;
                }
            }
            return bytes;
        }

        uintptr_t ScanModule(uintptr_t moduleBase, size_t moduleSize, const std::vector<PatternByte>& needle)
        {
            if (moduleBase == 0 || moduleSize == 0 || needle.empty())
                return 0;

            const uint8_t* base = reinterpret_cast<const uint8_t*>(moduleBase);
            const size_t needleLen = needle.size();
            if (needleLen > moduleSize)
                return 0;

            for (size_t i = 0; i <= moduleSize - needleLen; ++i)
            {
                bool match = true;
                for (size_t j = 0; j < needleLen; ++j)
                {
                    if (!needle[j].wildcard && base[i + j] != needle[j].value)
                    {
                        match = false;
                        break;
                    }
                }
                if (match)
                    return moduleBase + i;
            }
            return 0;
        }
    }

    uintptr_t FindPattern(const char* pattern, int offset, uintptr_t moduleBase, size_t moduleSize)
    {
        const auto needle = ParsePattern(pattern);
        const uintptr_t found = ScanModule(moduleBase, moduleSize, needle);
        return found ? found + offset : 0;
    }

    uintptr_t FindPatternAndReadDword(const char* pattern, int offset, uintptr_t moduleBase, size_t moduleSize)
    {
        const uintptr_t at = FindPattern(pattern, offset, moduleBase, moduleSize);
        if (!at)
            return 0;

        __try
        {
            return *reinterpret_cast<uint32_t*>(at);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return 0;
        }
    }

    namespace
    {
        bool GetMainModuleRange(uintptr_t& base, size_t& size)
        {
            HMODULE hMod = GetModuleHandleA(nullptr);
            if (!hMod)
                return false;

            base = reinterpret_cast<uintptr_t>(hMod);
            auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(hMod);
            if (dos->e_magic != IMAGE_DOS_SIGNATURE)
                return false;

            auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<uint8_t*>(hMod) + dos->e_lfanew);
            if (nt->Signature != IMAGE_NT_SIGNATURE)
                return false;

            size = nt->OptionalHeader.SizeOfImage;
            return true;
        }
    }

    bool GameAddresses::Resolve()
    {
        m_attempted = true;

        uintptr_t moduleBase = 0;
        size_t moduleSize = 0;
        if (!GetMainModuleRange(moduleBase, moduleSize))
            return false;

        // Live-verified against this CoD4X 1.7 build (base 0x400000):
        //   CG_DObjGetWorldBoneMatrix -> 0x433f00
        //   slConst (SL string table) -> 0x14e8a04
        m_CG_DObjGetWorldBoneMatrix = FindPattern("5C 24 38 55 8B 6C 24 40 57 8B F9", -5, moduleBase, moduleSize);
        m_slConstTableAddr = FindPatternAndReadDword("8B 15 ?? ?? ?? ?? 8D 0C 40 84 5C 8A 02", 2, moduleBase, moduleSize);

        // Ported from IW3MVM's IW3D.cpp (Patch_IW3D); not yet independently
        // live-verified on CoD4X beyond matching the pattern.
        m_viewmodelEntity = FindPatternAndReadDword("68 ?? ?? ?? ?? BF ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 C4 0C 85 C0", 1, moduleBase, moduleSize);
        m_poseArray = FindPatternAndReadDword("81 C2 ?? ?? ?? ?? 57 52", 2, moduleBase, moduleSize);
        m_iw3dUnknown1 = FindPatternAndReadDword("8B 1D ?? ?? ?? ?? 75 06", 2, moduleBase, moduleSize);
        m_iw3dUnknown2 = FindPatternAndReadDword("F6 05 ?? ?? ?? ?? ?? 8B 1D", 2, moduleBase, moduleSize);

        m_core = (m_CG_DObjGetWorldBoneMatrix != 0) && (m_viewmodelEntity != 0) && (m_poseArray != 0) &&
                 (m_iw3dUnknown1 != 0) && (m_iw3dUnknown2 != 0);
        return m_core;
    }

    uintptr_t GameAddresses::CG_DObjGetWorldBoneMatrix()
    {
        if (!m_attempted) Resolve();
        return m_CG_DObjGetWorldBoneMatrix;
    }

    uintptr_t GameAddresses::viewmodelEntity()
    {
        if (!m_attempted) Resolve();
        return m_viewmodelEntity;
    }

    uintptr_t GameAddresses::poseArray()
    {
        if (!m_attempted) Resolve();
        return m_poseArray;
    }

    uintptr_t GameAddresses::iw3dUnknown1()
    {
        if (!m_attempted) Resolve();
        return m_iw3dUnknown1;
    }

    uintptr_t GameAddresses::iw3dUnknown2()
    {
        if (!m_attempted) Resolve();
        return m_iw3dUnknown2;
    }

    uintptr_t GameAddresses::slConstTableAddr()
    {
        if (!m_attempted) Resolve();
        return m_slConstTableAddr;
    }

    GameAddresses& Get()
    {
        static GameAddresses instance;
        return instance;
    }
}
