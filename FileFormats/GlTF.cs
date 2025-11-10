
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
            var VerticalFOV = 60 * DEG_TO_RAD;
            var ZNear = 2f;
            var ZFar = 1;
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
            
            camNode.LocalTransform = Matrix4x4.Identity; // Reset maybe ?
            
            var translationKeys = new List<(float time, Vector3 value)>(camList.Count);
            var rotationKeys = new List<(float time, Quaternion value)>(camList.Count);

            for (int i = 0; i < camList.Count; i++)
            {
                var f = camList[i];
                float time = i / (float)header.ConstCaptureFps;

                // Position: Direct conversion, no axis swapping
                float x = f.X * INC_TO_METRE;
                float y = f.Z * INC_TO_METRE;  // Game Z becomes Blender Y
                float z = (flipY ? f.Y : -f.Y) * INC_TO_METRE;  // Game Y becomes Blender -Z
                var pos = new Vector3(x, y, z);

                var quat = ConvertCod4ToBlenderQuaternion(f.Yaw, f.Pitch);

                translationKeys.Add((time, pos));
                rotationKeys.Add((time, quat));

                if (i % Math.Max(1, header.ConstCaptureFps / 2) == 0) // sample occasionally
                {
                    Console.WriteLine("Rotation keys sample:" + rotationKeys[i]);
                }
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
            // Convert pitch exactly like Python
            float signedPitchDeg = (pitchDeg <= 85f) ? -pitchDeg : 360f - pitchDeg;
            float pitchRad = signedPitchDeg * DEG_TO_RAD + (float)(Math.PI / 2.0);

            // Convert yaw exactly like Python
            float yawRad = yawDeg * DEG_TO_RAD - (float)(Math.PI / 2.0);

            // Create quaternion DIRECTLY in Blender's Y-up coordinate system
            // Blender uses XYZ euler order: pitch around X, roll around Y, yaw around Z
            // BUT: In Blender's Y-up system, yaw should be around Y axis (not Z)
            var quat = EulerXYZToQuaternion(pitchRad, yawRad, 0f);

            return quat;
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
