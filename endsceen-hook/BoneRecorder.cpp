#include "BoneRecorder.h"
#include "BoneCapture.h"
#include "GameAddresses.h"
#include <Windows.h>
#include <string>
#include <fstream>

namespace CoD4Hook
{
    BoneRecorder* g_boneRecorder = nullptr;

    namespace
    {
        // The DLL's own module directory, i.e. the game's install directory -- NOT the
        // current working directory, which can differ from it. Entry.cs computes the exact
        // same directory (`Path.GetDirectoryName(MainModule.FileName)`) as `gameRootPath`
        // before injecting this DLL, so the two sides agree on where to look without any
        // path being passed between them.
        std::string GameRootDirectory()
        {
            char path[MAX_PATH] = {};
            GetModuleFileNameA(nullptr, path, MAX_PATH);

            std::string full(path);
            const size_t lastSlash = full.find_last_of('\\');
            return (lastSlash == std::string::npos) ? "" : full.substr(0, lastSlash);
        }

        std::string BoneCaptureDirectory()
        {
            const std::string dir = GameRootDirectory() + "\\bone_capture";
            CreateDirectoryA(dir.c_str(), nullptr);
            return dir;
        }

        std::string BoneLogPath()
        {
            return BoneCaptureDirectory() + "\\latest.cd4bones";
        }

        void WriteDiagnosticsFile()
        {
            std::ofstream file(BoneCaptureDirectory() + "\\diagnostics.txt", std::ios::trunc);
            if (file)
                file << GameAddresses::DebugText();
        }
    }

    void BoneRecorder::OnFrame(bool toggleEdge)
    {
        if (toggleEdge)
        {
            if (!_recording)
                Start();
            else
            {
                Stop();
                return; // matches NamedPipeServer's stop branch: no frame captured on the
                        // same edge that turns recording off.
            }
        }

        if (!_recording)
            return;

        _scratch.clear();
        BoneCapture::CaptureViewmodel(_scratch);

        // Write every heartbeat regardless of capture success so the bone log's frame index
        // stays 1:1 with the C# side's camera frame list (an empty-object frame just means
        // "no viewmodel this frame", not a dropped/missing frame).
        _log.WriteFrame(_scratch);
    }

    void BoneRecorder::Shutdown()
    {
        if (_recording)
            Stop();
    }

    void BoneRecorder::Start()
    {
        _recording = true;
        WriteDiagnosticsFile();
        _log.Open(BoneLogPath());
    }

    void BoneRecorder::Stop()
    {
        _recording = false;
        _log.Close();
    }
}
