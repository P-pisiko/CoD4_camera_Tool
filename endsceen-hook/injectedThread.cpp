#include "injectedThread.h"
#include "trampolines.h"
#include "d3d.h"
#include "hook.h"
#include <string>
#include <sstream>
#include "FrameCounter.h"
#include "PipeClientIO.h"
#include "GameAddresses.h"
#include "BoneRecorder.h"
endSceneFunc trampEndScene = nullptr;

void WINAPI injectedThread(HMODULE hModule)
{

    //  wait for the game to be up and running
    Sleep(1000 * 3);
    g_frameCounter = new FrameCounter();
    g_pipeClient = new PipeClientIO();
    // Get the function pointer from the dummy device object
    d3dHelper d3dHelper;
    void* ogEndSceneAddress = d3dHelper.d3d9DeviceTable[42];

    // hook EndScene() via MinHook and retrieve the trampoline
    Hook::Init();
    Hook::Create(ogEndSceneAddress, (void*)d3dHelper.endSceneHook, (void**)&trampEndScene, "EndScene");

    // Resolve the bone-export signatures once. Non-fatal if the game build doesn't match --
    // camera capture/overlay must keep working either way (see GameAddresses.h).
    CoD4Hook::GameAddresses::Init();
    CoD4Hook::g_boneRecorder = new CoD4Hook::BoneRecorder();

    // Keep thread alive until "HOME" was hit
    while (!GetAsyncKeyState(VK_HOME))
    {
        
        Sleep(1000);
    }
    delete g_pipeClient;
    delete g_frameCounter;
    g_pipeClient = nullptr;
    g_frameCounter = nullptr;

    CoD4Hook::g_boneRecorder->Shutdown();
    delete CoD4Hook::g_boneRecorder;
    CoD4Hook::g_boneRecorder = nullptr;

    // uninject and unhook
    Hook::Remove(ogEndSceneAddress, "EndScene");
    Hook::Shutdown();

    MessageBox(0, "BYE!", ":P", 0);
    FreeLibraryAndExitThread(hModule, 0);
}
