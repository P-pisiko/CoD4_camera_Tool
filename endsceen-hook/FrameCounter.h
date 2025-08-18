#pragma once
#include <d3dx9.h>

class FrameCounter {
public:
    FrameCounter();
    ~FrameCounter();

    void initDevice(LPDIRECT3DDEVICE9 device);
    void onFrame(LPDIRECT3DDEVICE9 device);

private:
    int frameCount;
    ID3DXFont* font;
};

// Declare the global pointer here as `extern`
extern FrameCounter* g_frameCounter;
