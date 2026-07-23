#pragma once
#ifndef BONE_RECORDER_H
#define BONE_RECORDER_H

#include "BoneLog.h"
#include "BoneTypes.h"
#include <vector>

// Owns the bone-capture recording lifecycle inside the injected DLL and writes to
// `<gameRoot>\bone_capture\latest.cd4bones`. Deliberately mirrors the existing camera
// capture's start/stop trigger instead of adding a new one: `d3dHelper::endSceneHook`
// already computes an edge-detected NUMPAD5 toggle (`v`) every frame and forwards it to the
// C# host over the pipe, which flips its own `_recordState` on `v==1` and starts/stops the
// camera CSV/GLB capture. Feeding that exact same edge value into `OnFrame()` here keeps
// this recorder in lockstep with the C# side's record state without adding any new pipe
// messages (cod4-dm1-migration-plan.md, section 4: "keep the pipe for control, not bulk
// data"), and keeps bone frame indices aligned 1:1 with camera frame indices since both are
// driven by the identical per-EndScene cadence.
namespace CoD4Hook
{
    class BoneRecorder
    {
    public:
        // `toggleEdge` = the same edge-detected bool the hook already sends as the pipe
        // heartbeat payload. Call once per EndScene call, every frame.
        void OnFrame(bool toggleEdge);

        // Force-stops and flushes if a recording is still open -- called on DLL unload so a
        // session doesn't get silently truncated if the user unloads mid-recording.
        void Shutdown();

    private:
        void Start();
        void Stop();

        BoneLog _log;
        bool _recording = false;
        std::vector<BoneObjectSample> _scratch;
    };

    extern BoneRecorder* g_boneRecorder;
}

#endif
