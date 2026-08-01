#pragma once

// Binary format (little-endian, written per capture session -- a session
// starts when recording turns on or the resolved viewmodel DObj changes):
//   header:
//     char     magic[9]     = "CD4VMBONE"
//     uint8_t  version      = 1
//     uint32_t boneCount
//     repeated boneCount times:
//       uint16_t nameLen   (0 if the name could not be resolved)
//       char     name[nameLen]   (not NUL-terminated)
//   then, appended once per captured frame:
//     uint32_t frameIndex
//     repeated boneCount times:
//       uint8_t  valid     (0 => CG_DObjGetWorldBoneMatrix failed for this bone; rot/origin are zeroed)
//       float    rotation[9]   (row-major 3x3)
//       float    origin[3]
namespace ViewmodelBoneExport
{
    void OnFrame(bool recording, int frameIndex);

    // Flushes and closes any open capture file. Safe to call multiple times.
    void Shutdown();
}
