
using SharpGLTF.Animations;
using SharpGLTF.Scenes;
using SharpGLTF.Schema2;
using System.Numerics;

namespace CoD4_dm1.FileFormats
{

    public class GlTF
    {
        private const float INC_TO_METRE = 0.0254f;
        private const float DEG_TO_RAD = (float)(Math.PI / 180.0);
        private const float RAD_TO_DEG = (float)(180.0 / Math.PI);

        public static Task ExportToGLB(Structs.Entitys.Header header, List<Structs.Entitys.Camera> camList)
        {
            var flipY = false;

            var scene = new SceneBuilder();
            const float VerticalFOV = 80f * DEG_TO_RAD;
            const float ZNear = 0.5f;
            const float ZFar = 1000f;
            var camBuilder = new CameraBuilder.Perspective(VerticalFOV, ZNear, ZFar);

            var first = camList[0];
            var camPos0 = new Vector3(
                first.X * INC_TO_METRE,
                (flipY ? -first.Y : first.Y) * INC_TO_METRE,
                first.Z * INC_TO_METRE
            );

            scene.AddCamera(camBuilder, Matrix4x4.Identity);
            var model = scene.ToGltf2();
            var camNode = model.LogicalNodes.FirstOrDefault(n => n.Camera != null);
            
            
            var translationKeys = new List<(float time, Vector3 value)>(camList.Count);
            var rotationKeys = new List<(float time, Quaternion value)>(camList.Count);

            for (int i = 0; i < camList.Count; i++)
            {
                var f = camList[i];
                float time = i / (float)header.ConstCaptureFps;

                float x = f.X * INC_TO_METRE;
                float y = f.Z * INC_TO_METRE;  // Game Z becomes Blender Y
                float z = (flipY ? f.Y : -f.Y) * INC_TO_METRE;  // Game Y becomes Blender -Z
                var pos = new Vector3(x, y, z);

                var quat = ConvertCod4ToBlenderQuaternion(f.Yaw, f.Pitch);

                translationKeys.Add((time, pos));
                rotationKeys.Add((time, quat));

                /*if (i % Math.Max(1, header.ConstCaptureFps / 2) == 0) //debug sample 
                {
                    Console.WriteLine("Rotation keys sample:" + rotationKeys[i]);
                }*/
            }

            if (camNode != null)
            {
                camNode.WithRotationAnimation("Camera_Animation", rotationKeys.ToArray());
                camNode.WithTranslationAnimation("Camera_Animation", translationKeys.ToArray());
            }

            var MapName = string.IsNullOrWhiteSpace(header.MapName) ? "map" : header.MapName;
            var filename = $"{MapName}_fixed.glb";

            if (!Directory.Exists("./exported_cams"))
            {
                Directory.CreateDirectory("./exported_cams");
            }

            model.SaveGLB($"./exported_cams/{filename}");
            Console.WriteLine($"Done Writing to glb file: {filename}. Frames:{camList.Count} FPS:{header.ConstCaptureFps}");

            return Task.CompletedTask;
        }

        private static Quaternion ConvertCod4ToBlenderQuaternion(float yawDeg, float pitchDeg)
        {
            float signedPitchDeg = (pitchDeg <= 85f) ? -pitchDeg : 360f - pitchDeg;

            float pitchRad = signedPitchDeg * DEG_TO_RAD;
            float yawRad = yawDeg * DEG_TO_RAD;

            // Blender axes: up = +Y, right = +X
            var quatPitch = Quaternion.CreateFromAxisAngle(Vector3.UnitX, pitchRad);
            var quatYaw = Quaternion.CreateFromAxisAngle(Vector3.UnitY, yawRad);

            // Combine: pitch first, then yaw (world-yaw)
            var q = Quaternion.Normalize(Quaternion.Multiply(quatYaw, quatPitch));
            // +90° yaw offset
            float yawOffsetRad = -90f * DEG_TO_RAD;
            var quatOffset = Quaternion.CreateFromAxisAngle(Vector3.UnitY, yawOffsetRad);

            return Quaternion.Normalize(Quaternion.Multiply(quatOffset, q));
        }

        private static Quaternion EulerXYZToQuaternion(float x, float y, float z)
        {
            // XYZ Euler to Quaternion conversion
            // This applies rotations in order: X, then Y, then Z
            float cx = (float)Math.Cos(x * 0.5);
            float sx = (float)Math.Sin(x * 0.5);
            float cy = (float)Math.Cos(y * 0.5);
            float sy = (float)Math.Sin(y * 0.5);
            float cz = (float)Math.Cos(z * 0.5);
            float sz = (float)Math.Sin(z * 0.5);

            Quaternion quat;
            quat.W = cx * cy * cz + sx * sy * sz;
            quat.X = sx * cy * cz - cx * sy * sz;
            quat.Y = cx * sy * cz + sx * cy * sz;
            quat.Z = cx * cy * sz - sx * sy * cz;

            return quat;
        }
    }
}
