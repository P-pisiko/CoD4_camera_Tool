using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace CoD4_dm1.Structs
{
    public class Entitys
    {
        public struct FrameRate 
        { 
            public float fps; 
        }

        public struct Camera
        {
            public float X;
            public float Y;
            public float Z;
            public float Yaw;
            public float Pitch;
        }

        public struct Header
        {
            public float ConstCaptureFps;
            public string MapName;
            public int TotalFrames;
            
        }

        /// <summary>
        /// One named bone's world-space transform for a single frame. `Valid` mirrors
        /// CG_DObjGetWorldBoneMatrix's own success flag (BoneLogReader.cs) rather than a
        /// zero-vector heuristic
        /// </summary>
        public struct BoneSample
        {
            public string Name;
            public bool Valid;
            public Vector3 Position;
            public Quaternion Rotation;
        }

        public struct BoneObjectFrame
        {
            public string ObjectName;
            public List<BoneSample> Bones;
        }
    }

}
