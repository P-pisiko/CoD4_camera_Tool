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

    if (!font) return;

    // size setting
    const int left = 10;
    const int top = 10;
    const int padding = 6;
    const int lineHeight = 24; // roughly matches D3DXCreateFont size 24
    const int numLines = 2;
    
    RECT bg;
    bg.left = left - padding;
    bg.top = top - padding;
    bg.right = left + 240 + padding; // width you want (adjust)
    bg.bottom = top + (lineHeight * numLines) + padding;

    //D3DRECT d3drect = { bg.left, bg.top, bg.right, bg.bottom };
    //device->Clear(1, &d3drect, D3DCLEAR_TARGET, D3DCOLOR_ARGB(85, 120, 200, 45), 0, 0);
    
    RECT r;
    r.left = left; r.right = bg.right - padding;

    // Line 0
    r.top = top;
    r.bottom = top + lineHeight;
    char buf[128];
    sprintf_s(buf, "Actual Frames: %d", frameCount);
    font->DrawTextA(NULL, buf, -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE, D3DCOLOR_ARGB(255, 255, 255, 0));

    // Line 1
    r.top = top + lineHeight;
    r.bottom = top + lineHeight * 2;
    sprintf_s(buf, "Registed Frame: 0");
    font->DrawTextA(NULL, buf, -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE, D3DCOLOR_ARGB(255, 255, 255, 0));
}
