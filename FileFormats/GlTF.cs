
using SharpGLTF.Animations;
using SharpGLTF.Scenes;
using SharpGLTF.Schema2;
using System.Numerics;

namespace CoD4_dm1.FileFormats
{
    
    public class GlTF
    {
        private const float INC_TO_METRE = 0.0254f;

        public static Task ExportToGLB(Structs.Entitys.Header header, List<Structs.Entitys.Camera> camList)
        {
            var flipY = false;

            var scene = new SceneBuilder();

            var VerticalFOV = (float)(45.0 * Math.PI / 180.0);
            var ZNear = 0.01f;
            var ZFar = 10000f;

            var camBuilder = new CameraBuilder.Perspective(VerticalFOV, ZNear, ZFar);

            var first = camList[0];
            var camPos0 = new Vector3(first.X * INC_TO_METRE, (flipY ? -first.Y : first.Y) * INC_TO_METRE, first.Z * INC_TO_METRE);

            var camTarget0 = camPos0 + Vector3.UnitZ;

            scene.AddCamera(camBuilder, camPos0, camTarget0);

            var model = scene.ToGltf2();
            var camNode = model.LogicalNodes.FirstOrDefault(n => n.Camera != null);

            var translationKeys = new List<(float time, Vector3 value)>(camList.Count);
            var rotationKeys = new List<(float time, Quaternion value)>(camList.Count);

            for (int i = 0; i < camList.Count; i++)
            {
                var f = camList[i];
                float time = i / header.ConstCaptureFps;


                // position: inches -> meters, 
                float x = f.X * INC_TO_METRE;
                float y = (flipY ? -f.Y : f.Y) * INC_TO_METRE;
                float z = f.Z * INC_TO_METRE;
                var pos = new Vector3(x, y, z);

                // rotation: NO ROTATIONAL TRANSFORMATION IS APPLYING TO ANYTHING Huhh ???
                float signedPitchDeg = (f.Pitch <= 85f) ? -f.Pitch : 360f - f.Pitch;
                float pitch = (float)(signedPitchDeg * Math.PI / 180.0) + (float)(Math.PI / 2.0); // +90°
                float yaw = (float)(f.Yaw * Math.PI / 180.0) - (float)(Math.PI / 2.0);           // -90°
                float roll = 0f;

                var quat = Quaternion.CreateFromYawPitchRoll(yaw, pitch, roll);

                translationKeys.Add((time, pos));
                rotationKeys.Add((time, quat));
            }

            if (camNode != null)
            {
                camNode.WithTranslationAnimation("Camera_Translation", translationKeys.ToArray());
                camNode.WithRotationAnimation("Camera_Rotation", rotationKeys.ToArray());
            }
            // Very descriptive filename, ehe splended indeed !
            var safeMapName = string.IsNullOrWhiteSpace(header.MapName) ? "map" : header.MapName;
            var filename = $"{safeMapName}_BROKEN.glb";

            model.SaveGLB(filename);
            Console.WriteLine($"Done Writing to glb file: {filename}. Frames:{camList.Count} FPS:{header.ConstCaptureFps}");
            return Task.CompletedTask;
        }

    }
}
