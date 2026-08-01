#include "injectedThread.h"
#include "trampolines.h"
#include "d3d.h"
#include "hook.h"
#include <string>
#include <sstream>
#include "FrameCounter.h"
#include "PipeClientIO.h"
#include "ViewmodelBoneExport.h"
endSceneFunc trampEndScene = nullptr;

void WINAPI injectedThread(HMODULE hModule)
{
    Sleep(1000 * 2);
    g_frameCounter = new FrameCounter();
    g_pipeClient = new PipeClientIO();

    Hook_Init();

    d3dHelper d3dHelper;
    void* ogEndSceneAddress = (void*)d3dHelper.d3d9DeviceTable[42];

    Hook_Attach(ogEndSceneAddress, (void*)d3dHelper.endSceneHook, (void**)&trampEndScene);

    // Keep thread alive until "HOME" was hit
    while (!GetAsyncKeyState(VK_HOME))
    {
        Sleep(500);
    }

    // stop new calls, then wait for any in-flight call to finish before
    // freeing anything it touches
    Hook_Detach(ogEndSceneAddress);
    while (g_hookActiveCalls.load(std::memory_order_acquire) != 0)
    {
        Sleep(1);
    }

    ViewmodelBoneExport::Shutdown();
    delete g_pipeClient;
    delete g_frameCounter;
    g_pipeClient = nullptr;
    g_frameCounter = nullptr;

    Hook_Shutdown();

    MessageBox(0, "BYE!", ":P", 0);
    FreeLibraryAndExitThread(hModule, 0);
}
