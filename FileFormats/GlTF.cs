
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
            var VerticalFOV = 24;
            var ZNear = 0.01f;
            var ZFar = 10000f;
            var camBuilder = new CameraBuilder.Perspective(VerticalFOV, ZNear, ZFar);

            var first = camList[0];
            var camPos0 = new Vector3(
                first.X * INC_TO_METRE,
                (flipY ? -first.Y : first.Y) * INC_TO_METRE,
                first.Z * INC_TO_METRE
            );

            // Calculate initial rotation
            var initialQuat = ConvertCod4ToBlenderQuaternion(first.Yaw, first.Pitch);
            var initialForward = Vector3.Transform(-Vector3.UnitZ, initialQuat);
            var camTarget0 = camPos0 + initialForward;

            scene.AddCamera(camBuilder, camPos0, camTarget0);
            var model = scene.ToGltf2();
            var camNode = model.LogicalNodes.FirstOrDefault(n => n.Camera != null);

            var translationKeys = new List<(float time, Vector3 value)>(camList.Count);
            var rotationKeys = new List<(float time, Quaternion value)>(camList.Count);

            for (int i = 0; i < camList.Count; i++)
            {
                var f = camList[i];
                float time = i / header.ConstCaptureFps;

                // Position: Direct conversion, no axis swapping
                float x = f.X * INC_TO_METRE;
                float y = f.Z * INC_TO_METRE;  // Game Z becomes Blender Y
                float z = (flipY ? f.Y : -f.Y) * INC_TO_METRE;  // Game Y becomes Blender -Z
                var pos = new Vector3(x, y, z);

                var quat = ConvertCod4ToBlenderQuaternion(f.Yaw, f.Pitch);

                translationKeys.Add((time, pos));
                rotationKeys.Add((time, quat));
            }

            if (camNode != null)
            {
                camNode.WithTranslationAnimation("Camera_Translation", translationKeys.ToArray());
                camNode.WithRotationAnimation("Camera_Rotation", rotationKeys.ToArray());
            }

            var safeMapName = string.IsNullOrWhiteSpace(header.MapName) ? "map" : header.MapName;
            var filename = $"{safeMapName}_FIXED.glb";

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
            float pitchRad = signedPitchDeg * DEG_TO_RAD + (float)(Math.PI / 2.0); // +90 degrees

            float yawRad = yawDeg * DEG_TO_RAD - (float)(Math.PI / 2.0); // -90 degrees

            float rollRad = 0f;

            return EulerXYZToQuaternion(pitchRad, 0f, yawRad);
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
