#include "hook.h"
#include "MinHook.h"
#include <string>

bool Hook_Init()
{
    MH_STATUS status = MH_Initialize();
    return status == MH_OK;
}

void Hook_Shutdown()
{
    MH_Uninitialize();
}

bool Hook_Attach(void* target, void* detour, void** original)
{
    MH_STATUS status = MH_CreateHook(target, detour, original);
    if (status != MH_OK)
    {
        MessageBox(0, std::string(std::string("[Hook] MH_CreateHook failed: ") + MH_StatusToString(status)).c_str(), ":(", 0);
        return false;
    }

    status = MH_EnableHook(target);
    if (status != MH_OK)
    {
        MessageBox(0, std::string(std::string("[Hook] MH_EnableHook failed: ") + MH_StatusToString(status)).c_str(), ":(", 0);
        MH_RemoveHook(target);
        return false;
    }

    return true;
}

bool Hook_Detach(void* target)
{
    MH_STATUS disableStatus = MH_DisableHook(target);
    MH_STATUS removeStatus = MH_RemoveHook(target);
    return disableStatus == MH_OK && removeStatus == MH_OK;
}
