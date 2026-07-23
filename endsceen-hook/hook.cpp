#include "hook.h"
#include "MinHook.h"
#include <string>

namespace Hook
{
    bool Init()
    {
        MH_STATUS status = MH_Initialize();
        if (status != MH_OK)
        {
            MessageBox(0, std::string("[Hook] MH_Initialize failed: " + std::string(MH_StatusToString(status))).c_str(), ":(", 0);
            return false;
        }
        return true;
    }

    bool Create(void* target, void* detour, void** original, const char* descr)
    {
        MH_STATUS status = MH_CreateHook(target, detour, original);
        if (status != MH_OK)
        {
            MessageBox(0, std::string("[Hook] Failed to create hook for " + std::string(descr) + ": " + MH_StatusToString(status)).c_str(), ":(", 0);
            return false;
        }

        status = MH_EnableHook(target);
        if (status != MH_OK)
        {
            MessageBox(0, std::string("[Hook] Failed to enable hook for " + std::string(descr) + ": " + MH_StatusToString(status)).c_str(), ":(", 0);
            return false;
        }

        return true;
    }

    bool Remove(void* target, const char* descr)
    {
        MH_STATUS status = MH_DisableHook(target);
        if (status != MH_OK)
        {
            MessageBox(0, std::string("[Hook] Failed to disable hook for " + std::string(descr) + ": " + MH_StatusToString(status)).c_str(), ":(", 0);
            return false;
        }

        status = MH_RemoveHook(target);
        if (status != MH_OK)
        {
            MessageBox(0, std::string("[Hook] Failed to remove hook for " + std::string(descr) + ": " + MH_StatusToString(status)).c_str(), ":(", 0);
            return false;
        }

        return true;
    }

    void Shutdown()
    {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }
}
