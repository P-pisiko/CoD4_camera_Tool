#pragma once
#ifndef D3D_H
#define D3D_H

#include "d3d9.h"
#include <stdexcept>
#include <atomic>

// counts calls currently inside endSceneHook; teardown waits for this to hit 0
extern std::atomic<int> g_hookActiveCalls;

class d3dHelper {
public:
    d3dHelper();
    bool getD3D9Device();
    // The vTable entries of the D3D9 dummy device
    char* d3d9DeviceTable[119];

    static void drawRectangle(int x, int y, int h, int w, D3DCOLOR color);
    static void APIENTRY endSceneHook(LPDIRECT3DDEVICE9 pDevice);

private:
    static BOOL CALLBACK enumGetProcessWindow(HWND _hwnd, LPARAM lParam);
};

#endif