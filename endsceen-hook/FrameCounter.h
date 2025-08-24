#pragma once
#include <d3dx9.h>

class FrameCounter {
public:
    FrameCounter();
    ~FrameCounter();

    void onFrame(LPDIRECT3DDEVICE9 device);
    int registedFrame;
    BOOL recState;
private:
    int frameCount;
    ID3DXFont* font;
    void initDevice(LPDIRECT3DDEVICE9 device);
};

// Declare the global pointer here as `extern`
extern FrameCounter* g_frameCounter;
