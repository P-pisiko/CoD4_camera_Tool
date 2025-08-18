#include "FrameCounter.h"
#include <cstdio>
FrameCounter* g_frameCounter = nullptr;

FrameCounter::FrameCounter() {
    frameCount = 0;
    font = nullptr;
}

FrameCounter::~FrameCounter() {
    if (font) {
        font->Release();
        font = nullptr;
    }
}

void FrameCounter::initDevice(LPDIRECT3DDEVICE9 device) {
    if (!font && device) {
        D3DXCreateFontA(device, 24, 0, FW_NORMAL, 1, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, "Arial", &font);
    }
}

void FrameCounter::onFrame(LPDIRECT3DDEVICE9 device) {
    ++frameCount;
    initDevice(device);

    if (font) {
        RECT r;
        SetRect(&r, 10, 10, 300, 50);
        char buf[64];
        sprintf_s(buf, "Frames: %d", frameCount);
        font->DrawTextA(NULL, buf, -1, &r, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 255, 0));
        sprintf_s(buf, "This is secont Line");
        font->DrawTextA(NULL, buf, -1, & r, DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 255, 0));
    }
}
