using System.Numerics;
using System.Text;

namespace CoD4_dm1.FileFormats
{
    /// <summary>
    /// Reads the `.cd4bones` file the injected DLL's BoneLog writes, converts every sample into
    /// Blender-space position/rotation using the same axis remap the camera path already
    /// uses (GlTF.cs)
    /// </summary>
    public static class BoneLogReader
    {
        private const float INC_TO_METRE = 0.0254f;

        // Same -90-degree-about-X remap the camera path uses to go from CoD4's X-forward,
        // Y-left, Z-up space to Blender's: (x, y, z) -> (x, z, -y).
        private static readonly Quaternion AxisConversion =
            Quaternion.CreateFromAxisAngle(Vector3.UnitX, -(float)(Math.PI / 2.0));

        public static List<List<Structs.Entitys.BoneObjectFrame>> Read(string path)
        {
            var frames = new List<List<Structs.Entitys.BoneObjectFrame>>();
            if (!File.Exists(path))
                return frames;

            using var stream = File.Open(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
            using var reader = new BinaryReader(stream);

            var magic = reader.ReadBytes(4);
            if (magic.Length != 4 || Encoding.ASCII.GetString(magic) != "CD4B")
                throw new InvalidDataException($"'{path}' is not a cd4bones file (bad magic).");

            var version = reader.ReadUInt32();
            if (version != 1)
                throw new InvalidDataException($"Unsupported cd4bones version {version} in '{path}'.");

            // last known-good rotation per (object, bone), for sign-continuity correction.
            var previousRotation = new Dictionary<(string obj, string bone), Quaternion>();

            while (stream.Position < stream.Length)
            {
                uint frameIndex;
                try
                {
                    frameIndex = reader.ReadUInt32();
                }
                catch (EndOfStreamException)
                {
                    break; // trailing partial frame (the game crashed mid-write)
                }

                byte objectCount = reader.ReadByte();
                var objects = new List<Structs.Entitys.BoneObjectFrame>(objectCount);

                for (int o = 0; o < objectCount; o++)
                {
                    string objectName = ReadString(reader);
                    ushort boneCount = reader.ReadUInt16();
                    var bones = new List<Structs.Entitys.BoneSample>(boneCount);

                    for (int b = 0; b < boneCount; b++)
                    {
                        string boneName = ReadString(reader);
                        bool valid = reader.ReadByte() != 0;
                        float px = reader.ReadSingle();
                        float py = reader.ReadSingle();
                        float pz = reader.ReadSingle();

                        var m = new float[9];
                        for (int i = 0; i < 9; i++)
                            m[i] = reader.ReadSingle();

                        var position = ConvertPosition(px, py, pz);
                        var key = (objectName, boneName);

                        Quaternion rotation;
                        if (valid)
                        {
                            rotation = ConvertRotation(m);
                            if (previousRotation.TryGetValue(key, out var prev) && Quaternion.Dot(prev, rotation) < 0f)
                                rotation = -rotation;
                            previousRotation[key] = rotation;
                        }
                        else
                        {
                            // Lookup failed this frame
                            rotation = Quaternion.Identity;
                        }

                        bones.Add(new Structs.Entitys.BoneSample
                        {
                            Name = boneName,
                            Valid = valid,
                            Position = position,
                            Rotation = rotation,
                        });
                    }

                    objects.Add(new Structs.Entitys.BoneObjectFrame { ObjectName = objectName, Bones = bones });
                }

                // BoneLog writes one frame per captured heartbeat, so frameIndex should equal, frames.Count already;
                while (frames.Count < frameIndex)
                    frames.Add(new List<Structs.Entitys.BoneObjectFrame>());

                frames.Add(objects);
            }

            return frames;
        }

        private static string ReadString(BinaryReader reader)
        {
            byte length = reader.ReadByte();
            var bytes = reader.ReadBytes(length);
            return Encoding.UTF8.GetString(bytes);
        }

        private static Vector3 ConvertPosition(float x, float y, float z)
            => new Vector3(x, z, -y) * INC_TO_METRE;

        private static Quaternion ConvertRotation(float[] m)
        {
            // m[0..2]/[3..5]/[6..8] are the bone's local X/Y/Z axes expressed in CoD4 world
            // space, one contiguous triple per axis (column-major, matching iwxmvm's own
            // glm::make_mat3(rotationMatrix)
            var rot = new Matrix4x4(
                m[0], m[1], m[2], 0f,
                m[3], m[4], m[5], 0f,
                m[6], m[7], m[8], 0f,
                0f, 0f, 0f, 1f);

            var q = Quaternion.CreateFromRotationMatrix(rot);
            return Quaternion.Normalize(AxisConversion * q * Quaternion.Conjugate(AxisConversion));
        }
    }
}
