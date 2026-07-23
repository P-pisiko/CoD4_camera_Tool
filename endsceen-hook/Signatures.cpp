#include "Signatures.h"
#include <vector>

namespace Signatures
{
    namespace
    {
        constexpr int Wildcard = -1;

        // "5C 24 38 55" / "BA ?? ?? ?? ??" -> [0x5C, 0x24, 0x38, 0x55] / [0xBA, -1, -1, -1, -1]
        std::vector<int> ParsePattern(const char* pattern)
        {
            std::vector<int> bytes;
            for (const char* p = pattern; *p;)
            {
                while (*p == ' ') ++p;
                if (!*p) break;

                if (p[0] == '?')
                {
                    bytes.push_back(Wildcard);
                    p += (p[1] == '?') ? 2 : 1;
                }
                else
                {
                    bytes.push_back(static_cast<int>(strtoul(p, nullptr, 16)));
                    p += 2;
                }
            }
            return bytes;
        }

        std::size_t FrontMaskCount(const std::vector<int>& bytes)
        {
            std::size_t count = 0;
            while (count < bytes.size() && bytes[count] == Wildcard) ++count;
            return count;
        }

        // Same lambda as iwxmvm's Lambdas::FollowCodeFlow: walks a chain of absolute
        // jmp (0xE9) / call (0xE8) opcodes
        std::uintptr_t FollowCodeFlow(std::uintptr_t address)
        {
            const std::uintptr_t original = address;
            auto isJumpOrCall = [](std::uintptr_t addr) {
                const uint8_t op = *reinterpret_cast<uint8_t*>(addr);
                return op == 0xE9 || op == 0xE8;
            };

            while (isJumpOrCall(address))
            {
                const int32_t rel = *reinterpret_cast<int32_t*>(address + 1);
                address += static_cast<std::uintptr_t>(rel) + 5;
            }

            return (address == original) ? 0 : address;
        }
    }

    std::uintptr_t Scan(const Pattern& pattern, HMODULE moduleBase)
    {
        HMODULE module = moduleBase ? moduleBase : GetModuleHandleA(nullptr);
        if (!module)
            return 0;

        // PE header walk instead of psapi's GetModuleInformation -- one fewer library
        // dependency, and this DLL is already loaded in-process so the headers are
        // directly readable memory, same technique IW3MVM's FindPattern() uses.
        auto* dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(module);
        auto* ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            reinterpret_cast<uint8_t*>(dosHeader) + dosHeader->e_lfanew);
        const uint32_t sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;

        const std::vector<int> bytes = ParsePattern(pattern.bytes);
        if (bytes.empty())
            return 0;

        const std::size_t frontMaskCount = FrontMaskCount(bytes);

        const std::uintptr_t start = reinterpret_cast<std::uintptr_t>(module);
        const std::uintptr_t end = start + sizeOfImage - bytes.size();

        for (std::uintptr_t i = start; i < end; ++i)
        {
            std::size_t j = frontMaskCount;
            for (; j < bytes.size(); ++j)
            {
                if (bytes[j] != Wildcard &&
                    bytes[j] != *reinterpret_cast<uint8_t*>(i + j))
                    break;
            }

            if (j != bytes.size())
                continue;

            std::uintptr_t address = i + pattern.offset;

            if (pattern.dereference)
                address = *reinterpret_cast<std::uintptr_t*>(address);

            if (pattern.followCodeFlow)
                address = FollowCodeFlow(address);

            return address;
        }

        return 0;
    }
}
