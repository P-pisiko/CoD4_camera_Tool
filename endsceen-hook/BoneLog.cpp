#include "BoneLog.h"
#include <algorithm>

namespace CoD4Hook
{
    namespace
    {
        void WriteBytes(FILE* file, const void* data, size_t size)
        {
            fwrite(data, 1, size, file);
        }

        template <typename T>
        void WriteValue(FILE* file, const T& value)
        {
            WriteBytes(file, &value, sizeof(T));
        }

        // Length-prefixed, not null-terminated; truncated to fit the uint8_t length prefix
        // (bone/object names are always short in practice -- this is just a safety clamp).
        void WriteString(FILE* file, const std::string& str)
        {
            const uint8_t length = static_cast<uint8_t>(std::min<size_t>(str.size(), 255));
            WriteValue(file, length);
            WriteBytes(file, str.data(), length);
        }
    }

    BoneLog::~BoneLog()
    {
        Close();
    }

    bool BoneLog::Open(const std::string& path)
    {
        Close();

        fopen_s(&_file, path.c_str(), "wb");
        if (!_file)
            return false;

        _nextFrameIndex = 0;

        WriteBytes(_file, "CD4B", 4);
        WriteValue<uint32_t>(_file, 1);
        fflush(_file);
        return true;
    }

    void BoneLog::WriteFrame(const std::vector<BoneObjectSample>& objects)
    {
        if (!_file)
            return;

        WriteValue<uint32_t>(_file, _nextFrameIndex++);

        const uint8_t objectCount = static_cast<uint8_t>(std::min<size_t>(objects.size(), 255));
        WriteValue(_file, objectCount);

        for (uint8_t i = 0; i < objectCount; ++i)
        {
            const BoneObjectSample& object = objects[i];
            WriteString(_file, object.name);

            const uint16_t boneCount = static_cast<uint16_t>(std::min<size_t>(object.bones.size(), 65535));
            WriteValue(_file, boneCount);

            for (uint16_t b = 0; b < boneCount; ++b)
            {
                const BoneSample& bone = object.bones[b];
                WriteString(_file, bone.name);
                WriteValue<uint8_t>(_file, bone.valid ? 1 : 0);
                WriteBytes(_file, bone.position, sizeof(bone.position));
                WriteBytes(_file, bone.rotation, sizeof(bone.rotation));
            }
        }

        // Flush every frame -- this file is the durability net a game crash mid-recording
        // shouldn't lose (cod4-dm1-migration-plan.md, section 4). One fflush() per render
        // frame is negligible next to a D3D9 Present().
        fflush(_file);
    }

    void BoneLog::Close()
    {
        if (_file)
        {
            fclose(_file);
            _file = nullptr;
        }
    }
}
