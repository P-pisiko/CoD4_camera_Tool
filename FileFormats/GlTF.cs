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

        public static Task ExportToGLB(Structs.Entitys.Header header, List<Structs.Entitys.Camera> camList, List<List<Structs.Entitys.BoneObjectFrame>> boneFrames)
        {
            var flipY = false;
            var scene = new SceneBuilder();

            #region Camera Properties
            const float VerticalFOV = 80f * DEG_TO_RAD;
            const float ZNear = 0.5f;
            const float ZFar = 1000f;
            var camBuilder = new CameraBuilder.Perspective(null, VerticalFOV, ZNear, ZFar ); //16f / 9f
            #endregion

            scene.AddCamera(camBuilder, Matrix4x4.Identity);

            // Bones: one NodeBuilder (mesh-less -> imports as an Empty in Blender) per unique
            // (object, bone) name, added to the same scene/GLB as the camera -- one file, no
            // separate import step. Must happen before ToGltf2() below so the nodes exist in
            // the resulting Schema2 model to key animation onto (viewmodel-bone-export.md
            // section 4: "Empties, one per named bone/tag, world matrix keyed every frame").
            var boneNodeNames = new HashSet<string>();
            foreach (var frame in boneFrames)
            {
                foreach (var obj in frame)
                {
                    foreach (var bone in obj.Bones)
                    {
                        string nodeName = $"{obj.ObjectName}_{bone.Name}";
                        if (boneNodeNames.Add(nodeName))
                        {
                            scene.AddNode(new NodeBuilder(nodeName));
                        }
                    }
                }
            }

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

                var quat = Cod4ToQuaternion(f.Yaw, f.Pitch);

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

            // Same "look up the resolved Schema2 node by name, key it after conversion"
            // pattern the camera above uses. Skips invalid samples (lookup failed that
            // frame) rather than keyframing a meaningless identity transform onto them.
            var boneTranslationKeys = new Dictionary<string, List<(float time, Vector3 value)>>();
            var boneRotationKeys = new Dictionary<string, List<(float time, Quaternion value)>>();

            for (int i = 0; i < boneFrames.Count; i++)
            {
                float time = i / (float)header.ConstCaptureFps;
                foreach (var obj in boneFrames[i])
                {
                    foreach (var bone in obj.Bones)
                    {
                        if (!bone.Valid)
                            continue;

                        string nodeName = $"{obj.ObjectName}_{bone.Name}";
                        if (!boneTranslationKeys.TryGetValue(nodeName, out var translations))
                        {
                            translations = new List<(float, Vector3)>();
                            boneTranslationKeys[nodeName] = translations;
                            boneRotationKeys[nodeName] = new List<(float, Quaternion)>();
                        }

                        translations.Add((time, bone.Position));
                        boneRotationKeys[nodeName].Add((time, bone.Rotation));
                    }
                }
            }

            foreach (var nodeName in boneTranslationKeys.Keys)
            {
                var boneNode = model.LogicalNodes.FirstOrDefault(n => n.Name == nodeName);
                if (boneNode == null)
                    continue;

                boneNode.WithRotationAnimation("Bone_Animation", boneRotationKeys[nodeName].ToArray());
                boneNode.WithTranslationAnimation("Bone_Animation", boneTranslationKeys[nodeName].ToArray());
            }
            #region Shit everything happened here to disk.
            var MapName = string.IsNullOrWhiteSpace(header.MapName) ? "map" : header.MapName;

            var filename = $"{MapName}_{header.ConstCaptureFps}fps_{DateTime.Now.ToString("HH-mm-ss")}.glb";

            if (!Directory.Exists("./exported_cams"))
            {
                Directory.CreateDirectory("./exported_cams");
            }
            model.SaveGLB($"./exported_cams/{filename}");
            Console.WriteLine($"Done Writing glb file: {filename}. Frames:{camList.Count} FPS:{header.ConstCaptureFps} BoneNodes:{boneTranslationKeys.Count}");
            #endregion
            return Task.CompletedTask;
            
        }
        #region Euler to Quaternion Conversion stuff
        private static Quaternion Cod4ToQuaternion(float yawDeg, float pitchDeg)
        {   // Turning CoD4's wierd pitch deg to signed so its always positive
            float signedPitchDeg = (pitchDeg <= 85f) ? -pitchDeg : 360f - pitchDeg;
            
            float pitchRad = signedPitchDeg * DEG_TO_RAD;
            float yawRad = yawDeg * DEG_TO_RAD;

            // Blender axes: up = +Y, right = +X
            var quatPitch = Quaternion.CreateFromAxisAngle(Vector3.UnitX, pitchRad);
            var quatYaw = Quaternion.CreateFromAxisAngle(Vector3.UnitY, yawRad);

            // Combine: pitch first, then yaw (i have no idea what this does..)
            var q = Quaternion.Normalize(Quaternion.Multiply(quatYaw, quatPitch));
            // -90° yaw offset
            float yawOffsetRad = -90f * DEG_TO_RAD;
            var quatOffset = Quaternion.CreateFromAxisAngle(Vector3.UnitY, yawOffsetRad);

            return Quaternion.Normalize(Quaternion.Multiply(quatOffset, q));
        }
        /// <summary>
        /// NOT USED ANYMORE
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <param name="z"></param>
        /// <returns></returns>
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

        #endregion
    }
}
