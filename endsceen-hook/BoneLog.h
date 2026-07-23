#pragma once
#ifndef BONE_LOG_H
#define BONE_LOG_H

#include "BoneTypes.h"
#include <cstdio>
#include <string>
#include <vector>

// Streams captured bone frames straight to disk as they're captured (per
// cod4-dm1-migration-plan.md, section 4: bulk per-frame data stays off the control pipe and
// gets written to a file the C# host reads once, after recording stops).
//
// Binary layout (little-endian, native x86 float/int sizes):
//   Header (written once, on Open()):
//     char     magic[4]      = "CD4B"
//     uint32_t version       = 1
//   Per frame (written once per WriteFrame() call):
//     uint32_t frameIndex        // 0-based, matches the C# side's camera frame index
//     uint8_t  objectCount
//     per object:
//       uint8_t  nameLength
//       char     name[nameLength]              // NOT null-terminated
//       uint16_t boneCount
//       per bone:
//         uint8_t  boneNameLength
//         char     boneName[boneNameLength]     // NOT null-terminated
//         uint8_t  valid                        // 0/1
//         float    position[3]
//         float    rotation[9]                  // row-major 3x3
//
// Every frame is fully self-describing (object/bone names inline, no deferred string table)
// so the file is valid to parse at any point during capture, unlike IW3MVM's old IW3D format
// which only became readable after the trailing string table was written on stop.
namespace CoD4Hook
{
    class BoneLog
    {
    public:
        ~BoneLog();

        bool Open(const std::string& path);
        void WriteFrame(const std::vector<BoneObjectSample>& objects);
        void Close();
        bool IsOpen() const { return _file != nullptr; }

    private:
        FILE* _file = nullptr;
        uint32_t _nextFrameIndex = 0;
    };
}

#endif
